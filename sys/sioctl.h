/*++

Copyright (c) 1997  Microsoft Corporation

Module Name:

    SIOCTL.H

Abstract:


    Defines the IOCTL codes that will be used by this driver.  The IOCTL code
    contains a command identifier, plus other information about the device,
    the type of access with which the file must have been opened,
    and the type of buffering.

Environment:

    Kernel mode only.

--*/

//
// Device type           -- in the "User Defined" range."
//
#define SIOCTL_TYPE 40000
//
// The IOCTL function codes from 0x800 to 0xFFF are for customer use.
//
#define IOCTL_SIOCTL_METHOD_BUFFERED \
    CTL_CODE( SIOCTL_TYPE, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS  )


#define DRIVER_FUNC_INSTALL       0x01
#define DRIVER_FUNC_REMOVE        0x02
#define DRIVER_FUNC_QUIET_REMOVE  0x03

#define DRIVER_NAME       "sioctl"

