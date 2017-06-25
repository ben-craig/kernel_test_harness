#include <ntddk.h>
#include <wdf.h>

#define NTSTRSAFE_LIB
#include <ntstrsafe.h>
#include <wdmsec.h> // for SDDLs
#include "public.h" // contains IOCTL definitions

#define NTDEVICE_NAME_STRING      L"\\Device\\LIBCXX"
#define SYMBOLIC_NAME_STRING     L"\\DosDevices\\LIBCXX"
#define POOL_TAG                   'ELIF'

typedef struct _CONTROL_DEVICE_EXTENSION {

    //TODO: probably delete
    HANDLE   FileHandle; // Store your control data here

} CONTROL_DEVICE_EXTENSION, *PCONTROL_DEVICE_EXTENSION;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(CONTROL_DEVICE_EXTENSION,
                                        ControlGetData)

//
// Device driver routine declarations.
//

DRIVER_INITIALIZE DriverEntry;

//
// Don't use EVT_WDF_DRIVER_DEVICE_ADD for NonPnpDeviceAdd even though
// the signature is same because this is not an event called by the
// framework.
//
NTSTATUS
NonPnpDeviceAdd(
    IN WDFDRIVER Driver,
    IN PWDFDEVICE_INIT DeviceInit
    );

EVT_WDF_DRIVER_UNLOAD NonPnpEvtDriverUnload;

EVT_WDF_DEVICE_CONTEXT_CLEANUP NonPnpEvtDriverContextCleanup;
EVT_WDF_DEVICE_SHUTDOWN_NOTIFICATION NonPnpShutdown;

EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL FileEvtIoDeviceControl;
EVT_WDF_IO_QUEUE_IO_READ FileEvtIoRead;
EVT_WDF_IO_QUEUE_IO_WRITE FileEvtIoWrite;

EVT_WDF_DEVICE_FILE_CREATE NonPnpEvtDeviceFileCreate;
EVT_WDF_FILE_CLOSE NonPnpEvtFileClose;

#pragma warning(disable:4127)

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, DriverEntry )
#pragma alloc_text( PAGE, NonPnpDeviceAdd)
#pragma alloc_text( PAGE, NonPnpEvtDriverContextCleanup)
#pragma alloc_text( PAGE, NonPnpEvtDriverUnload)
#pragma alloc_text( PAGE, NonPnpEvtDeviceFileCreate)
#pragma alloc_text( PAGE, NonPnpEvtFileClose)
#pragma alloc_text( PAGE, FileEvtIoRead)
#pragma alloc_text( PAGE, FileEvtIoWrite)
#pragma alloc_text( PAGE, FileEvtIoDeviceControl)
#endif // ALLOC_PRAGMA

int __cdecl main();

NTSTATUS
DriverEntry(
    IN OUT PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING      RegistryPath
    )
/*++

Routine Description:
    This routine is called by the Operating System to initialize the driver.

    It creates the device object, fills in the dispatch entry points and
    completes the initialization.

Arguments:
    DriverObject - a pointer to the object that represents this device
    driver.

    RegistryPath - a pointer to our Services key in the registry.

Return Value:
    STATUS_SUCCESS if initialized; an error otherwise.

--*/
{
    NTSTATUS                       status;
    WDF_DRIVER_CONFIG              config;
    WDFDRIVER                      hDriver;
    PWDFDEVICE_INIT                pInit = NULL;
    WDF_OBJECT_ATTRIBUTES          attributes;

    KdPrint(("Libcxx test harness driver\n"));


    WDF_DRIVER_CONFIG_INIT(
        &config,
        WDF_NO_EVENT_CALLBACK // This is a non-pnp driver.
        );

    //
    // Tell the framework that this is non-pnp driver so that it doesn't
    // set the default AddDevice routine.
    //
    config.DriverInitFlags |= WdfDriverInitNonPnpDriver;

    //
    // NonPnp driver must explicitly register an unload routine for
    // the driver to be unloaded.
    //
    config.EvtDriverUnload = NonPnpEvtDriverUnload;

    //
    // Register a cleanup callback so that we can call WPP_CLEANUP when
    // the framework driver object is deleted during driver unload.
    //
    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.EvtCleanupCallback = NonPnpEvtDriverContextCleanup;

    //
    // Create a framework driver object to represent our driver.
    //
    status = WdfDriverCreate(DriverObject,
                            RegistryPath,
                            &attributes,
                            &config,
                            &hDriver);
    if (!NT_SUCCESS(status)) {
        KdPrint (("LibCxx: WdfDriverCreate failed with status 0x%x\n", status));
        return status;
    }

    //
    // On Win2K system,  you will experience some delay in getting trace events
    // due to the way the ETW is activated to accept trace messages.
    //
    KdPrint(("LibCxx: DriverEntry: tracing enabled\n"));

    //
    //
    // In order to create a control device, we first need to allocate a
    // WDFDEVICE_INIT structure and set all properties.
    //
    pInit = WdfControlDeviceInitAllocate(
                            hDriver,
                            &SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RW_RES_R
                            );

    if (pInit == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        return status;
    }

    //
    // Call NonPnpDeviceAdd to create a deviceobject to represent our
    // software device.
    //
    status = NonPnpDeviceAdd(hDriver, pInit);

    return status;
}

NTSTATUS
NonPnpDeviceAdd(
    IN WDFDRIVER Driver,
    IN PWDFDEVICE_INIT DeviceInit
    )
/*++

Routine Description:

    Called by the DriverEntry to create a control-device. This call is
    responsible for freeing the memory for DeviceInit.

Arguments:

    DriverObject - a pointer to the object that represents this device
    driver.

    DeviceInit - Pointer to a driver-allocated WDFDEVICE_INIT structure.

Return Value:

    STATUS_SUCCESS if initialized; an error otherwise.

--*/
{
    NTSTATUS                       status;
    WDF_OBJECT_ATTRIBUTES           attributes;
    WDF_IO_QUEUE_CONFIG      ioQueueConfig;
    WDF_FILEOBJECT_CONFIG fileConfig;
    WDFQUEUE                            queue;
    WDFDEVICE   controlDevice;
    DECLARE_CONST_UNICODE_STRING(ntDeviceName, NTDEVICE_NAME_STRING) ;
    DECLARE_CONST_UNICODE_STRING(symbolicLinkName, SYMBOLIC_NAME_STRING) ;

    UNREFERENCED_PARAMETER( Driver );

    PAGED_CODE();

    // Set exclusive to TRUE so that no more than one app can talk to the
    // control device at any time.
    //
    WdfDeviceInitSetExclusive(DeviceInit, TRUE);

    WdfDeviceInitSetIoType(DeviceInit, WdfDeviceIoBuffered);


    status = WdfDeviceInitAssignName(DeviceInit, &ntDeviceName);

    if (!NT_SUCCESS(status)) {
        goto End;
    }

    WdfControlDeviceInitSetShutdownNotification(DeviceInit,
                                                NonPnpShutdown,
                                                WdfDeviceShutdown);

    //
    // Initialize WDF_FILEOBJECT_CONFIG_INIT struct to tell the
    // framework whether you are interested in handling Create, Close and
    // Cleanup requests that gets generated when an application or another
    // kernel component opens an handle to the device. If you don't register
    // the framework default behaviour would be to complete these requests
    // with STATUS_SUCCESS. A driver might be interested in registering these
    // events if it wants to do security validation and also wants to maintain
    // per handle (fileobject) context.
    //

    WDF_FILEOBJECT_CONFIG_INIT(
                        &fileConfig,
                        NonPnpEvtDeviceFileCreate,
                        NonPnpEvtFileClose,
                        WDF_NO_EVENT_CALLBACK // not interested in Cleanup
                        );

    WdfDeviceInitSetFileObjectConfig(DeviceInit,
                                       &fileConfig,
                                       WDF_NO_OBJECT_ATTRIBUTES);

    //
    // Specify the size of device context
    //
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes,
                                    CONTROL_DEVICE_EXTENSION);

    status = WdfDeviceCreate(&DeviceInit,
                             &attributes,
                             &controlDevice);
    if (!NT_SUCCESS(status)) {
        goto End;
    }

    //
    // Create a symbolic link for the control object so that usermode can open
    // the device.
    //


    status = WdfDeviceCreateSymbolicLink(controlDevice,
                                &symbolicLinkName);

    if (!NT_SUCCESS(status)) {
        goto End;
    }

    //
    // Configure a default queue so that requests that are not
    // configure-fowarded using WdfDeviceConfigureRequestDispatching to goto
    // other queues get dispatched here.
    //
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&ioQueueConfig,
                                    WdfIoQueueDispatchSequential);

    ioQueueConfig.EvtIoRead = FileEvtIoRead;
    ioQueueConfig.EvtIoWrite = FileEvtIoWrite;
    ioQueueConfig.EvtIoDeviceControl = FileEvtIoDeviceControl;

    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    //
    // Since we are using Zw function set execution level to passive so that
    // framework ensures that our Io callbacks called at only passive-level
    // even if the request came in at DISPATCH_LEVEL from another driver.
    //
    //attributes.ExecutionLevel = WdfExecutionLevelPassive;

    //
    // By default, Static Driver Verifier (SDV) displays a warning if it
    // doesn't find the EvtIoStop callback on a power-managed queue.
    // The 'assume' below causes SDV to suppress this warning. If the driver
    // has not explicitly set PowerManaged to WdfFalse, the framework creates
    // power-managed queues when the device is not a filter driver.  Normally
    // the EvtIoStop is required for power-managed queues, but for this driver
    // it is not needed b/c the driver doesn't hold on to the requests or
    // forward them to other drivers. This driver completes the requests
    // directly in the queue's handlers. If the EvtIoStop callback is not
    // implemented, the framework waits for all driver-owned requests to be
    // done before moving in the Dx/sleep states or before removing the
    // device, which is the correct behavior for this type of driver.
    // If the requests were taking an indeterminate amount of time to complete,
    // or if the driver forwarded the requests to a lower driver/another stack,
    // the queue should have an EvtIoStop/EvtIoResume.
    //
    __analysis_assume(ioQueueConfig.EvtIoStop != 0);
    status = WdfIoQueueCreate(controlDevice,
                              &ioQueueConfig,
                              &attributes,
                              &queue // pointer to default queue
                              );
    __analysis_assume(ioQueueConfig.EvtIoStop == 0);
    if (!NT_SUCCESS(status)) {
        goto End;
    }

    //
    // Control devices must notify WDF when they are done initializing.   I/O is
    // rejected until this call is made.
    //
    WdfControlFinishInitializing(controlDevice);

End:
    //
    // If the device is created successfully, framework would clear the
    // DeviceInit value. Otherwise device create must have failed so we
    // should free the memory ourself.
    //
    if (DeviceInit != NULL) {
        WdfDeviceInitFree(DeviceInit);
    }

    return status;

}

VOID
NonPnpEvtDriverContextCleanup(
    IN WDFOBJECT Driver
    )
/*++
Routine Description:

   Called when the driver object is deleted during driver unload.
   You can free all the resources created in DriverEntry that are
   not automatically freed by the framework.

Arguments:

    Driver - Handle to a framework driver object created in DriverEntry

Return Value:

    NTSTATUS

--*/
{
    PAGED_CODE();

}



VOID
NonPnpEvtDeviceFileCreate (
    IN WDFDEVICE            Device,
    IN WDFREQUEST Request,
    IN WDFFILEOBJECT        FileObject
    )
/*++

Routine Description:

    The framework calls a driver's EvtDeviceFileCreate callback
    when it receives an IRP_MJ_CREATE request.
    The system sends this request when a user application opens the
    device to perform an I/O operation, such as reading or writing a file.
    This callback is called synchronously, in the context of the thread
    that created the IRP_MJ_CREATE request.

Arguments:

    Device - Handle to a framework device object.
    FileObject - Pointer to fileobject that represents the open handle.
    CreateParams - Parameters of IO_STACK_LOCATION for create

Return Value:

   NT status code

--*/
{
    PUNICODE_STRING             fileName;
    UNICODE_STRING              absFileName, directory;
    OBJECT_ATTRIBUTES           fileAttributes;
    IO_STATUS_BLOCK             ioStatus;
    PCONTROL_DEVICE_EXTENSION   devExt;
    NTSTATUS                    status;
    USHORT                      length = 0;


    UNREFERENCED_PARAMETER( FileObject );

    PAGED_CODE ();

    devExt = ControlGetData(Device);

    //
    // Assume the directory is a temp directory under %windir%
    //
    RtlInitUnicodeString(&directory, L"\\SystemRoot\\temp");

    //
    // Parsed filename has "\" in the begining. The object manager strips
    // of all "\", except one, after the device name.
    //
    fileName = WdfFileObjectGetFileName(FileObject);

    // Find the total length of the directory + filename
    //
    length = directory.Length + fileName->Length;

    absFileName.Buffer = ExAllocatePoolWithTag(PagedPool, length, POOL_TAG);
    if(absFileName.Buffer == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto End;
    }
    absFileName.Length = 0;
    absFileName.MaximumLength =  length;

    status = RtlAppendUnicodeStringToString(&absFileName, &directory);
    if (!NT_SUCCESS(status)) {
        goto End;
    }

    status = RtlAppendUnicodeStringToString(&absFileName, fileName);
    if (!NT_SUCCESS(status)) {
        goto End;
    }

    InitializeObjectAttributes( &fileAttributes,
                                &absFileName,
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                NULL, // RootDirectory
                                NULL // SecurityDescriptor
                                );

    status = ZwCreateFile (
                    &devExt->FileHandle,
                    SYNCHRONIZE | GENERIC_WRITE | GENERIC_READ,
                    &fileAttributes,
                    &ioStatus,
                    NULL,// alloc size = none
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ,
                    FILE_OPEN_IF,
                    FILE_SYNCHRONOUS_IO_NONALERT |FILE_NON_DIRECTORY_FILE,
                    NULL,// eabuffer
                    0// ealength
                    );

    if (!NT_SUCCESS(status)) {

        devExt->FileHandle = NULL;
    }

End:
    if(absFileName.Buffer != NULL) {
        ExFreePool(absFileName.Buffer);
    }

    WdfRequestComplete(Request, status);

    return;
}


VOID
NonPnpEvtFileClose (
    IN WDFFILEOBJECT    FileObject
    )

/*++

Routine Description:

   EvtFileClose is called when all the handles represented by the FileObject
   is closed and all the references to FileObject is removed. This callback
   may get called in an arbitrary thread context instead of the thread that
   called CloseHandle. If you want to delete any per FileObject context that
   must be done in the context of the user thread that made the Create call,
   you should do that in the EvtDeviceCleanp callback.

Arguments:

    FileObject - Pointer to fileobject that represents the open handle.

Return Value:

   VOID

--*/
{
    PCONTROL_DEVICE_EXTENSION devExt;

    PAGED_CODE ();

    devExt = ControlGetData(WdfFileObjectGetDevice(FileObject));

    if(devExt->FileHandle) {
        ZwClose(devExt->FileHandle);
    }

    return;
}


VOID
FileEvtIoRead(
    IN WDFQUEUE         Queue,
    IN WDFREQUEST       Request,
    IN size_t            Length
    )
/*++

Routine Description:

    This event is called when the framework receives IRP_MJ_READ requests.
    We will just read the file.

Arguments:

    Queue -  Handle to the framework queue object that is associated with the
            I/O request.
    Request - Handle to a framework request object.

    Length  - number of bytes to be read.
                   Queue is by default configured to fail zero length read & write requests.

Return Value:

  None.

--*/
{
    ULONG_PTR bytesRead = 0;
    PAGED_CODE ();
    WdfRequestCompleteWithInformation(Request, STATUS_INVALID_DEVICE_REQUEST, bytesRead);
}



VOID
FileEvtIoWrite(
    IN WDFQUEUE         Queue,
    IN WDFREQUEST       Request,
    IN size_t            Length
    )
/*++

Routine Description:

    This event is called when the framework receives IRP_MJ_WRITE requests.

Arguments:

    Queue -  Handle to the framework queue object that is associated with the
            I/O request.
    Request - Handle to a framework request object.

    Length  - number of bytes to be written.
                   Queue is by default configured to fail zero length read & write requests.


Return Value:

   None
--*/
{
    ULONG_PTR bytesWritten = 0;
    PAGED_CODE ();
    WdfRequestCompleteWithInformation(Request, STATUS_INVALID_DEVICE_REQUEST, bytesWritten);
}

VOID
FileEvtIoDeviceControl(
    IN WDFQUEUE         Queue,
    IN WDFREQUEST       Request,
    IN size_t            OutputBufferLength,
    IN size_t            InputBufferLength,
    IN ULONG            IoControlCode
    )
/*++
Routine Description:

    This event is called when the framework receives IRP_MJ_DEVICE_CONTROL
    requests from the system.

Arguments:

    Queue - Handle to the framework queue object that is associated
            with the I/O request.
    Request - Handle to a framework request object.

    OutputBufferLength - length of the request's output buffer,
                        if an output buffer is available.
    InputBufferLength - length of the request's input buffer,
                        if an input buffer is available.

    IoControlCode - the driver-defined or system-defined I/O control code
                    (IOCTL) that is associated with the request.

Return Value:

   VOID

--*/
{
    NTSTATUS            status = STATUS_SUCCESS;// Assume success
    PCHAR               inBuf = NULL, outBuf = NULL; // pointer to Input and output buffer
    PCHAR               buffer = NULL;
    size_t               bufSize;

    UNREFERENCED_PARAMETER( Queue );

    PAGED_CODE();

    if(OutputBufferLength < 4 || !InputBufferLength)
    {
        WdfRequestComplete(Request, STATUS_INVALID_PARAMETER);
        return;
    }

    //
    // Determine which I/O control code was specified.
    //

    switch (IoControlCode)
    {
    case IOCTL_NONPNP_METHOD_BUFFERED:
        //
        // For bufffered ioctls WdfRequestRetrieveInputBuffer &
        // WdfRequestRetrieveOutputBuffer return the same buffer
        // pointer (Irp->AssociatedIrp.SystemBuffer), so read the
        // content of the buffer before writing to it.
        //
        status = WdfRequestRetrieveInputBuffer(Request, 0, &inBuf, &bufSize);
        if(!NT_SUCCESS(status)) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        ASSERT(bufSize == InputBufferLength);

        //
        // TODO: Read the input buffer content.
        int result = main();


        status = WdfRequestRetrieveOutputBuffer(Request, 0, &outBuf, &bufSize);
        if(!NT_SUCCESS(status)) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        ASSERT(bufSize == OutputBufferLength);

        //
        // TODO: copy result to outBuf
        //

        RtlCopyMemory(outBuf, (PCHAR)&result, sizeof(result));


        //
        // Assign the length of the data copied to IoStatus.Information
        // of the request and complete the request.
        //
        WdfRequestSetInformation(Request, sizeof(result));

        //
        // When the request is completed the content of the SystemBuffer
        // is copied to the User output buffer and the SystemBuffer is
        // is freed.
        //

       break;


    default:

        //
        // The specified I/O control code is unrecognized by this driver.
        //
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    WdfRequestComplete( Request, status);

}

VOID
NonPnpShutdown(WDFDEVICE Device)
{
    UNREFERENCED_PARAMETER(Device);
    return;
}


VOID
NonPnpEvtDriverUnload(IN WDFDRIVER Driver)
{
    UNREFERENCED_PARAMETER(Driver);
    PAGED_CODE();
    return;
}
