/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name:

    devcon.cpp

Abstract:

    Device Console
    command-line interface for managing devices

--*/

#include "devcon.h"
#include <newdev.h>

struct GenericContext {
    DWORD count;
    DWORD control;
    BOOL  reboot;
};

int cmdUpdate(LPCTSTR inf, LPCTSTR hwid)
/*++

Routine Description:
    UPDATE
    update driver for existing device(s)

Arguments:

    argc/argv - remaining parameters

Return Value:

    EXIT_xxxx

--*/
{
    int failcode = EXIT_FAIL;
    BOOL reboot = FALSE;
    DWORD flags = 0;
    DWORD res;
    TCHAR InfPath[MAX_PATH];

    //
    // Inf must be a full pathname
    //
    res = GetFullPathName(inf,MAX_PATH,InfPath,NULL);
    if((res >= MAX_PATH) || (res == 0)) {
        //
        // inf pathname too long
        //
        return EXIT_FAIL;
    }
    if(GetFileAttributes(InfPath)==(DWORD)(-1)) {
        //
        // inf doesn't exist
        //
        return EXIT_FAIL;
    }
    inf = InfPath;
    flags |= INSTALLFLAG_FORCE;

    if(!UpdateDriverForPlugAndPlayDevices(NULL,hwid,inf,flags,&reboot)) {
        goto final;
    }

    failcode = reboot ? EXIT_REBOOT : EXIT_OK;

final:

    return failcode;
}

int cmdInstall(LPCTSTR inf, LPCTSTR hwid)
/*++

Routine Description:

    CREATE
    Creates a root enumerated devnode and installs drivers on it

Arguments:

    argc/argv - remaining parameters

Return Value:

    EXIT_xxxx

--*/
{
    HDEVINFO DeviceInfoSet = INVALID_HANDLE_VALUE;
    SP_DEVINFO_DATA DeviceInfoData;
    GUID ClassGUID;
    TCHAR ClassName[MAX_CLASS_NAME_LEN];
    TCHAR hwIdList[LINE_LEN+4];
    TCHAR InfPath[MAX_PATH];
    int failcode = EXIT_FAIL;

    //
    // Inf must be a full pathname
    //
    if(GetFullPathName(inf,MAX_PATH,InfPath,NULL) >= MAX_PATH) {
        //
        // inf pathname too long
        //
        return EXIT_FAIL;
    }

    //
    // List of hardware ID's must be double zero-terminated
    //
    ZeroMemory(hwIdList,sizeof(hwIdList));
    if (FAILED(StringCchCopy(hwIdList,LINE_LEN,hwid))) {
        goto final;
    }

    //
    // Use the INF File to extract the Class GUID.
    //
    if (!SetupDiGetINFClass(InfPath,&ClassGUID,ClassName,sizeof(ClassName)/sizeof(ClassName[0]),0))
    {
        goto final;
    }

    //
    // Create the container for the to-be-created Device Information Element.
    //
    DeviceInfoSet = SetupDiCreateDeviceInfoList(&ClassGUID,0);
    if(DeviceInfoSet == INVALID_HANDLE_VALUE)
    {
        goto final;
    }

    //
    // Now create the element.
    // Use the Class GUID and Name from the INF file.
    //
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    if (!SetupDiCreateDeviceInfo(DeviceInfoSet,
        ClassName,
        &ClassGUID,
        NULL,
        0,
        DICD_GENERATE_ID,
        &DeviceInfoData))
    {
        goto final;
    }

    //
    // Add the HardwareID to the Device's HardwareID property.
    //
    if(!SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
        &DeviceInfoData,
        SPDRP_HARDWAREID,
        (LPBYTE)hwIdList,
        ((DWORD)_tcslen(hwIdList)+1+1)*sizeof(TCHAR)))
    {
        goto final;
    }

    //
    // Transform the registry element into an actual devnode
    // in the PnP HW tree.
    //
    if (!SetupDiCallClassInstaller(DIF_REGISTERDEVICE,
        DeviceInfoSet,
        &DeviceInfoData))
    {
        goto final;
    }

    //
    // update the driver for the device we just created
    //
    failcode = cmdUpdate(inf, hwid);

final:

    if (DeviceInfoSet != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(DeviceInfoSet);
    }

    return failcode;
}

int RemoveCallback(_In_ HDEVINFO Devs, _In_ PSP_DEVINFO_DATA DevInfo, _In_ DWORD Index, _In_ LPVOID Context)
/*++

Routine Description:

    Callback for use by Remove
    Invokes DIF_REMOVE
    uses SetupDiCallClassInstaller so cannot be done for remote devices
    Don't use CM_xxx API's, they bypass class/co-installers and this is bad.

Arguments:

    Devs    )_ uniquely identify the device
    DevInfo )
    Index    - index of device
    Context  - GenericContext

Return Value:

    EXIT_xxxx

--*/
{
    SP_REMOVEDEVICE_PARAMS rmdParams;
    GenericContext *pControlContext = (GenericContext*)Context;
    SP_DEVINSTALL_PARAMS devParams;
    LPCTSTR action = NULL;
    //
    // need hardware ID before trying to remove, as we wont have it after
    //
    TCHAR devID[MAX_DEVICE_ID_LEN];
    SP_DEVINFO_LIST_DETAIL_DATA devInfoListDetail;

    UNREFERENCED_PARAMETER(Index);

    devInfoListDetail.cbSize = sizeof(devInfoListDetail);
    if((!SetupDiGetDeviceInfoListDetail(Devs,&devInfoListDetail)) ||
            (CM_Get_Device_ID_Ex(DevInfo->DevInst,devID,MAX_DEVICE_ID_LEN,0,devInfoListDetail.RemoteMachineHandle)!=CR_SUCCESS)) {
        //
        // skip this
        //
        return EXIT_OK;
    }

    rmdParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    rmdParams.ClassInstallHeader.InstallFunction = DIF_REMOVE;
    rmdParams.Scope = DI_REMOVEDEVICE_GLOBAL;
    rmdParams.HwProfile = 0;
    if(!SetupDiSetClassInstallParams(Devs,DevInfo,&rmdParams.ClassInstallHeader,sizeof(rmdParams)) ||
       !SetupDiCallClassInstaller(DIF_REMOVE,Devs,DevInfo)) {
        //
        // failed to invoke DIF_REMOVE
        //
        action = L"fail";
    } else {
        //
        // see if device needs reboot
        //
        devParams.cbSize = sizeof(devParams);
        if(SetupDiGetDeviceInstallParams(Devs,DevInfo,&devParams) && (devParams.Flags & (DI_NEEDRESTART|DI_NEEDREBOOT))) {
            //
            // reboot required
            //
            action = L"reboot";
            pControlContext->reboot = TRUE;
        } else {
            //
            // appears to have succeeded
            //
            action = L"success";
        }
        pControlContext->count++;
    }
    _tprintf(TEXT("%-60s: %s\n"),devID,action);

    return EXIT_OK;
}

int cmdRemove(LPCTSTR hwid)
/*++

Routine Description:

    REMOVE
    remove devices

Arguments:

    Machine   - machine name, must be NULL
    argc/argv - remaining parameters

Return Value:

    EXIT_xxxx

--*/
{
    GenericContext context;
    int failcode = EXIT_FAIL;

    context.reboot = FALSE;
    context.count = 0;

    failcode = EnumerateDevices(DIGCF_PRESENT,hwid,RemoveCallback,&context);

    if(failcode == EXIT_OK) {

        if(!context.count) {
            printf("Removed none\n");
        } else if(!context.reboot) {
            printf("Removed %d\n", (int)context.count);
        } else {
            printf("Removed %d.  Please reboot.\n", (int)context.count);
            failcode = EXIT_REBOOT;
        }
    }
    return failcode;
}
