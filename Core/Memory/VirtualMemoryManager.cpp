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

#include <convert.h>
#include <debug.h>

namespace Memory
{
    bool Virtual::Check(void *VirtualAddress, PTFlag Flag, MapType Type)
    {
        // 0x1000 aligned
        uintptr_t Address = (uintptr_t)VirtualAddress;
        Address &= 0xFFFFFFFFFFFFF000;

        PageMapIndexer Index = PageMapIndexer(Address);
        PageMapLevel4 PML4 = this->Table->Entries[Index.PMLIndex];

        PageDirectoryPointerTableEntryPtr *PDPTE = nullptr;
        PageDirectoryEntryPtr *PDE = nullptr;
        PageTableEntryPtr *PTE = nullptr;

        if ((PML4.raw & Flag) > 0)
        {
            PDPTE = (PageDirectoryPointerTableEntryPtr *)((uintptr_t)PML4.GetAddress() << 12);
            if (PDPTE)
            {
                if ((PDPTE->Entries[Index.PDPTEIndex].Present))
                {
                    if (Type == MapType::OneGB && PDPTE->Entries[Index.PDPTEIndex].PageSize)
                        return true;

                    PDE = (PageDirectoryEntryPtr *)((uintptr_t)PDPTE->Entries[Index.PDPTEIndex].GetAddress() << 12);
                    if (PDE)
                    {
                        if (Type == MapType::TwoMB && PDE->Entries[Index.PDEIndex].PageSize)
                            return true;

                        if ((PDE->Entries[Index.PDEIndex].Present))
                        {
                            PTE = (PageTableEntryPtr *)((uintptr_t)PDE->Entries[Index.PDEIndex].GetAddress() << 12);
                            if (PTE)
                            {
                                if ((PTE->Entries[Index.PTEIndex].Present))
                                    return true;
                            }
                        }
                    }
                }
            }
        }
        return false;
    }

    void *Virtual::GetPhysical(void *VirtualAddress)
    {
        // 0x1000 aligned
        uintptr_t Address = (uintptr_t)VirtualAddress;
        Address &= 0xFFFFFFFFFFFFF000;

        PageMapIndexer Index = PageMapIndexer(Address);
        PageMapLevel4 PML4 = this->Table->Entries[Index.PMLIndex];

        PageDirectoryPointerTableEntryPtr *PDPTE = nullptr;
        PageDirectoryEntryPtr *PDE = nullptr;
        PageTableEntryPtr *PTE = nullptr;

        if (PML4.Present)
        {
            PDPTE = (PageDirectoryPointerTableEntryPtr *)((uintptr_t)PML4.GetAddress() << 12);
            if (PDPTE)
            {
                if (PDPTE->Entries[Index.PDPTEIndex].Present)
                {
                    if (PDPTE->Entries[Index.PDPTEIndex].PageSize)
                        return (void *)((uintptr_t)PDPTE->Entries[Index.PDPTEIndex].GetAddress() << 12);

                    PDE = (PageDirectoryEntryPtr *)((uintptr_t)PDPTE->Entries[Index.PDPTEIndex].GetAddress() << 12);
                    if (PDE)
                    {
                        if (PDE->Entries[Index.PDEIndex].Present)
                        {
                            if (PDE->Entries[Index.PDEIndex].PageSize)
                                return (void *)((uintptr_t)PDE->Entries[Index.PDEIndex].GetAddress() << 12);

                            PTE = (PageTableEntryPtr *)((uintptr_t)PDE->Entries[Index.PDEIndex].GetAddress() << 12);
                            if (PTE)
                            {
                                if (PTE->Entries[Index.PTEIndex].Present)
                                    return (void *)((uintptr_t)PTE->Entries[Index.PTEIndex].GetAddress() << 12);
                            }
                        }
                    }
                }
            }
        }
        return nullptr;
    }

    void Virtual::Map(void *VirtualAddress, void *PhysicalAddress, uint64_t Flags, MapType Type)
    {
        SmartLock(this->MemoryLock);
        if (unlikely(!this->Table))
        {
            error("No page table");
            return;
        }

        Flags |= PTFlag::P;

        PageMapIndexer Index = PageMapIndexer((uintptr_t)VirtualAddress);
        // Clear any flags that are not 1 << 0 (Present) - 1 << 5 (Accessed) because rest are for page table entries only
        uint64_t DirectoryFlags = Flags & 0x3F;

        PageMapLevel4 PML4 = this->Table->Entries[Index.PMLIndex];
        PageDirectoryPointerTableEntryPtr *PDPTEPtr = nullptr;
        if (!PML4.Present)
        {
            PDPTEPtr = (PageDirectoryPointerTableEntryPtr *)KernelAllocator.RequestPage();
            memset(PDPTEPtr, 0, PAGE_SIZE);
            PML4.Present = true;
            PML4.SetAddress((uintptr_t)PDPTEPtr >> 12);
        }
        else
            PDPTEPtr = (PageDirectoryPointerTableEntryPtr *)((uintptr_t)PML4.GetAddress() << 12);

        if (Type == MapType::OneGB)
        {
            PageDirectoryPointerTableEntry PDPTE = PDPTEPtr->Entries[Index.PDPTEIndex];
            PDPTE.raw |= Flags;
            PDPTE.PageSize = true;
            PDPTE.SetAddress((uintptr_t)PhysicalAddress >> 12);
            PDPTEPtr->Entries[Index.PDPTEIndex] = PDPTE;
            return;
        }

        PML4.raw |= DirectoryFlags;
        this->Table->Entries[Index.PMLIndex] = PML4;

        PageDirectoryPointerTableEntry PDPTE = PDPTEPtr->Entries[Index.PDPTEIndex];
        PageDirectoryEntryPtr *PDEPtr = nullptr;
        if (!PDPTE.Present)
        {
            PDEPtr = (PageDirectoryEntryPtr *)KernelAllocator.RequestPage();
            memset(PDEPtr, 0, PAGE_SIZE);
            PDPTE.Present = true;
            PDPTE.SetAddress((uintptr_t)PDEPtr >> 12);
        }
        else
            PDEPtr = (PageDirectoryEntryPtr *)((uintptr_t)PDPTE.GetAddress() << 12);

        if (Type == MapType::TwoMB)
        {
            PageDirectoryEntry PDE = PDEPtr->Entries[Index.PDEIndex];
            PDE.raw |= Flags;
            PDE.PageSize = true;
            PDE.SetAddress((uintptr_t)PhysicalAddress >> 12);
            PDEPtr->Entries[Index.PDEIndex] = PDE;
            return;
        }

        PDPTE.raw |= DirectoryFlags;
        PDPTEPtr->Entries[Index.PDPTEIndex] = PDPTE;

        PageDirectoryEntry PDE = PDEPtr->Entries[Index.PDEIndex];
        PageTableEntryPtr *PTEPtr = nullptr;
        if (!PDE.Present)
        {
            PTEPtr = (PageTableEntryPtr *)KernelAllocator.RequestPage();
            memset(PTEPtr, 0, PAGE_SIZE);
            PDE.Present = true;
            PDE.SetAddress((uintptr_t)PTEPtr >> 12);
        }
        else
            PTEPtr = (PageTableEntryPtr *)((uintptr_t)PDE.GetAddress() << 12);
        PDE.raw |= DirectoryFlags;
        PDEPtr->Entries[Index.PDEIndex] = PDE;

        PageTableEntry PTE = PTEPtr->Entries[Index.PTEIndex];
        PTE.Present = true;
        PTE.raw |= Flags;
        PTE.SetAddress((uintptr_t)PhysicalAddress >> 12);
        PTEPtr->Entries[Index.PTEIndex] = PTE;

#if defined(a64)
        CPU::x64::invlpg(VirtualAddress);
#elif defined(a32)
        CPU::x32::invlpg(VirtualAddress);
#elif defined(aa64)
        asmv("dsb sy");
        asmv("tlbi vae1is, %0"
             :
             : "r"(VirtualAddress)
             : "memory");
        asmv("dsb sy");
        asmv("isb");
#endif

#ifdef DEBUG
/* https://stackoverflow.com/a/3208376/9352057 */
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)       \
    (byte & 0x80 ? '1' : '0'),     \
        (byte & 0x40 ? '1' : '0'), \
        (byte & 0x20 ? '1' : '0'), \
        (byte & 0x10 ? '1' : '0'), \
        (byte & 0x08 ? '1' : '0'), \
        (byte & 0x04 ? '1' : '0'), \
        (byte & 0x02 ? '1' : '0'), \
        (byte & 0x01 ? '1' : '0')

        if (!this->Check(VirtualAddress, (PTFlag)Flags, Type)) // quick workaround just to see where it fails
            warn("Failed to map %#lx - %#lx with flags: " BYTE_TO_BINARY_PATTERN, VirtualAddress, PhysicalAddress, BYTE_TO_BINARY(Flags));
#endif
    }

    void Virtual::Map(void *VirtualAddress, void *PhysicalAddress, size_t Length, uint64_t Flags, MapType Type)
    {
        int PageSize = PAGE_SIZE_4K;

        if (Type == MapType::TwoMB)
            PageSize = PAGE_SIZE_2M;
        else if (Type == MapType::OneGB)
            PageSize = PAGE_SIZE_1G;

        for (uintptr_t i = 0; i < Length; i += PageSize)
            this->Map((void *)((uintptr_t)VirtualAddress + i), (void *)((uintptr_t)PhysicalAddress + i), Flags, Type);
    }

    void Virtual::Unmap(void *VirtualAddress, MapType Type)
    {
        SmartLock(this->MemoryLock);
        if (!this->Table)
        {
            error("No page table");
            return;
        }

        PageMapIndexer Index = PageMapIndexer((uintptr_t)VirtualAddress);
        PageMapLevel4 PML4 = this->Table->Entries[Index.PMLIndex];
        if (!PML4.Present)
        {
            error("Page %#lx not present", PML4.GetAddress());
            return;
        }

        PageDirectoryPointerTableEntryPtr *PDPTEPtr = (PageDirectoryPointerTableEntryPtr *)((uintptr_t)PML4.Address << 12);
        PageDirectoryPointerTableEntry PDPTE = PDPTEPtr->Entries[Index.PDPTEIndex];
        if (!PDPTE.Present)
        {
            error("Page %#lx not present", PDPTE.GetAddress());
            return;
        }

        if (Type == MapType::OneGB && PDPTE.PageSize)
        {
            PDPTE.Present = false;
            PDPTEPtr->Entries[Index.PDPTEIndex] = PDPTE;
            return;
        }

        PageDirectoryEntryPtr *PDEPtr = (PageDirectoryEntryPtr *)((uintptr_t)PDPTE.Address << 12);
        PageDirectoryEntry PDE = PDEPtr->Entries[Index.PDEIndex];
        if (!PDE.Present)
        {
            error("Page %#lx not present", PDE.GetAddress());
            return;
        }

        if (Type == MapType::TwoMB && PDE.PageSize)
        {
            PDE.Present = false;
            PDEPtr->Entries[Index.PDEIndex] = PDE;
            return;
        }

        PageTableEntryPtr *PTEPtr = (PageTableEntryPtr *)((uintptr_t)PDE.Address << 12);
        PageTableEntry PTE = PTEPtr->Entries[Index.PTEIndex];
        if (!PTE.Present)
        {
            error("Page %#lx not present", PTE.GetAddress());
            return;
        }

        PTE.Present = false;
        PTEPtr->Entries[Index.PTEIndex] = PTE;

#if defined(a64)
        CPU::x64::invlpg(VirtualAddress);
#elif defined(a32)
        CPU::x32::invlpg(VirtualAddress);
#elif defined(aa64)
        asmv("dsb sy");
        asmv("tlbi vae1is, %0"
             :
             : "r"(VirtualAddress)
             : "memory");
        asmv("dsb sy");
        asmv("isb");
#endif
    }

    void Virtual::Unmap(void *VirtualAddress, size_t Length, MapType Type)
    {
        int PageSize = PAGE_SIZE_4K;

        if (Type == MapType::TwoMB)
            PageSize = PAGE_SIZE_2M;
        else if (Type == MapType::OneGB)
            PageSize = PAGE_SIZE_1G;

        for (uintptr_t i = 0; i < Length; i += PageSize)
            this->Unmap((void *)((uintptr_t)VirtualAddress + i), Type);
    }

    void Virtual::Remap(void *VirtualAddress, void *PhysicalAddress, uint64_t Flags, MapType Type)
    {
        this->Unmap(VirtualAddress, Type);
        this->Map(VirtualAddress, PhysicalAddress, Flags, Type);
    }

    Virtual::Virtual(PageTable4 *Table)
    {
        if (Table)
            this->Table = Table;
        else
            this->Table = (PageTable4 *)CPU::PageTable();
    }

    Virtual::~Virtual() {}
}
