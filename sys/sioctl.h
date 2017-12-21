#pragma once

#define SIOCTL_TYPE 40000

#define IOCTL_SIOCTL_METHOD_RUN_TEST \
    CTL_CODE( SIOCTL_TYPE, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS  )

#define DRIVER_FUNC_INSTALL       0x01
#define DRIVER_FUNC_REMOVE        0x02
#define DRIVER_FUNC_QUIET_REMOVE  0x03

#define DRIVER_NAME       "sioctl"

typedef struct TestResults {
    int main_return;
    long tests_failed;
    char output[4088];
} TestResults;
