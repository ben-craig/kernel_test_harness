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

typedef int (*DispatchFunc)(_In_ LPCTSTR BaseName, _In_opt_ LPCTSTR Machine, _In_ DWORD Flags, _In_ int argc, _In_ LPTSTR argv[]);
typedef int (*CallbackFunc)(_In_ HDEVINFO Devs, _In_ PSP_DEVINFO_DATA DevInfo, _In_ DWORD Index, _In_ LPVOID Context);

typedef struct {
    LPCTSTR         cmd;
    DispatchFunc    func;
} DispatchEntry;

extern DispatchEntry DispatchTable[];

#define INSTANCEID_PREFIX_CHAR TEXT('@') // character used to prefix instance ID's
#define CLASS_PREFIX_CHAR      TEXT('=') // character used to prefix class name
#define WILD_CHAR              TEXT('*') // wild character
#define QUOTE_PREFIX_CHAR      TEXT('\'') // prefix character to ignore wild characters

//
// Devcon.exe command line flags
//
#define DEVCON_FLAG_FORCE       0x00000001

//void FormatToStream(_In_ FILE * stream, _In_ DWORD fmt,...);
# define Stringify(s) #s
# define FormatToStream(file, fmt, ...) fprintf(file, "%s", Stringify(fmt))

int EnumerateDevices(_In_ LPCTSTR BaseName, _In_opt_ LPCTSTR Machine, _In_ DWORD Flags, _In_ int argc, _In_reads_(argc) PWSTR* argv, _In_ CallbackFunc Callback, _In_ LPVOID Context);
__drv_allocatesMem(object) LPTSTR * GetDevMultiSz(_In_ HDEVINFO Devs, _In_ PSP_DEVINFO_DATA DevInfo, _In_ DWORD Prop);
__drv_allocatesMem(object) LPTSTR * GetMultiSzIndexArray(_In_ __drv_aliasesMem LPTSTR MultiSz);
void DelMultiSz(_In_opt_ __drv_freesMem(object) PZPWSTR Array);

BOOL Reboot();


//
// UpdateDriverForPlugAndPlayDevices
//
typedef BOOL (WINAPI *UpdateDriverForPlugAndPlayDevicesProto)(_In_opt_ HWND hwndParent,
                                                              _In_ LPCTSTR HardwareId,
                                                              _In_ LPCTSTR FullInfPath,
                                                              _In_ DWORD InstallFlags,
                                                              _Out_opt_ PBOOL bRebootRequired
                                                         );

#ifdef _UNICODE
#define UPDATEDRIVERFORPLUGANDPLAYDEVICES "UpdateDriverForPlugAndPlayDevicesW"
#else
#define UPDATEDRIVERFORPLUGANDPLAYDEVICES "UpdateDriverForPlugAndPlayDevicesA"
#endif

//
// exit codes
//
#define EXIT_OK      (0)
#define EXIT_REBOOT  (1)
#define EXIT_FAIL    (2)
#define EXIT_USAGE   (3)

