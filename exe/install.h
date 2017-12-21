#pragma once

#include <windows.h>
#include <winioctl.h>

#ifdef __cplusplus
extern "C"
#endif
BOOLEAN
ManageDriver(
    _In_ LPCTSTR  DriverName,
    _In_ LPCTSTR  ServiceName,
    _In_ USHORT   Function
    );

#ifdef __cplusplus
extern "C"
#endif
BOOLEAN
SetupDriverName(
    _Inout_updates_bytes_all_(BufferLength) PCHAR DriverLocation,
    _In_ ULONG BufferLength
    );
