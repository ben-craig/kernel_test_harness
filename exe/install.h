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
