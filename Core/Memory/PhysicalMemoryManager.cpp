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

#include "../../kernel.h"

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

    void Physical::Init()
    {
        SmartLock(this->MemoryLock);

        uint64_t MemorySize = bInfo.Memory.Size;
        debug("Memory size: %lld bytes (%ld pages)", MemorySize, TO_PAGES(MemorySize));
        TotalMemory = MemorySize;
        FreeMemory = MemorySize;

        size_t BitmapSize = (MemorySize / PAGE_SIZE) / 8 + 1;
        uintptr_t BitmapAddress = 0x0;
        size_t BitmapAddressSize = 0;

        uintptr_t KernelStart = (uintptr_t)bInfo.Kernel.PhysicalBase;
        uintptr_t KernelEnd = (uintptr_t)bInfo.Kernel.PhysicalBase + bInfo.Kernel.Size;

        for (uint64_t i = 0; i < bInfo.Memory.Entries; i++)
        {
            if (bInfo.Memory.Entry[i].Type == Usable)
            {
                uintptr_t RegionAddress = (uintptr_t)bInfo.Memory.Entry[i].BaseAddress;
                uintptr_t RegionSize = bInfo.Memory.Entry[i].Length;

                /* We don't want to use 0 as a memory address. */
                if (RegionAddress == 0x0)
                    continue;

                if ((BitmapSize + 0x100) > RegionSize)
                {
                    debug("Region %p-%p (%dMB) is too small for bitmap.",
                          (void *)RegionAddress,
                          (void *)(RegionAddress + RegionSize),
                          TO_MB(RegionSize));
                    continue;
                }

                BitmapAddress = RegionAddress;
                BitmapAddressSize = RegionSize;

                if (RegionAddress >= KernelStart && KernelEnd <= (RegionAddress + RegionSize))
                {
                    BitmapAddress = KernelEnd;
                    BitmapAddressSize = RegionSize - (KernelEnd - RegionAddress);
                }

                if ((BitmapSize + 0x100) > BitmapAddressSize)
                {
                    debug("Region %p-%p (%dMB) is too small for bitmap.",
                          (void *)RegionAddress,
                          (void *)(RegionAddress + BitmapAddressSize),
                          TO_MB(BitmapAddressSize));
                    continue;
                }

                for (size_t i = 0; i < MAX_MODULES; i++)
                {
                    uintptr_t ModuleStart = (uintptr_t)bInfo.Modules[i].Address;
                    uintptr_t ModuleEnd = (uintptr_t)bInfo.Modules[i].Address + bInfo.Modules[i].Size;

                    if (ModuleStart == 0x0)
                        break;

                    if (RegionAddress >= ModuleStart && ModuleEnd <= (RegionAddress + RegionSize))
                    {
                        BitmapAddress = ModuleEnd;
                        BitmapAddressSize = RegionSize - (ModuleEnd - RegionAddress);
                    }
                }

                if ((BitmapSize + 0x100) > BitmapAddressSize)
                {
                    debug("Region %p-%p (%dMB) is too small for bitmap.",
                          (void *)BitmapAddress,
                          (void *)(BitmapAddress + BitmapAddressSize),
                          TO_MB(BitmapAddressSize));
                    continue;
                }

                debug("Found free memory for bitmap: %p (%dMB)",
                      (void *)BitmapAddress,
                      TO_MB(BitmapAddressSize));
                break;
            }
        }

        if (BitmapAddress == 0x0)
        {
            error("No free memory found!");
            CPU::Stop();
        }

        /* TODO: Read swap config and make the configure the bitmap size correctly */
        debug("Initializing Bitmap at %p-%p (%d Bytes)",
              BitmapAddress,
              (void *)(BitmapAddress + BitmapSize),
              BitmapSize);

        PageBitmap.Size = BitmapSize;
        PageBitmap.Buffer = (uint8_t *)BitmapAddress;
        for (size_t i = 0; i < BitmapSize; i++)
            *(uint8_t *)(PageBitmap.Buffer + i) = 0;

        ReserveEssentials();
    }

    Physical::Physical() {}
    Physical::~Physical() {}
}
