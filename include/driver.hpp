/*
   This file is part of Fennix Kernel.

   Fennix Kernel is free software: you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of
   the License, or (at your option) any later version.

   Fennix Kernel is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Fennix Kernel. If not, see <https://www.gnu.org/licenses/>.
*/

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

    class DriverInterruptHook;
    struct DriverFile
    {
        bool Enabled = false;
        unsigned long DriverUID = 0;
        void *Address = nullptr;
        void *InterruptCallback = nullptr;
        Memory::MemMgr *MemTrk = nullptr;
        DriverInterruptHook *InterruptHook[16] = {nullptr};
    };

    class DriverInterruptHook : public Interrupts::Handler
    {
    private:
        DriverFile Handle;

#if defined(a64)
        void OnInterruptReceived(CPU::x64::TrapFrame *Frame);
#elif defined(a32)
        void OnInterruptReceived(CPU::x32::TrapFrame *Frame);
#elif defined(aa64)
        void OnInterruptReceived(void *Frame);
#endif

    public:
        DriverInterruptHook(int Interrupt, DriverFile Handle);
        virtual ~DriverInterruptHook() = default;
    };

    class Driver
    {
    private:
        std::vector<DriverFile> Drivers;
        unsigned long DriverUIDs = 0;
        DriverCode CallDriverEntryPoint(void *fex, void *KAPIAddress);

        void MapPCIAddresses(PCI::PCIDeviceHeader *PCIDevice);
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
        std::vector<DriverFile> GetDrivers() { return Drivers; }
        void Panic();
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
