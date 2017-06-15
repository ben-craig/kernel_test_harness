/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name:

    devcon.h

Abstract:

    Device Console header

--*/

#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <setupapi.h>
#include <regstr.h>
#include <infstr.h>
#include <cfgmgr32.h>
#include <string.h>
#include <malloc.h>
#include <newdev.h>
#include <objbase.h>
#include <strsafe.h>
#include <io.h>
#include <fcntl.h>

typedef int (*CallbackFunc)(_In_ HDEVINFO Devs, _In_ PSP_DEVINFO_DATA DevInfo, _In_ DWORD Index, _In_ LPVOID Context);

int EnumerateDevices(_In_ DWORD Flags,LPCTSTR hwid, _In_ CallbackFunc Callback, _In_ LPVOID Context);
__drv_allocatesMem(object) LPTSTR * GetDevMultiSz(_In_ HDEVINFO Devs, _In_ PSP_DEVINFO_DATA DevInfo, _In_ DWORD Prop);
__drv_allocatesMem(object) LPTSTR * GetMultiSzIndexArray(_In_ __drv_aliasesMem LPTSTR MultiSz);
void DelMultiSz(_In_opt_ __drv_freesMem(object) PZPWSTR Array);

int cmdInstall(LPCTSTR inf, LPCTSTR hwid);
int cmdRemove(LPCTSTR hwid);

// exit codes
#define EXIT_OK      (0)
#define EXIT_REBOOT  (1)
#define EXIT_FAIL    (2)
#define EXIT_USAGE   (3)

