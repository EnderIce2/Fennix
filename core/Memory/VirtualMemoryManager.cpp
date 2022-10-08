#include <memory.hpp>

#include <string.h>
#include <debug.h>

namespace Memory
{
    void Virtual::Map(void *VirtualAddress, void *PhysicalAddress, uint64_t Flags)
    {
        SMARTLOCK(this->MemoryLock);
        if (!this->Table)
        {
            error("No page table");
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
#if defined(__amd64__) || defined(__i386__)
        asmv("invlpg (%0)"
             :
             : "r"(VirtualAddress)
             : "memory");
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

    void Virtual::Map(void *VirtualAddress, void *PhysicalAddress, uint64_t PageCount, uint64_t Flags)
    {
        for (uint64_t i = 0; i < PageCount; i++)
            this->Map((void *)((uint64_t)VirtualAddress + (i * PAGE_SIZE)), (void *)((uint64_t)PhysicalAddress + (i * PAGE_SIZE)), Flags);
    }

    void Virtual::Unmap(void *VirtualAddress)
    {
        SMARTLOCK(this->MemoryLock);
        if (!this->Table)
        {
            error("No page table");
            return;
        }

        PageMapIndexer Index = PageMapIndexer((uint64_t)VirtualAddress);
        PageDirectoryEntry PDE = this->Table->Entries[Index.PDP_i];
        PDE.ClearFlags();

#if defined(__amd64__) || defined(__i386__)
        asmv("invlpg (%0)"
             :
             : "r"(VirtualAddress)
             : "memory");
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

    void Virtual::Unmap(void *VirtualAddress, uint64_t PageCount)
    {
        for (uint64_t i = 0; i < PageCount; i++)
            this->Unmap((void *)((uint64_t)VirtualAddress + (i * PAGE_SIZE)));
    }

    Virtual::Virtual(PageTable *Table) { this->Table = Table; }
    Virtual::~Virtual() {}
}
