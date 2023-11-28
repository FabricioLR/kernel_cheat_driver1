#pragma warning (disable : 4100 4047 4024 4022)
#include "test.h"

//sc create testDriver type= kernel binpath="C:\Users\Fabrício\source\repos\KMDF Driver1\x64\Release\KMDFDriver1.sys"
//bcdedit /set testsigning on
//sc start testDriver
//sc stop testDriver
//sc delete testDriver

NTSTATUS KernelReadVirtualMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size) {
    PSIZE_T Bytes;

    return MmCopyVirtualMemory(Process, SourceAddress, PsGetCurrentProcess(), TargetAddress, Size, KernelMode, &Bytes);
}

NTSTATUS KernelWriteVirtualMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size) {
    PSIZE_T Bytes;

    return MmCopyVirtualMemory(PsGetCurrentProcess(), SourceAddress, Process, TargetAddress, Size, KernelMode, &Bytes);
}

NTSTATUS CreateCall(PDEVICE_OBJECT DeviceObject, PIRP pIrp) {
    pIrp->IoStatus.Status = STATUS_SUCCESS;
    pIrp->IoStatus.Information = 0;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    DbgPrintEx(0, 0, "Create was called\n");

    return STATUS_SUCCESS;
}

NTSTATUS CloseCall(PDEVICE_OBJECT DeviceObject, PIRP pIrp) {
    pIrp->IoStatus.Status = STATUS_SUCCESS;
    pIrp->IoStatus.Information = 0;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    DbgPrintEx(0, 0, "Connection terminated\n");

    return STATUS_SUCCESS;
}

NTSTATUS IoControl(PDEVICE_OBJECT DeviceObject, PIRP pIrp){
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    ULONG ByteIO = 0;

    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);

    ULONG ControlCode = stack->Parameters.DeviceIoControl.IoControlCode;

    DbgPrintEx(0, 0, "%d\n", ControlCode);

    if (ControlCode == IO_GET_CLIENTADDRESS) {
        PULONG OutPut = (PULONG)pIrp->AssociatedIrp.SystemBuffer;
        *OutPut = AssaultCubeAcClientAddress;

        DbgPrintEx(0, 0, "Client Address requested\n");

        Status = STATUS_SUCCESS;
        ByteIO = sizeof(*OutPut);
    } else if (ControlCode == IO_GET_PROCESSID) {
        PULONG OutPut = (PULONG)pIrp->AssociatedIrp.SystemBuffer;
        *OutPut = AssaultCubeAcProcessId;

        DbgPrintEx(0, 0, "Process Id requested\n");

        Status = STATUS_SUCCESS;
        ByteIO = sizeof(*OutPut);
    } else if (ControlCode == IO_READ_REQUEST) {
        PKERNEL_READ_REQUEST ReadInput = (PKERNEL_READ_REQUEST)pIrp->AssociatedIrp.SystemBuffer;
        PEPROCESS Process;

        if (NT_SUCCESS(PsLookupProcessByProcessId(ReadInput->ProcessId, &Process))) {
            KernelReadVirtualMemory(Process, ReadInput->Address, ReadInput->pBuff, ReadInput->Size);

            Status = STATUS_SUCCESS;
            ByteIO = sizeof(KERNEL_READ_REQUEST);
        }
    } else if (ControlCode == IO_WRITE_REQUEST) {
        PKERNEL_WRITE_REQUEST WriteInput = (PKERNEL_WRITE_REQUEST)pIrp->AssociatedIrp.SystemBuffer;
        PEPROCESS Process;

        if (NT_SUCCESS(PsLookupProcessByProcessId(WriteInput->ProcessId, &Process))) {
            KernelWriteVirtualMemory(Process, WriteInput->pBuff, WriteInput->Address, WriteInput->Size);

            Status = STATUS_SUCCESS;
            ByteIO = sizeof(KERNEL_WRITE_REQUEST);
        }
    } else {
        ByteIO = 0;
    }

    pIrp->IoStatus.Status = Status;
    pIrp->IoStatus.Information = ByteIO;

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return Status;
}

PLOAD_IMAGE_NOTIFY_ROUTINE ImageLoadCallback(PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo){
    //DbgPrintEx(0, 0, "Image Loaded: %ls \n", FullImageName->Buffer);

    if (wcsstr(FullImageName->Buffer, L"\\AssaultCube 1.3.0.2\\bin_win32\\ac_client.exe")) {
        DbgPrintEx(0, 0, "ac_client found\n");

        AssaultCubeAcClientAddress = ImageInfo->ImageBase;
        AssaultCubeAcProcessId = ProcessId;

        DbgPrintEx(0, 0, "Process id: %d \n", ProcessId);
        DbgPrintEx(0, 0, "AssaultCubeAcClientAddress: %d \n", AssaultCubeAcClientAddress);
    }

    return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath){
    pDriverObject->DriverUnload = UnloadDriver;

    DbgPrintEx(0, 0, "Load Driver");

    PsSetLoadImageNotifyRoutine(ImageLoadCallback);

    RtlInitUnicodeString(&dev, L"\\Device\\testDriver");
    RtlInitUnicodeString(&dos, L"\\??\\testDriverLink");

    IoCreateDevice(pDriverObject, 0, &dev, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &pDeviceObject);
    IoCreateSymbolicLink(&dos, &dev);

    pDriverObject->MajorFunction[IRP_MJ_CREATE] = CreateCall;
    pDriverObject->MajorFunction[IRP_MJ_CLOSE] = CloseCall;
    pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoControl;

    pDeviceObject->Flags |= DO_DIRECT_IO;
    pDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    return STATUS_SUCCESS;
}

NTSTATUS UnloadDriver(PDRIVER_OBJECT pDriverObject) {
    DbgPrintEx(0, 0, "Unload Driver");

    PsRemoveLoadImageNotifyRoutine(ImageLoadCallback);

    IoDeleteSymbolicLink(&dev);
    IoDeleteDevice(pDriverObject->DeviceObject);

    return STATUS_SUCCESS;
}