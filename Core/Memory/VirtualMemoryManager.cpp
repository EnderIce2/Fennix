#include <memory.hpp>

#include <convert.h>
#include <debug.h>

namespace Memory
{
    bool Virtual::Check(void *VirtualAddress, PTFlag Flag)
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
                if ((PDPTE->Entries[Index.PDPTEIndex].Present))
                {
                    PDE = (PageDirectoryEntryPtr *)((uintptr_t)PDPTE->Entries[Index.PDPTEIndex].GetAddress() << 12);
                    if (PDE)
                        if ((PDE->Entries[Index.PDEIndex].Present))
                        {
                            PTE = (PageTableEntryPtr *)((uintptr_t)PDE->Entries[Index.PDEIndex].GetAddress() << 12);
                            if (PTE)
                                if ((PTE->Entries[Index.PTEIndex].Present))
                                {
                                    return true;
                                }
                        }
                }
        }
        return false;
    }

    void Virtual::Map(void *VirtualAddress, void *PhysicalAddress, uint64_t Flags)
    {
        SmartLock(this->MemoryLock);
        if (unlikely(!this->Table))
        {
            error("No page table");
            return;
        }

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

#if defined(__amd64__)
        CPU::x64::invlpg(VirtualAddress);
#elif defined(__i386__)
        CPU::x32::invlpg(VirtualAddress);
#elif defined(__aarch64__)
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

        if (!this->Check(VirtualAddress, (PTFlag)Flags)) // quick workaround just to see where it fails
            warn("Failed to map %#lx - %#lx with flags: " BYTE_TO_BINARY_PATTERN, VirtualAddress, PhysicalAddress, BYTE_TO_BINARY(Flags));
#endif
    }

    void Virtual::Map(void *VirtualAddress, void *PhysicalAddress, size_t PageCount, uint64_t Flags)
    {
        for (size_t i = 0; i < PageCount; i++)
            this->Map((void *)((uintptr_t)VirtualAddress + (i * PAGE_SIZE)), (void *)((uintptr_t)PhysicalAddress + (i * PAGE_SIZE)), Flags);
    }

    void Virtual::Unmap(void *VirtualAddress)
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
            error("Page not present");
            return;
        }
        PageDirectoryPointerTableEntryPtr *PDPTEPtr = (PageDirectoryPointerTableEntryPtr *)((uintptr_t)PML4.Address << 12);
        PageDirectoryPointerTableEntry PDPTE = PDPTEPtr->Entries[Index.PDPTEIndex];
        if (!PDPTE.Present)
        {
            error("Page not present");
            return;
        }
        PageDirectoryEntryPtr *PDEPtr = (PageDirectoryEntryPtr *)((uintptr_t)PDPTE.Address << 12);
        PageDirectoryEntry PDE = PDEPtr->Entries[Index.PDEIndex];
        if (!PDE.Present)
        {
            error("Page not present");
            return;
        }
        PageTableEntryPtr *PTEPtr = (PageTableEntryPtr *)((uintptr_t)PDE.Address << 12);
        PageTableEntry PTE = PTEPtr->Entries[Index.PTEIndex];
        if (!PTE.Present)
        {
            error("Page not present");
            return;
        }

        PTE.Present = false;
        PTEPtr->Entries[Index.PTEIndex] = PTE;

#if defined(__amd64__)
        CPU::x64::invlpg(VirtualAddress);
#elif defined(__i386__)
        CPU::x32::invlpg(VirtualAddress);
#elif defined(__aarch64__)
        asmv("dsb sy");
        asmv("tlbi vae1is, %0"
             :
             : "r"(VirtualAddress)
             : "memory");
        asmv("dsb sy");
        asmv("isb");
#endif
    }

    void Virtual::Unmap(void *VirtualAddress, size_t PageCount)
    {
        for (size_t i = 0; i < PageCount; i++)
            this->Unmap((void *)((uintptr_t)VirtualAddress + (i * PAGE_SIZE)));
    }

    void Virtual::Remap(void *VirtualAddress, void *PhysicalAddress, uint64_t Flags)
    {
        this->Unmap(VirtualAddress);
        this->Map(VirtualAddress, PhysicalAddress, Flags);
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
