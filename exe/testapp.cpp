#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <cstdlib>

#include "sys\sioctl.h"
#include "install.h"

TestResults OutputBuffer;
char InputBuffer[1];

class DriverLoader
{
private:
    const char  *loc;
public:
    explicit DriverLoader(const char *location) : 
        loc(location) 
    {
        ManageDriver(DRIVER_NAME, loc, DRIVER_FUNC_QUIET_REMOVE);
        BOOLEAN result =
            ManageDriver(DRIVER_NAME, loc, DRIVER_FUNC_INSTALL);
        if (!result)
        {
            printf("Unable to install driver. \n");
            ManageDriver(DRIVER_NAME, loc, DRIVER_FUNC_REMOVE);
            std::exit(-2);
        }
    }
    ~DriverLoader() 
    {
        ManageDriver(DRIVER_NAME, loc, DRIVER_FUNC_REMOVE);            
    }
};

struct DeviceOpener {
    HANDLE h = INVALID_HANDLE_VALUE;
    DeviceOpener()
    {
        h = CreateFile( "\\\\.\\IoctlTest",
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);

        if ( h == INVALID_HANDLE_VALUE ){
            printf ( "Error: CreateFile Failed : %d\n", GetLastError());
            std::exit(-2);
        }
    }
    ~DeviceOpener() {CloseHandle ( h );}
};

int __cdecl main()
{
    BOOL bRc;
    ULONG bytesReturned;
    TCHAR driverLocation[MAX_PATH];

    if (!SetupDriverName(driverLocation, sizeof(driverLocation))) {
        printf ( "Error in SetupDriverName : %d", GetLastError());
        return -2;
    }
    DriverLoader d(driverLocation);
    DeviceOpener dev;

    bRc = DeviceIoControl ( dev.h,
                            (DWORD) IOCTL_SIOCTL_METHOD_RUN_TEST,
                            &InputBuffer,
                            sizeof(InputBuffer),
                            &OutputBuffer,
                            sizeof( OutputBuffer),
                            &bytesReturned,
                            NULL
                            );

    if ( !bRc )
    {
        printf ( "Error in DeviceIoControl : %d", GetLastError());
        return -2;
    }

    if(OutputBuffer.main_return != 0 || OutputBuffer.tests_failed != 0)
    {
        printf("retval: %d\ntests failed: %d\noutput =\n%s\n",
            OutputBuffer.main_return,
            OutputBuffer.tests_failed,
            OutputBuffer.output);
        return 1;
    }
    printf("retval: %d\ntests failed: %d\noutput =\n%s\n",
        OutputBuffer.main_return,
        OutputBuffer.tests_failed,
        OutputBuffer.output);
    return 0;
}
