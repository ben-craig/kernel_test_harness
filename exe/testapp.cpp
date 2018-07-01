#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <cstdlib>
#include <string>

#include "sys\sioctl.h"
#include "install.h"

TestResults OutputBuffer;
char InputBuffer[1];

class DriverLoader
{
private:
    const char *loc;
    const char *name;
public:
    explicit DriverLoader(const char *location, const char *driver_name) :
        loc(location),
        name(driver_name)
    {
        ManageDriver(name, loc, DRIVER_FUNC_QUIET_REMOVE);
        BOOLEAN result =
            ManageDriver(name, loc, DRIVER_FUNC_INSTALL);
        if (!result)
        {
            printf("Unable to install driver. \n");
            ManageDriver(name, loc, DRIVER_FUNC_REMOVE);
            std::exit(-2);
        }
    }
    ~DriverLoader()
    {
        ManageDriver(name, loc, DRIVER_FUNC_REMOVE);
    }
};

struct DeviceOpener {
    HANDLE h = INVALID_HANDLE_VALUE;
    explicit DeviceOpener(const char *driverName)
    {
        std::string dosName = "\\\\.\\";
        dosName += driverName;
        h = CreateFile( dosName.c_str(),
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

int __cdecl main(int argc, char *argv[])
{
    if(argc < 3) {
        printf( "Usage: %s <some\\path\\foo.sys> <driver_dos_name>\n", argv[0]);
        return -2;
    }
    BOOL bRc;
    ULONG bytesReturned;
    char filePathBuf[MAX_PATH] = {0};
    ::GetFullPathNameA(argv[1], sizeof(filePathBuf), filePathBuf, nullptr);
    DriverLoader d(filePathBuf, argv[2]);
    DeviceOpener dev(argv[2]);

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
