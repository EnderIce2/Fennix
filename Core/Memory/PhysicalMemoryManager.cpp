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

#include <memory.hpp>

#include <debug.h>
#ifdef DEBUG
#include <uart.hpp>
#endif

#include "../../Architecture/amd64/acpi.hpp"

#include "../../kernel.h"

extern "C" char BootPageTable[]; // 0x10000 in length

namespace Memory
{
    uint64_t Physical::GetTotalMemory()
    {
        SmartLock(this->MemoryLock);
        return this->TotalMemory;
    }

    uint64_t Physical::GetFreeMemory()
    {
        SmartLock(this->MemoryLock);
        return this->FreeMemory;
    }

    uint64_t Physical::GetReservedMemory()
    {
        SmartLock(this->MemoryLock);
        return this->ReservedMemory;
    }

    uint64_t Physical::GetUsedMemory()
    {
        SmartLock(this->MemoryLock);
        return this->UsedMemory;
    }

    bool Physical::SwapPage(void *Address)
    {
        fixme("%p", Address);
        return false;
    }

    bool Physical::SwapPages(void *Address, size_t PageCount)
    {
        for (size_t i = 0; i < PageCount; i++)
        {
            if (!this->SwapPage((void *)((uintptr_t)Address + (i * PAGE_SIZE))))
                return false;
        }
        return false;
    }

    bool Physical::UnswapPage(void *Address)
    {
        fixme("%p", Address);
        return false;
    }

    bool Physical::UnswapPages(void *Address, size_t PageCount)
    {
        for (size_t i = 0; i < PageCount; i++)
        {
            if (!this->UnswapPage((void *)((uintptr_t)Address + (i * PAGE_SIZE))))
                return false;
        }
        return false;
    }

    void *Physical::RequestPage()
    {
        SmartLock(this->MemoryLock);

        for (; PageBitmapIndex < PageBitmap.Size * 8; PageBitmapIndex++)
        {
            if (PageBitmap[PageBitmapIndex] == true)
                continue;

            this->LockPage((void *)(PageBitmapIndex * PAGE_SIZE));
#ifdef DEBUG
            if (EnableExternalMemoryTracer)
            {
                char LockTmpStr[64];
                strcpy_unsafe(LockTmpStr, __FUNCTION__);
                strcat_unsafe(LockTmpStr, "_memTrk");
                mExtTrkLock.TimeoutLock(LockTmpStr, 10000);
                sprintf(mExtTrkLog, "RequestPage( )=%p~%p\n\r",
                        (void *)(PageBitmapIndex * PAGE_SIZE), __builtin_return_address(0));
                UniversalAsynchronousReceiverTransmitter::UART mTrkUART = UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM3);
                for (short i = 0; i < MEM_TRK_MAX_SIZE; i++)
                {
                    if (mExtTrkLog[i] == '\r')
                        break;
                    mTrkUART.Write(mExtTrkLog[i]);
                }
                mExtTrkLock.Unlock();
            }
#endif
            return (void *)(PageBitmapIndex * PAGE_SIZE);
        }

        if (this->SwapPage((void *)(PageBitmapIndex * PAGE_SIZE)))
        {
            this->LockPage((void *)(PageBitmapIndex * PAGE_SIZE));
            return (void *)(PageBitmapIndex * PAGE_SIZE);
        }

        error("Out of memory! (Free: %ldMB; Used: %ldMB; Reserved: %ldMB)", TO_MB(FreeMemory), TO_MB(UsedMemory), TO_MB(ReservedMemory));
        CPU::Stop();
        __builtin_unreachable();
    }

    void *Physical::RequestPages(size_t Count)
    {
        SmartLock(this->MemoryLock);

        for (; PageBitmapIndex < PageBitmap.Size * 8; PageBitmapIndex++)
        {
            if (PageBitmap[PageBitmapIndex] == true)
                continue;

            for (uint64_t Index = PageBitmapIndex; Index < PageBitmap.Size * 8; Index++)
            {
                if (PageBitmap[Index] == true)
                    continue;

                for (size_t i = 0; i < Count; i++)
                {
                    if (PageBitmap[Index + i] == true)
                        goto NextPage;
                }

                this->LockPages((void *)(Index * PAGE_SIZE), Count);
#ifdef DEBUG
                if (EnableExternalMemoryTracer)
                {
                    char LockTmpStr[64];
                    strcpy_unsafe(LockTmpStr, __FUNCTION__);
                    strcat_unsafe(LockTmpStr, "_memTrk");
                    mExtTrkLock.TimeoutLock(LockTmpStr, 10000);
                    sprintf(mExtTrkLog, "RequestPages( %ld )=%p~%p\n\r",
                            Count,
                            (void *)(Index * PAGE_SIZE), __builtin_return_address(0));
                    UniversalAsynchronousReceiverTransmitter::UART mTrkUART = UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM3);
                    for (short i = 0; i < MEM_TRK_MAX_SIZE; i++)
                    {
                        if (mExtTrkLog[i] == '\r')
                            break;
                        mTrkUART.Write(mExtTrkLog[i]);
                    }
                    mExtTrkLock.Unlock();
                }
#endif
                return (void *)(Index * PAGE_SIZE);

            NextPage:
                Index += Count;
                continue;
            }
        }

        if (this->SwapPages((void *)(PageBitmapIndex * PAGE_SIZE), Count))
        {
            this->LockPages((void *)(PageBitmapIndex * PAGE_SIZE), Count);
            return (void *)(PageBitmapIndex * PAGE_SIZE);
        }

        error("Out of memory! (Free: %ldMB; Used: %ldMB; Reserved: %ldMB)", TO_MB(FreeMemory), TO_MB(UsedMemory), TO_MB(ReservedMemory));
        CPU::Halt(true);
        __builtin_unreachable();
    }

    void Physical::FreePage(void *Address)
    {
        SmartLock(this->MemoryLock);

        if (unlikely(Address == nullptr))
        {
            warn("Null pointer passed to FreePage.");
            return;
        }

        size_t Index = (size_t)Address / PAGE_SIZE;

        if (unlikely(PageBitmap[Index] == false))
        {
            warn("Tried to free an already free page. (%p)", Address);
            return;
        }

        if (PageBitmap.Set(Index, false))
        {
            FreeMemory += PAGE_SIZE;
            UsedMemory -= PAGE_SIZE;
            if (PageBitmapIndex > Index)
                PageBitmapIndex = Index;
        }

#ifdef DEBUG
        if (EnableExternalMemoryTracer)
        {
            char LockTmpStr[64];
            strcpy_unsafe(LockTmpStr, __FUNCTION__);
            strcat_unsafe(LockTmpStr, "_memTrk");
            mExtTrkLock.TimeoutLock(LockTmpStr, 10000);
            sprintf(mExtTrkLog, "FreePage( %p )~%p\n\r",
                    Address,
                    __builtin_return_address(0));
            UniversalAsynchronousReceiverTransmitter::UART mTrkUART = UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM3);
            for (short i = 0; i < MEM_TRK_MAX_SIZE; i++)
            {
                if (mExtTrkLog[i] == '\r')
                    break;
                mTrkUART.Write(mExtTrkLog[i]);
            }
            mExtTrkLock.Unlock();
        }
#endif
    }

    void Physical::FreePages(void *Address, size_t Count)
    {
        if (unlikely(Address == nullptr || Count == 0))
        {
            warn("%s%s%s passed to FreePages.", Address == nullptr ? "Null pointer " : "", Address == nullptr && Count == 0 ? "and " : "", Count == 0 ? "Zero count" : "");
            return;
        }
#ifdef DEBUG
        if (EnableExternalMemoryTracer)
        {
            char LockTmpStr[64];
            strcpy_unsafe(LockTmpStr, __FUNCTION__);
            strcat_unsafe(LockTmpStr, "_memTrk");
            mExtTrkLock.TimeoutLock(LockTmpStr, 10000);
            sprintf(mExtTrkLog, "!FreePages( %p %ld )~%p\n\r",
                    Address, Count,
                    __builtin_return_address(0));
            UniversalAsynchronousReceiverTransmitter::UART mTrkUART = UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM3);
            for (short i = 0; i < MEM_TRK_MAX_SIZE; i++)
            {
                if (mExtTrkLog[i] == '\r')
                    break;
                mTrkUART.Write(mExtTrkLog[i]);
            }
            mExtTrkLock.Unlock();
        }
#endif
        for (size_t t = 0; t < Count; t++)
            this->FreePage((void *)((uintptr_t)Address + (t * PAGE_SIZE)));
    }

    void Physical::LockPage(void *Address)
    {
        if (unlikely(Address == nullptr))
            warn("Trying to lock null address.");

        uintptr_t Index = (uintptr_t)Address / PAGE_SIZE;

        if (unlikely(PageBitmap[Index] == true))
            return;

        if (PageBitmap.Set(Index, true))
        {
            FreeMemory -= PAGE_SIZE;
            UsedMemory += PAGE_SIZE;
        }
    }

    void Physical::LockPages(void *Address, size_t PageCount)
    {
        if (unlikely(Address == nullptr || PageCount == 0))
            warn("Trying to lock %s%s.", Address ? "null address" : "", PageCount ? "0 pages" : "");

        for (size_t i = 0; i < PageCount; i++)
            this->LockPage((void *)((uintptr_t)Address + (i * PAGE_SIZE)));
    }

    void Physical::ReservePage(void *Address)
    {
        if (unlikely(Address == nullptr))
            warn("Trying to reserve null address.");

        uintptr_t Index = (Address == NULL) ? 0 : (uintptr_t)Address / PAGE_SIZE;

        if (unlikely(PageBitmap[Index] == true))
            return;

        if (PageBitmap.Set(Index, true))
        {
            FreeMemory -= PAGE_SIZE;
            ReservedMemory += PAGE_SIZE;
        }
    }

    void Physical::ReservePages(void *Address, size_t PageCount)
    {
        if (unlikely(Address == nullptr || PageCount == 0))
            warn("Trying to reserve %s%s.", Address ? "null address" : "", PageCount ? "0 pages" : "");

        for (size_t t = 0; t < PageCount; t++)
        {
            uintptr_t Index = ((uintptr_t)Address + (t * PAGE_SIZE)) / PAGE_SIZE;

            if (unlikely(PageBitmap[Index] == true))
                return;

            if (PageBitmap.Set(Index, true))
            {
                FreeMemory -= PAGE_SIZE;
                ReservedMemory += PAGE_SIZE;
            }
        }
    }

    void Physical::UnreservePage(void *Address)
    {
        if (unlikely(Address == nullptr))
            warn("Trying to unreserve null address.");

        uintptr_t Index = (Address == NULL) ? 0 : (uintptr_t)Address / PAGE_SIZE;

        if (unlikely(PageBitmap[Index] == false))
            return;

        if (PageBitmap.Set(Index, false))
        {
            FreeMemory += PAGE_SIZE;
            ReservedMemory -= PAGE_SIZE;
            if (PageBitmapIndex > Index)
                PageBitmapIndex = Index;
        }
    }

    void Physical::UnreservePages(void *Address, size_t PageCount)
    {
        if (unlikely(Address == nullptr || PageCount == 0))
            warn("Trying to unreserve %s%s.", Address ? "null address" : "", PageCount ? "0 pages" : "");

        for (size_t t = 0; t < PageCount; t++)
        {
            uintptr_t Index = ((uintptr_t)Address + (t * PAGE_SIZE)) / PAGE_SIZE;

            if (unlikely(PageBitmap[Index] == false))
                return;

            if (PageBitmap.Set(Index, false))
            {
                FreeMemory += PAGE_SIZE;
                ReservedMemory -= PAGE_SIZE;
                if (PageBitmapIndex > Index)
                    PageBitmapIndex = Index;
            }
        }
    }

    void Physical::Init(BootInfo *Info)
    {
        SmartLock(this->MemoryLock);

        uint64_t MemorySize = Info->Memory.Size;
        debug("Memory size: %lld bytes (%ld pages)", MemorySize, TO_PAGES(MemorySize));
        TotalMemory = MemorySize;
        FreeMemory = MemorySize;

        size_t BitmapSize = (MemorySize / PAGE_SIZE) / 8 + 1;
        void *LargestFreeMemorySegment = nullptr;
        uint64_t LargestFreeMemorySegmentSize = 0;

        for (uint64_t i = 0; i < Info->Memory.Entries; i++)
        {
            if (Info->Memory.Entry[i].Type == Usable)
            {
                if (Info->Memory.Entry[i].Length > LargestFreeMemorySegmentSize)
                {
                    /* We don't want to use 0 as a memory address. */
                    if (Info->Memory.Entry[i].BaseAddress == 0x0)
                    {
                        debug("Ignoring memory segment at 0x0");
                        continue;
                    }

                    if (Info->Memory.Entry[i].Length > BitmapSize + 0x1000)
                    {
                        LargestFreeMemorySegment = (void *)Info->Memory.Entry[i].BaseAddress;
                        LargestFreeMemorySegmentSize = Info->Memory.Entry[i].Length;

#define ROUND_UP(N, S) ((((N) + (S)-1) / (S)) * (S))
                        if (LargestFreeMemorySegment >= Info->Kernel.PhysicalBase &&
                            LargestFreeMemorySegment <= (void *)((uintptr_t)Info->Kernel.PhysicalBase + Info->Kernel.Size))
                        {
                            debug("Kernel range: %#lx-%#lx", Info->Kernel.PhysicalBase, (void *)((uintptr_t)Info->Kernel.PhysicalBase + Info->Kernel.Size));

                            void *NewLargestFreeMemorySegment = (void *)((uintptr_t)Info->Kernel.PhysicalBase + Info->Kernel.Size);
                            void *RoundNewLargestFreeMemorySegment = (void *)ROUND_UP((uintptr_t)NewLargestFreeMemorySegment, PAGE_SIZE);
                            RoundNewLargestFreeMemorySegment = (void *)((uintptr_t)RoundNewLargestFreeMemorySegment + PAGE_SIZE); /* Leave a page between the kernel and the bitmap */

                            debug("Rounding %p to %p", NewLargestFreeMemorySegment, RoundNewLargestFreeMemorySegment);
                            info("Memory bitmap's memory segment is in the kernel, moving it to %p", RoundNewLargestFreeMemorySegment);
                            LargestFreeMemorySegmentSize = (uintptr_t)LargestFreeMemorySegmentSize - ((uintptr_t)RoundNewLargestFreeMemorySegment - (uintptr_t)LargestFreeMemorySegment);
                            LargestFreeMemorySegment = RoundNewLargestFreeMemorySegment;
                        }
#undef ROUND_UP

                        if (LargestFreeMemorySegmentSize < BitmapSize + 0x1000)
                        {
                            trace("Largest free memory segment is too small (%lld bytes), skipping...",
                                  LargestFreeMemorySegmentSize);
                            continue;
                        }

                        debug("Found a memory segment of %lld bytes (%lldMB) at %llp (out segment is %lld bytes (%lldKB)))",
                              LargestFreeMemorySegmentSize,
                              TO_MB(LargestFreeMemorySegmentSize),
                              LargestFreeMemorySegment,
                              BitmapSize,
                              TO_KB(BitmapSize));
                        break;
                    }

                    //     LargestFreeMemorySegment = (void *)Info->Memory.Entry[i].BaseAddress;
                    //     LargestFreeMemorySegmentSize = Info->Memory.Entry[i].Length;

                    //     debug("Largest free memory segment: %llp (%lldMB)",
                    //           (void *)Info->Memory.Entry[i].BaseAddress,
                    //           TO_MB(Info->Memory.Entry[i].Length));
                }
            }
        }

        if (LargestFreeMemorySegment == nullptr)
        {
            error("No free memory found!");
            CPU::Stop();
        }

        /* TODO: Read swap config and make the configure the bitmap size correctly */
        debug("Initializing Bitmap at %llp-%llp (%lld Bytes)",
              LargestFreeMemorySegment,
              (void *)((uintptr_t)LargestFreeMemorySegment + BitmapSize),
              BitmapSize);

        PageBitmap.Size = BitmapSize;
        PageBitmap.Buffer = (uint8_t *)LargestFreeMemorySegment;
        for (size_t i = 0; i < BitmapSize; i++)
            *(uint8_t *)(PageBitmap.Buffer + i) = 0;

        debug("Reserving pages...");
        this->ReservePages(0, TO_PAGES(Info->Memory.Size));
        debug("Unreserving usable pages...");

        for (uint64_t i = 0; i < Info->Memory.Entries; i++)
        {
            if (Info->Memory.Entry[i].Type == Usable && Info->Memory.Entry[i].BaseAddress != 0x0)
                this->UnreservePages(Info->Memory.Entry[i].BaseAddress, TO_PAGES(Info->Memory.Entry[i].Length));
        }

        debug("Reserving pages for SMP...");
        this->ReservePage((void *)0x0);        /* Trampoline stack, gdt, idt, etc... */
        this->ReservePages((void *)0x2000, 4); /* TRAMPOLINE_START */

        debug("Reserving bitmap region %#lx-%#lx...", PageBitmap.Buffer, (void *)((uintptr_t)PageBitmap.Buffer + PageBitmap.Size));
        this->ReservePages(PageBitmap.Buffer, TO_PAGES(PageBitmap.Size));
        // debug("Reserving page table...");
        // this->ReservePages(BootPageTable, TO_PAGES(0x10000)); << in the bootstrap region
        debug("Reserving kernel bootstrap region %#lx-%#lx...", &_bootstrap_start, &_bootstrap_end);
        this->ReservePages(&_bootstrap_start, TO_PAGES((uintptr_t)&_bootstrap_end - (uintptr_t)&_bootstrap_start));
        void *KernelPhysicalStart = (void *)(((uintptr_t)&_kernel_start - KERNEL_VMA_OFFSET));
        void *KernelPhysicalEnd = (void *)(((uintptr_t)&_kernel_end - KERNEL_VMA_OFFSET));
        debug("Reserving kernel region %#lx-%#lx...", KernelPhysicalStart, KernelPhysicalEnd);
        this->ReservePages((void *)KernelPhysicalStart, TO_PAGES((uintptr_t)&_kernel_end - (uintptr_t)&_kernel_start));

        ACPI::ACPI::ACPIHeader *hdr = nullptr;
        bool XSDT = false;

        if (Info->RSDP->Revision >= 2 && Info->RSDP->XSDTAddress)
        {
            hdr = (ACPI::ACPI::ACPIHeader *)(Info->RSDP->XSDTAddress);
            XSDT = true;
        }
        else
        {
            hdr = (ACPI::ACPI::ACPIHeader *)(uintptr_t)Info->RSDP->RSDTAddress;
        }

        debug("Reserving RSDT...");
        this->ReservePages((void*)Info->RSDP, TO_PAGES(sizeof(BootInfo::RSDPInfo)));

        debug("Reserving ACPI tables...");

        uint64_t TableSize = ((hdr->Length - sizeof(ACPI::ACPI::ACPIHeader)) / (XSDT ? 8 : 4));
        debug("Table size: %lld", TableSize);

        for (uint64_t t = 0; t < TableSize; t++)
        {
            // TODO: Should I be concerned about unaligned memory access?
            ACPI::ACPI::ACPIHeader *SDTHdr = nullptr;
            if (XSDT)
                SDTHdr = (ACPI::ACPI::ACPIHeader *)(*(uint64_t *)((uint64_t)hdr + sizeof(ACPI::ACPI::ACPIHeader) + (t * 8)));
            else
                SDTHdr = (ACPI::ACPI::ACPIHeader *)(*(uint32_t *)((uint64_t)hdr + sizeof(ACPI::ACPI::ACPIHeader) + (t * 4)));

            this->ReservePages(SDTHdr, TO_PAGES(SDTHdr->Length));
        }

        debug("Reserving kernel modules...");

        for (uint64_t i = 0; i < MAX_MODULES; i++)
        {
            if (Info->Modules[i].Address == 0x0)
                continue;

            debug("Reserving module %s (%#lx-%#lx)...", Info->Modules[i].CommandLine,
                  Info->Modules[i].Address, (void *)((uintptr_t)Info->Modules[i].Address + Info->Modules[i].Size));
        
            this->ReservePages((void *)Info->Modules[i].Address, TO_PAGES(Info->Modules[i].Size));
        }

    }

    Physical::Physical() {}
    Physical::~Physical() {}
}
