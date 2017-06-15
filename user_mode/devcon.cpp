/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name:

    devcon.cpp

Abstract:

    Device Console
    command-line interface for managing devices

--*/

#include "devcon.h"

__drv_allocatesMem(object)
LPTSTR * GetMultiSzIndexArray(_In_ __drv_aliasesMem LPTSTR MultiSz)
/*++

Routine Description:

    Get an index array pointing to the MultiSz passed in

Arguments:

    MultiSz - well formed multi-sz string

Return Value:

    array of strings. last entry+1 of array contains NULL
    returns NULL on failure

--*/
{
    LPTSTR scan;
    LPTSTR * array;
    int elements;

    for(scan = MultiSz, elements = 0; scan[0] ;elements++) {
        scan += _tcslen(scan)+1;
    }
    array = new LPTSTR[elements+2];
    if(!array) {
        return NULL;
    }
    array[0] = MultiSz;
    array++;
    if(elements) {
        for(scan = MultiSz, elements = 0; scan[0]; elements++) {
            array[elements] = scan;
            scan += _tcslen(scan)+1;
        }
    }
    array[elements] = NULL;
    return array;
}

void DelMultiSz(_In_opt_ __drv_freesMem(object) PZPWSTR Array)
/*++

Routine Description:

    Deletes the string array allocated by GetDevMultiSz/GetRegMultiSz/GetMultiSzIndexArray

Arguments:

    Array - pointer returned by GetMultiSzIndexArray

Return Value:

    None

--*/
{
    if(Array) {
        Array--;
        if(Array[0]) {
            delete [] Array[0];
        }
        delete [] Array;
    }
}

__drv_allocatesMem(object)
LPTSTR * GetDevMultiSz(_In_ HDEVINFO Devs, _In_ PSP_DEVINFO_DATA DevInfo, _In_ DWORD Prop)
/*++

Routine Description:

    Get a multi-sz device property
    and return as an array of strings

Arguments:

    Devs    - HDEVINFO containing DevInfo
    DevInfo - Specific device
    Prop    - SPDRP_HARDWAREID or SPDRP_COMPATIBLEIDS

Return Value:

    array of strings. last entry+1 of array contains NULL
    returns NULL on failure

--*/
{
    LPTSTR buffer;
    DWORD size;
    DWORD reqSize;
    DWORD dataType;
    LPTSTR * array;
    DWORD szChars;

    size = 8192; // initial guess, nothing magic about this
    buffer = new TCHAR[(size/sizeof(TCHAR))+2];
    if(!buffer) {
        return NULL;
    }
    while(!SetupDiGetDeviceRegistryProperty(Devs,DevInfo,Prop,&dataType,(LPBYTE)buffer,size,&reqSize)) {
        if(GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            goto failed;
        }
        if(dataType != REG_MULTI_SZ) {
            goto failed;
        }
        size = reqSize;
        delete [] buffer;
        buffer = new TCHAR[(size/sizeof(TCHAR))+2];
        if(!buffer) {
            goto failed;
        }
    }
    szChars = reqSize/sizeof(TCHAR);
    buffer[szChars] = TEXT('\0');
    buffer[szChars+1] = TEXT('\0');
    array = GetMultiSzIndexArray(buffer);
    if(array) {
        return array;
    }

failed:
    if(buffer) {
        delete [] buffer;
    }
    return NULL;
}

BOOL AnyMatch(_In_ PZPWSTR Array, _In_ LPCTSTR MatchEntry)
/*++

Routine Description:

    Compares all strings in Array against Id
    Use WildCardMatch to do real compare

Arguments:

    Array - pointer returned by GetDevMultiSz
    MatchEntry - string to compare against

Return Value:

    TRUE if any match, otherwise FALSE

--*/
{
    if(Array) {
        while(Array[0]) {
            if(_tcsicmp(Array[0], MatchEntry) == 0) {
                return TRUE;
            }
            Array++;
        }
    }
    return FALSE;
}

int EnumerateDevices(_In_ DWORD Flags, LPCTSTR hwid, _In_ CallbackFunc Callback, _In_ LPVOID Context)
/*++

Routine Description:

    Enumerator for devices that will be passed the following arguments:
    <id>
    where <id> is a hardware-id

Arguments:

    Flags    - extra enumeration flags (eg DIGCF_PRESENT)
    hwid -     hwid to find
    Callback - function to call for each hit
    Context  - data to pass function for each hit

Return Value:

    EXIT_xxxx

--*/
{
    HDEVINFO devs = INVALID_HANDLE_VALUE;
    int failcode = EXIT_FAIL;
    int retcode;
    DWORD devIndex;
    SP_DEVINFO_DATA devInfo;
    SP_DEVINFO_LIST_DETAIL_DATA devInfoListDetail;
    BOOL match;

    //
    // add all id's to list
    // if there's a class, filter on specified class
    //
    devs = SetupDiGetClassDevsEx(NULL,
                                 NULL,
                                 NULL,
                                 DIGCF_ALLCLASSES | Flags,
                                 NULL,
                                 NULL,
                                 NULL);

    if(devs == INVALID_HANDLE_VALUE) {
        goto final;
    }

    devInfoListDetail.cbSize = sizeof(devInfoListDetail);
    if(!SetupDiGetDeviceInfoListDetail(devs,&devInfoListDetail)) {
        goto final;
    }

    devInfo.cbSize = sizeof(devInfo);
    for(devIndex=0;SetupDiEnumDeviceInfo(devs,devIndex,&devInfo);devIndex++) {
        match = FALSE;
        TCHAR devID[MAX_DEVICE_ID_LEN];
        LPTSTR *hwIds = NULL;
        LPTSTR *compatIds = NULL;
        //
        // determine instance ID
        //
        if(CM_Get_Device_ID_Ex(devInfo.DevInst,devID,MAX_DEVICE_ID_LEN,0,devInfoListDetail.RemoteMachineHandle)!=CR_SUCCESS) {
            devID[0] = TEXT('\0');
        }

        // determine hardware ID's
        // and search for matches
        hwIds = GetDevMultiSz(devs,&devInfo,SPDRP_HARDWAREID);
        compatIds = GetDevMultiSz(devs,&devInfo,SPDRP_COMPATIBLEIDS);

        if(AnyMatch(hwIds,hwid) ||
           AnyMatch(compatIds,hwid)) {
            match = TRUE;
        }
        DelMultiSz(hwIds);
        DelMultiSz(compatIds);

        if(match) {
            retcode = Callback(devs,&devInfo,devIndex,Context);
            if(retcode) {
                failcode = retcode;
                goto final;
            }
        }
    }

    failcode = EXIT_OK;

final:
    if(devs != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(devs);
    }
    return failcode;

}

int
__cdecl
_tmain(_In_ int argc, _In_reads_(argc) PWSTR* argv)
{
   int retval = EXIT_USAGE;
   LPCTSTR hwid = NULL;
   LPCTSTR inf = NULL;

   if (argc<3) {
      printf("insufficient args\n");
      return EXIT_USAGE;
   }
   inf = argv[1];
   if (!inf[0]) {
      printf("inf name required\n");
      return EXIT_USAGE;
   }

   hwid = argv[2];
   if (!hwid[0]) {
      printf("hwid required\n");
      return EXIT_USAGE;
   }

   printf("remove\n");
   if ((retval = cmdRemove(hwid)) != EXIT_OK) goto fail;
   printf("install\n");
   if ((retval = cmdInstall(inf, hwid)) != EXIT_OK) goto fail;
   printf("Installed!\n");

   getchar(); //do stuff here

   printf("remove\n");
   if ((retval = cmdRemove(hwid)) != EXIT_OK) goto fail;

fail:
   switch (retval) {
   case EXIT_USAGE:
      printf("bad usage\n");
      break;
   case EXIT_REBOOT:
      printf("reboot needed\n");
      break;
   case EXIT_OK:
      break;
   case EXIT_FAIL:
      printf("fail\n");
      break;
   default:
      printf("unknown error %d\n", retval);
      break;
   }
   return retval;
}
