#include <iostream>
#include "kernelinterface.hpp"

int main(){
    KernelInterface Driver = KernelInterface("\\\\.\\testDriverLink");

    DWORD address = Driver.GetClientAddress();
    DWORD processId = Driver.GetProcessId();

    printf("Base Address: %d\n", address);
    printf("Process id: %d\n", processId);

    uint32_t LocalPlayerAddress = Driver.ReadVirtualMemory<uint32_t>(processId, address + 0x000183828, sizeof(uint32_t));
    uint32_t LocalPlayerAddress1 = Driver.ReadVirtualMemory<uint32_t>(processId, LocalPlayerAddress + 0x8, sizeof(uint32_t));
    uint32_t LocalPlayerAddress2 = Driver.ReadVirtualMemory<uint32_t>(processId, LocalPlayerAddress1 + 0x730, sizeof(uint32_t));
    uint32_t LocalPlayerAddress3 = Driver.ReadVirtualMemory<uint32_t>(processId, LocalPlayerAddress2 + 0x34, sizeof(uint32_t));
    uint32_t LocalPlayerAddress4 = Driver.ReadVirtualMemory<uint32_t>(processId, LocalPlayerAddress3 + 0x30, sizeof(uint32_t));
    uint32_t LocalPlayerAddress5 = Driver.ReadVirtualMemory<uint32_t>(processId, LocalPlayerAddress4 + 0x30, sizeof(uint32_t));
    uint32_t LocalPlayerAddress6 = Driver.ReadVirtualMemory<uint32_t>(processId, LocalPlayerAddress5 + 0x6BC, sizeof(uint32_t));

    uint32_t LocalPlayerAddress7 = Driver.ReadVirtualMemory<uint32_t>(processId, 0x00089F0C4, sizeof(uint32_t));


    while (true) {
        uint32_t LocalPlayerAddress8 = Driver.ReadVirtualMemory<uint32_t>(processId, 0x00089F0C4, sizeof(uint32_t));
    
        if (!Driver.WriteVirtualMemory(processId, 0x00089F0C4, 100, sizeof(100))) {
            printf("no");
        }
    }
}