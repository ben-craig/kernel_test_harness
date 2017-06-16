#define INITGUID

#include <ntddk.h>
#include <wdf.h>
#include "public.h"

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD ToasterEvtDeviceAdd;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL ToasterEvtIoDeviceControl;

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, ToasterEvtDeviceAdd)
#pragma alloc_text (PAGE, ToasterEvtIoDeviceControl)
#endif

int __cdecl main(void);

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath)
{
    NTSTATUS            status = STATUS_SUCCESS;
    WDF_DRIVER_CONFIG   config;

    WDF_DRIVER_CONFIG_INIT(
        &config,
        ToasterEvtDeviceAdd
        );

    status = WdfDriverCreate(
        DriverObject,
        RegistryPath,
        WDF_NO_OBJECT_ATTRIBUTES, // Driver Attributes
        &config,
        WDF_NO_HANDLE);

    if (!NT_SUCCESS(status)) {
        KdPrint( ("WdfDriverCreate failed with status 0x%x\n", status));
    }

    return status;
}


NTSTATUS
ToasterEvtDeviceAdd(
    IN WDFDRIVER       Driver,
    IN PWDFDEVICE_INIT DeviceInit)
{
    NTSTATUS               status = STATUS_SUCCESS;
    WDF_IO_QUEUE_CONFIG    queueConfig = { 0 };
    WDFDEVICE              hDevice = NULL;
    WDFQUEUE               queue = NULL;

    UNREFERENCED_PARAMETER(Driver);
    PAGED_CODE();

    status = WdfDeviceCreate(&DeviceInit, WDF_NO_OBJECT_ATTRIBUTES, &hDevice);
    if (!NT_SUCCESS(status)) {
        KdPrint( ("WdfDeviceCreate failed with status code 0x%x\n", status));
        return status;
    }

    // Tell the Framework that this device will need an interface
    status = WdfDeviceCreateDeviceInterface(
        hDevice,
        (LPGUID)&GUID_DEVINTERFACE_TOASTER,
        NULL /* ReferenceString */);

    if (!NT_SUCCESS (status)) {
        KdPrint( ("WdfDeviceCreateDeviceInterface failed 0x%x\n", status));
        return status;
    }

    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queueConfig,  WdfIoQueueDispatchParallel);

    queueConfig.EvtIoDeviceControl = ToasterEvtIoDeviceControl;

    __analysis_assume(queueConfig.EvtIoStop != 0);
    status = WdfIoQueueCreate(
        hDevice,
        &queueConfig,
        WDF_NO_OBJECT_ATTRIBUTES,
        &queue);
    __analysis_assume(queueConfig.EvtIoStop == 0);

    if (!NT_SUCCESS (status)) {

        KdPrint( ("WdfIoQueueCreate failed 0x%x\n", status));
        return status;
    }

    return status;
}

VOID
ToasterEvtIoDeviceControl(
    IN WDFQUEUE     Queue,
    IN WDFREQUEST   Request,
    IN size_t       OutputBufferLength,
    IN size_t       InputBufferLength,
    IN ULONG        IoControlCode)
{
    NTSTATUS  status= STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(Queue);
    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(IoControlCode);

    PAGED_CODE();

    status = main();
    WdfRequestCompleteWithInformation(Request, status, (ULONG_PTR) 0);
}
