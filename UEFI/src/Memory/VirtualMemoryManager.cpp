#include "memory.hpp"

extern "C" void printf(const char* format, ...);

void *memset(void *dest, int c, size_t n)
{
    unsigned int i;
    for (i = 0; i < n; i++)
        ((char *)dest)[i] = c;
    return dest;
}

namespace Memory
{
    void Virtual::Map(void *VirtualAddress, void *PhysicalAddress, uint64_t Flags)
    {
        if (!this->Table)
        {
            printf("No page table");
            return;
        }
        PageMapIndexer Index = PageMapIndexer((uint64_t)VirtualAddress);
        PageDirectoryEntry PDE = this->Table->Entries[Index.PDP_i];
        PageTable *PDP;
        if (!PDE.GetFlag(PTFlag::P))
        {
            PDP = (PageTable *)KernelAllocator.RequestPage();
            memset(PDP, 0, PAGE_SIZE);
            PDE.SetAddress((uint64_t)PDP >> 12);
            PDE.SetFlag(PTFlag::P, true);
            PDE.AddFlag(Flags);
            this->Table->Entries[Index.PDP_i] = PDE;
        }
        else
            PDP = (PageTable *)((uint64_t)PDE.GetAddress() << 12);

        PDE = PDP->Entries[Index.PD_i];
        PageTable *PD;
        if (!PDE.GetFlag(PTFlag::P))
        {
            PD = (PageTable *)KernelAllocator.RequestPage();
            memset(PD, 0, PAGE_SIZE);
            PDE.SetAddress((uint64_t)PD >> 12);
            PDE.SetFlag(PTFlag::P, true);
            PDE.AddFlag(Flags);
            PDP->Entries[Index.PD_i] = PDE;
        }
        else
            PD = (PageTable *)((uint64_t)PDE.GetAddress() << 12);

        PDE = PD->Entries[Index.PT_i];
        PageTable *PT;
        if (!PDE.GetFlag(PTFlag::P))
        {
            PT = (PageTable *)KernelAllocator.RequestPage();
            memset(PT, 0, PAGE_SIZE);
            PDE.SetAddress((uint64_t)PT >> 12);
            PDE.SetFlag(PTFlag::P, true);
            PDE.AddFlag(Flags);
            PD->Entries[Index.PT_i] = PDE;
        }
        else
            PT = (PageTable *)((uint64_t)PDE.GetAddress() << 12);

        PDE = PT->Entries[Index.P_i];
        PDE.SetAddress((uint64_t)PhysicalAddress >> 12);
        PDE.SetFlag(PTFlag::P, true);
        PDE.AddFlag(Flags);
        PT->Entries[Index.P_i] = PDE;
        __asm__ volatile("invlpg (%0)"
                         :
                         : "r"(VirtualAddress)
                         : "memory");
    }

    void Virtual::Map(void *VirtualAddress, void *PhysicalAddress, uint64_t PageCount, uint64_t Flags)
    {
        for (uint64_t i = 0; i < PageCount; i++)
            this->Map((void *)((uint64_t)VirtualAddress + (i * PAGE_SIZE)), (void *)((uint64_t)PhysicalAddress + (i * PAGE_SIZE)), Flags);
    }

    void Virtual::Unmap(void *VirtualAddress)
    {
        if (!this->Table)
        {
            printf("No page table");
            return;
        }

        PageMapIndexer Index = PageMapIndexer((uint64_t)VirtualAddress);
        PageDirectoryEntry PDE = this->Table->Entries[Index.PDP_i];
        PDE.ClearFlags();

        __asm__ volatile("invlpg (%0)"
                         :
                         : "r"(VirtualAddress)
                         : "memory");
    }

    void Virtual::Unmap(void *VirtualAddress, uint64_t PageCount)
    {
        for (uint64_t i = 0; i < PageCount; i++)
            this->Unmap((void *)((uint64_t)VirtualAddress + (i * PAGE_SIZE)));
    }

    Virtual::Virtual(PageTable *Table)
    {
        uint64_t cr3;
        __asm__ volatile("mov %%cr3, %0"
                         : "=r"(cr3));

        if (Table)
            this->Table = Table;
        else
            this->Table = (PageTable *)cr3;
    }

    Virtual::~Virtual() {}
}
