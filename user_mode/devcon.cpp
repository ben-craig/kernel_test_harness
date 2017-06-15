/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name:

    devcon.cpp

Abstract:

    Device Console
    command-line interface for managing devices

--*/

#include "devcon.h"

struct IdEntry {
    LPCTSTR String;     // string looking for
    LPCTSTR Wild;       // first wild character if any
    BOOL    InstanceId;
};

BOOL Reboot()
/*++

Routine Description:

    Attempt to reboot computer

Arguments:

    none

Return Value:

    TRUE if API suceeded

--*/
{
    HANDLE Token;
    TOKEN_PRIVILEGES NewPrivileges;
    LUID Luid;

    //
    // we need to "turn on" reboot privilege
    // if any of this fails, try reboot anyway
    //
    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&Token)) {
        goto final;
    }

    if(!LookupPrivilegeValue(NULL,SE_SHUTDOWN_NAME,&Luid)) {
        CloseHandle(Token);
        goto final;
    }

    NewPrivileges.PrivilegeCount = 1;
    NewPrivileges.Privileges[0].Luid = Luid;
    NewPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(
            Token,
            FALSE,
            &NewPrivileges,
            0,
            NULL,
            NULL
            );

    CloseHandle(Token);

final:

    //
    // attempt reboot - inform system that this is planned hardware install
    //
    // warning 28159 is a warning to rearchitect to avoid rebooting.  However,
    // sometimes during device installation, a reboot is needed, so this warning
    // is being suppressed for the call to InitiateSystemShutdownEx.
    //
#pragma warning( suppress: 28159)
    return InitiateSystemShutdownEx(NULL,
                                    NULL,
                                    0,
                                    FALSE,
                                    TRUE,
                                    REASON_PLANNED_FLAG | REASON_HWINSTALL);

}

IdEntry GetIdType(_In_ LPCTSTR Id)
/*++

Routine Description:

    Determine if this is instance id or hardware id and if there's any wildcards
    instance ID is prefixed by '@'
    wildcards are '*'


Arguments:

    Id - ptr to string to check

Return Value:

    IdEntry

--*/
{
    IdEntry Entry;

    Entry.InstanceId = FALSE;
    Entry.Wild = NULL;
    Entry.String = Id;

    if(Entry.String[0] == INSTANCEID_PREFIX_CHAR) {
        Entry.InstanceId = TRUE;
        Entry.String = CharNext(Entry.String);
    }
    if(Entry.String[0] == QUOTE_PREFIX_CHAR) {
        //
        // prefix to treat rest of string literally
        //
        Entry.String = CharNext(Entry.String);
    } else {
        //
        // see if any wild characters exist
        //
        Entry.Wild = _tcschr(Entry.String,WILD_CHAR);
    }
    return Entry;
}

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

BOOL WildCardMatch(_In_ LPCTSTR Item, _In_ const IdEntry & MatchEntry)
/*++

Routine Description:

    Compare a single item against wildcard
    I'm sure there's better ways of implementing this
    Other than a command-line management tools
    it's a bad idea to use wildcards as it implies
    assumptions about the hardware/instance ID
    eg, it might be tempting to enumerate root\* to
    find all root devices, however there is a CfgMgr
    API to query status and determine if a device is
    root enumerated, which doesn't rely on implementation
    details.

Arguments:

    Item - item to find match for eg a\abcd\c
    MatchEntry - eg *\*bc*\*

Return Value:

    TRUE if any match, otherwise FALSE

--*/
{
    LPCTSTR scanItem;
    LPCTSTR wildMark;
    LPCTSTR nextWild;
    size_t matchlen;

    //
    // before attempting anything else
    // try and compare everything up to first wild
    //
    if(!MatchEntry.Wild) {
        return _tcsicmp(Item,MatchEntry.String) ? FALSE : TRUE;
    }
    if(_tcsnicmp(Item,MatchEntry.String,MatchEntry.Wild-MatchEntry.String) != 0) {
        return FALSE;
    }
    wildMark = MatchEntry.Wild;
    scanItem = Item + (MatchEntry.Wild-MatchEntry.String);

    for(;wildMark[0];) {
        //
        // if we get here, we're either at or past a wildcard
        //
        if(wildMark[0] == WILD_CHAR) {
            //
            // so skip wild chars
            //
            wildMark = CharNext(wildMark);
            continue;
        }
        //
        // find next wild-card
        //
        nextWild = _tcschr(wildMark,WILD_CHAR);
        if(nextWild) {
            //
            // substring
            //
            matchlen = nextWild-wildMark;
        } else {
            //
            // last portion of match
            //
            size_t scanlen = _tcslen(scanItem);
            matchlen = _tcslen(wildMark);
            if(scanlen < matchlen) {
                return FALSE;
            }
            return _tcsicmp(scanItem+scanlen-matchlen,wildMark) ? FALSE : TRUE;
        }
        if(_istalpha(wildMark[0])) {
            //
            // scan for either lower or uppercase version of first character
            //

            //
            // the code suppresses the warning 28193 for the calls to _totupper
            // and _totlower.  This suppression is done because those functions
            // have a check return annotation on them.  However, they don't return
            // error codes and the check return annotation is really being used
            // to indicate that the return value of the function should be looked
            // at and/or assigned to a variable.  The check return annotation means
            // the return value should always be checked in all code paths.
            // We assign the return values to variables but the while loop does not
            // examine both values in all code paths (e.g. when scanItem[0] == 0,
            // neither u nor l will be examined) and it doesn't need to examine
            // the values in all code paths.
            //
#pragma warning( suppress: 28193)
            TCHAR u = _totupper(wildMark[0]);
#pragma warning( suppress: 28193)
            TCHAR l = _totlower(wildMark[0]);
            while(scanItem[0] && scanItem[0]!=u && scanItem[0]!=l) {
                scanItem = CharNext(scanItem);
            }
            if(!scanItem[0]) {
                //
                // ran out of string
                //
                return FALSE;
            }
        } else {
            //
            // scan for first character (no case)
            //
            scanItem = _tcschr(scanItem,wildMark[0]);
            if(!scanItem) {
                //
                // ran out of string
                //
                return FALSE;
            }
        }
        //
        // try and match the sub-string at wildMark against scanItem
        //
        if(_tcsnicmp(scanItem,wildMark,matchlen)!=0) {
            //
            // nope, try again
            //
            scanItem = CharNext(scanItem);
            continue;
        }
        //
        // substring matched
        //
        scanItem += matchlen;
        wildMark += matchlen;
    }
    return (wildMark[0] ? FALSE : TRUE);
}

BOOL WildCompareHwIds(_In_ PZPWSTR Array, _In_ const IdEntry & MatchEntry)
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
            if(WildCardMatch(Array[0],MatchEntry)) {
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

    Generic enumerator for devices that will be passed the following arguments:
    <id> [<id>...]
    =<class> [<id>...]
    where <id> can either be @instance-id, or hardware-id and may contain wildcards
    <class> is a class name

Arguments:

    Machine  - name of machine to enumerate
    Flags    - extra enumeration flags (eg DIGCF_PRESENT)
    argc/argv - remaining arguments on command line
    Callback - function to call for each hit
    Context  - data to pass function for each hit

Return Value:

    EXIT_xxxx

--*/
{
    HDEVINFO devs = INVALID_HANDLE_VALUE;
    IdEntry templ;
    int failcode = EXIT_FAIL;
    int retcode;
    DWORD devIndex;
    SP_DEVINFO_DATA devInfo;
    SP_DEVINFO_LIST_DETAIL_DATA devInfoListDetail;
    BOOL doSearch = FALSE;
    BOOL match;
    GUID cls;
    DWORD numClass = 0;
    int skip = 0;

    //
    // determine if any instance id's were specified
    //
    // note, if =<class> was specified with no id's
    // we'll mark it as not doSearch
    // but will go ahead and add them all
    //
    templ = GetIdType(hwid);
    if(templ.Wild || !templ.InstanceId) {
        //
        // anything other than simple InstanceId's require a search
        //
        doSearch = TRUE;
    }
    if(doSearch) {
        //
        // add all id's to list
        // if there's a class, filter on specified class
        //
        devs = SetupDiGetClassDevsEx(numClass ? &cls : NULL,
                                     NULL,
                                     NULL,
                                     (numClass ? 0 : DIGCF_ALLCLASSES) | Flags,
                                     NULL,
                                     NULL,
                                     NULL);

    } else {
        //
        // blank list, we'll add instance id's by hand
        //
        devs = SetupDiCreateDeviceInfoListEx(numClass ? &cls : NULL,
                                             NULL,
                                             NULL,
                                             NULL);
    }
    if(devs == INVALID_HANDLE_VALUE) {
        goto final;
    }
    //
    // add explicit instances to list (even if enumerated all,
    // this gets around DIGCF_PRESENT)
    // do this even if wildcards appear to be detected since they
    // might actually be part of the instance ID of a non-present device
    //
    if(templ.InstanceId) {
        SetupDiOpenDeviceInfo(devs,templ.String,NULL,0,NULL);
    }

    devInfoListDetail.cbSize = sizeof(devInfoListDetail);
    if(!SetupDiGetDeviceInfoListDetail(devs,&devInfoListDetail)) {
        goto final;
    }

    devInfo.cbSize = sizeof(devInfo);
    for(devIndex=0;SetupDiEnumDeviceInfo(devs,devIndex,&devInfo);devIndex++) {

        if(doSearch) {
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

            if(templ.InstanceId) {
                //
                // match on the instance ID
                //
                if(WildCardMatch(devID,templ)) {
                    match = TRUE;
                }
            } else {
                //
                // determine hardware ID's
                // and search for matches
                //
                hwIds = GetDevMultiSz(devs,&devInfo,SPDRP_HARDWAREID);
                compatIds = GetDevMultiSz(devs,&devInfo,SPDRP_COMPATIBLEIDS);

                if(WildCompareHwIds(hwIds,templ) ||
                    WildCompareHwIds(compatIds,templ)) {
                    match = TRUE;
                }
            }
            DelMultiSz(hwIds);
            DelMultiSz(compatIds);
        } else {
            match = TRUE;
        }
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
   getchar();
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
