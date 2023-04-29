#include <memory.hpp>
#include <debug.h>

namespace Memory32
{
    void Virtual::Map(void *VirtualAddress, void *PhysicalAddress, uint64_t Flags)
    {
        if (unlikely(!this->Table))
        {
            error("No page table");
            return;
        }

        Flags |= PTFlag::P;

        PageMapIndexer Index = PageMapIndexer((uintptr_t)VirtualAddress);
        // Clear any flags that are not 1 << 0 (Present) - 1 << 5 (Accessed) because rest are for page table entries only
        uint64_t DirectoryFlags = Flags & 0x3F;

        PageDirectoryEntry *PDE = &this->Table->Entries[Index.PDEIndex];
        PageTableEntryPtr *PTEPtr = nullptr;
        if (!PDE->Present)
        {
            PTEPtr = (PageTableEntryPtr *)KernelAllocator32.RequestPages(TO_PAGES(sizeof(PageTableEntryPtr) + 1));
            memset(PTEPtr, 0, sizeof(PageTableEntryPtr));
            PDE->Present = true;
            PDE->SetAddress((uintptr_t)PTEPtr >> 12);
        }
        else
            PTEPtr = (PageTableEntryPtr *)(PDE->GetAddress() << 12);
        PDE->raw |= DirectoryFlags;

        PageTableEntry *PTE = &PTEPtr->Entries[Index.PTEIndex];
        PTE->Present = true;
        PTE->raw |= Flags;
        PTE->SetAddress((uintptr_t)PhysicalAddress >> 12);

        asmv("invlpg (%0)" ::"r"(VirtualAddress)
             : "memory");
    }

    Virtual::Virtual(PageTable *Table)
    {
        if (Table)
            this->Table = Table;
        else
        {
			asmv("movl %%cr3, %0"
				 : "=r"(this->Table));
        }
    }

    Virtual::~Virtual() {}
}

namespace Memory64
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
        if (unlikely(!this->Table))
        {
            error("No page table");
            return;
        }

        Flags |= PTFlag::P;

        PageMapIndexer Index = PageMapIndexer((uintptr_t)VirtualAddress);
        // Clear any flags that are not 1 << 0 (Present) - 1 << 5 (Accessed) because rest are for page table entries only
        uint64_t DirectoryFlags = Flags & 0x3F;

        PageMapLevel4 *PML4 = &this->Table->Entries[Index.PMLIndex];
        PageDirectoryPointerTableEntryPtr *PDPTEPtr = nullptr;
        if (!PML4->Present)
        {
            PDPTEPtr = (PageDirectoryPointerTableEntryPtr *)KernelAllocator64.RequestPages(TO_PAGES(sizeof(PageDirectoryPointerTableEntryPtr) + 1));
            memset(PDPTEPtr, 0, sizeof(PageDirectoryPointerTableEntryPtr));
            PML4->Present = true;
            PML4->SetAddress((uintptr_t)PDPTEPtr >> 12);
        }
        else
            PDPTEPtr = (PageDirectoryPointerTableEntryPtr *)(PML4->GetAddress() << 12);
        PML4->raw |= DirectoryFlags;

        PageDirectoryPointerTableEntry *PDPTE = &PDPTEPtr->Entries[Index.PDPTEIndex];
        if (Type == MapType::OneGB)
        {
            PDPTE->raw |= Flags;
            PDPTE->PageSize = true;
            PDPTE->SetAddress((uintptr_t)PhysicalAddress >> 12);
            debug("Mapped 1GB page at %p to %p", VirtualAddress, PhysicalAddress);
            return;
        }

        PageDirectoryEntryPtr *PDEPtr = nullptr;
        if (!PDPTE->Present)
        {
            PDEPtr = (PageDirectoryEntryPtr *)KernelAllocator64.RequestPages(TO_PAGES(sizeof(PageDirectoryEntryPtr) + 1));
            memset(PDEPtr, 0, sizeof(PageDirectoryEntryPtr));
            PDPTE->Present = true;
            PDPTE->SetAddress((uintptr_t)PDEPtr >> 12);
        }
        else
            PDEPtr = (PageDirectoryEntryPtr *)(PDPTE->GetAddress() << 12);
        PDPTE->raw |= DirectoryFlags;

        PageDirectoryEntry *PDE = &PDEPtr->Entries[Index.PDEIndex];
        if (Type == MapType::TwoMB)
        {
            PDE->raw |= Flags;
            PDE->PageSize = true;
            PDE->SetAddress((uintptr_t)PhysicalAddress >> 12);
            debug("Mapped 2MB page at %p to %p", VirtualAddress, PhysicalAddress);
            return;
        }

        PageTableEntryPtr *PTEPtr = nullptr;
        if (!PDE->Present)
        {
            PTEPtr = (PageTableEntryPtr *)KernelAllocator64.RequestPages(TO_PAGES(sizeof(PageTableEntryPtr) + 1));
            memset(PTEPtr, 0, sizeof(PageTableEntryPtr));
            PDE->Present = true;
            PDE->SetAddress((uintptr_t)PTEPtr >> 12);
        }
        else
            PTEPtr = (PageTableEntryPtr *)(PDE->GetAddress() << 12);
        PDE->raw |= DirectoryFlags;

        PageTableEntry *PTE = &PTEPtr->Entries[Index.PTEIndex];
        PTE->Present = true;
        PTE->raw |= Flags;
        PTE->SetAddress((uintptr_t)PhysicalAddress >> 12);

        asmv("invlpg (%0)" ::"r"(VirtualAddress)
             : "memory");

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
            warn("Failed to map v:%#lx p:%#lx with flags: " BYTE_TO_BINARY_PATTERN, VirtualAddress, PhysicalAddress, BYTE_TO_BINARY(Flags));
#endif
    }

    void Virtual::Unmap(void *VirtualAddress, MapType Type)
    {
        if (!this->Table)
        {
            error("No page table");
            return;
        }

        PageMapIndexer Index = PageMapIndexer((uintptr_t)VirtualAddress);
        PageMapLevel4 *PML4 = &this->Table->Entries[Index.PMLIndex];
        if (!PML4->Present)
        {
            error("Page %#lx not present", PML4->GetAddress());
            return;
        }

        PageDirectoryPointerTableEntryPtr *PDPTEPtr = (PageDirectoryPointerTableEntryPtr *)((uintptr_t)PML4->Address << 12);
        PageDirectoryPointerTableEntry *PDPTE = &PDPTEPtr->Entries[Index.PDPTEIndex];
        if (!PDPTE->Present)
        {
            error("Page %#lx not present", PDPTE->GetAddress());
            return;
        }

        if (Type == MapType::OneGB && PDPTE->PageSize)
        {
            PDPTE->Present = false;
            return;
        }

        PageDirectoryEntryPtr *PDEPtr = (PageDirectoryEntryPtr *)((uintptr_t)PDPTE->Address << 12);
        PageDirectoryEntry *PDE = &PDEPtr->Entries[Index.PDEIndex];
        if (!PDE->Present)
        {
            error("Page %#lx not present", PDE->GetAddress());
            return;
        }

        if (Type == MapType::TwoMB && PDE->PageSize)
        {
            PDE->Present = false;
            return;
        }

        PageTableEntryPtr *PTEPtr = (PageTableEntryPtr *)((uintptr_t)PDE->Address << 12);
        PageTableEntry PTE = PTEPtr->Entries[Index.PTEIndex];
        if (!PTE.Present)
        {
            error("Page %#lx not present", PTE.GetAddress());
            return;
        }

        PTE.Present = false;
        PTEPtr->Entries[Index.PTEIndex] = PTE;

        asmv("invlpg (%0)" ::"r"(VirtualAddress)
             : "memory");
    }

    Virtual::Virtual(PageTable *Table)
    {
        if (Table)
            this->Table = Table;
        else
        {
			asmv("movl %%cr3, %0" // FIXME: movq
				 : "=r"(this->Table));
        }
    }

    Virtual::~Virtual() {}
}
