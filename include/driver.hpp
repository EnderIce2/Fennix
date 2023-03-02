#ifndef __FENNIX_KERNEL_DRIVER_H__
#define __FENNIX_KERNEL_DRIVER_H__

#include <types.h>

#include <vector.hpp>
#include <memory.hpp>
#include <ints.hpp>
#include <debug.h>
#include <cpu.hpp>
#include <pci.hpp>

namespace Driver
{
    enum DriverCode
    {
        ERROR,
        OK,
        NOT_AVAILABLE,
        INVALID_FEX_HEADER,
        INVALID_DRIVER_DATA,
        NOT_DRIVER,
        NOT_IMPLEMENTED,
        DRIVER_RETURNED_ERROR,
        UNKNOWN_DRIVER_TYPE,
        PCI_DEVICE_NOT_FOUND,
        DRIVER_CONFLICT
    };

    class DriverInterruptHook : public Interrupts::Handler
    {
    private:
        void *Handle;
        void *Data;

#if defined(__amd64__)
        void OnInterruptReceived(CPU::x64::TrapFrame *Frame);
#elif defined(__i386__)
        void OnInterruptReceived(void *Frame);
#elif defined(__aarch64__)
        void OnInterruptReceived(void *Frame);
#endif

    public:
        DriverInterruptHook(int Interrupt, void *Address, void *ParamData);
        virtual ~DriverInterruptHook() = default;
    };

    struct DriverFile
    {
        bool Enabled;
        unsigned long DriverUID;
        void *Address;
        Memory::MemMgr *MemTrk;
        DriverInterruptHook *InterruptHook[16];
    };

    class Driver
    {
    private:
        Vector<DriverFile *> Drivers;
        unsigned long DriverUIDs = 0;
        DriverCode CallDriverEntryPoint(void *fex, void *KAPIAddress);

        DriverCode BindPCIGeneric(Memory::MemMgr *mem, void *fex, PCI::PCIDeviceHeader *PCIDevice);
        DriverCode BindPCIDisplay(Memory::MemMgr *mem, void *fex, PCI::PCIDeviceHeader *PCIDevice);
        DriverCode BindPCINetwork(Memory::MemMgr *mem, void *fex, PCI::PCIDeviceHeader *PCIDevice);
        DriverCode BindPCIStorage(Memory::MemMgr *mem, void *fex, PCI::PCIDeviceHeader *PCIDevice);
        DriverCode BindPCIFileSystem(Memory::MemMgr *mem, void *fex, PCI::PCIDeviceHeader *PCIDevice);
        DriverCode BindPCIInput(Memory::MemMgr *mem, void *fex, PCI::PCIDeviceHeader *PCIDevice);
        DriverCode BindPCIAudio(Memory::MemMgr *mem, void *fex, PCI::PCIDeviceHeader *PCIDevice);
        DriverCode DriverLoadBindPCI(void *DrvExtHdr, uintptr_t DriverAddress, size_t Size, bool IsElf = false);

        DriverCode BindInterruptGeneric(Memory::MemMgr *mem, void *fex);
        DriverCode BindInterruptDisplay(Memory::MemMgr *mem, void *fex);
        DriverCode BindInterruptNetwork(Memory::MemMgr *mem, void *fex);
        DriverCode BindInterruptStorage(Memory::MemMgr *mem, void *fex);
        DriverCode BindInterruptFileSystem(Memory::MemMgr *mem, void *fex);
        DriverCode BindInterruptInput(Memory::MemMgr *mem, void *fex);
        DriverCode BindInterruptAudio(Memory::MemMgr *mem, void *fex);
        DriverCode DriverLoadBindInterrupt(void *DrvExtHdr, uintptr_t DriverAddress, size_t Size, bool IsElf = false);

        DriverCode BindInputGeneric(Memory::MemMgr *mem, void *fex);
        DriverCode BindInputDisplay(Memory::MemMgr *mem, void *fex);
        DriverCode BindInputNetwork(Memory::MemMgr *mem, void *fex);
        DriverCode BindInputStorage(Memory::MemMgr *mem, void *fex);
        DriverCode BindInputFileSystem(Memory::MemMgr *mem, void *fex);
        DriverCode BindInputInput(Memory::MemMgr *mem, void *fex);
        DriverCode BindInputAudio(Memory::MemMgr *mem, void *fex);
        DriverCode DriverLoadBindInput(void *DrvExtHdr, uintptr_t DriverAddress, size_t Size, bool IsElf = false);

        DriverCode BindProcessGeneric(Memory::MemMgr *mem, void *fex);
        DriverCode BindProcessDisplay(Memory::MemMgr *mem, void *fex);
        DriverCode BindProcessNetwork(Memory::MemMgr *mem, void *fex);
        DriverCode BindProcessStorage(Memory::MemMgr *mem, void *fex);
        DriverCode BindProcessFileSystem(Memory::MemMgr *mem, void *fex);
        DriverCode BindProcessInput(Memory::MemMgr *mem, void *fex);
        DriverCode BindProcessAudio(Memory::MemMgr *mem, void *fex);
        DriverCode DriverLoadBindProcess(void *DrvExtHdr, uintptr_t DriverAddress, size_t Size, bool IsElf = false);

    public:
        Vector<DriverFile *> GetDrivers() { return Drivers; }
        void UnloadAllDrivers();
        bool UnloadDriver(unsigned long DUID);
        int IOCB(unsigned long DUID, /* KernelCallback */ void *KCB);
        DriverCode LoadDriver(uintptr_t DriverAddress, size_t Size);
        DriverCode StartDrivers();
        Driver();
        ~Driver();
    };
}

#endif // !__FENNIX_KERNEL_DRIVER_H__
