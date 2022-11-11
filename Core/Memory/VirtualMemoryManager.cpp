#include <memory.hpp>

#include <convert.h>
#include <debug.h>

namespace Memory
{
    bool Virtual::Check(void *VirtualAddress, PTFlag Flag)
    {
        // 0x1000 aligned
        uint64_t Address = (uint64_t)VirtualAddress;
        Address &= 0xFFFFFFFFFFFFF000;

        PageMapIndexer Index = PageMapIndexer((uint64_t)Address);
        PageDirectoryEntry PDE = this->Table->Entries[Index.PDPIndex];
        PageTable *PDP = nullptr;
        PageTable *PD = nullptr;
        PageTable *PT = nullptr;
        if (PDE.GetFlag(Flag))
            PDP = (PageTable *)((uint64_t)PDE.GetAddress() << 12);
        else
            return false;

        PDE = PDP->Entries[Index.PDIndex];
        if (PDE.GetFlag(Flag))
            PD = (PageTable *)((uint64_t)PDE.GetAddress() << 12);
        else
            return false;

        PDE = PD->Entries[Index.PTIndex];
        if (PDE.GetFlag(Flag))
            PT = (PageTable *)((uint64_t)PDE.GetAddress() << 12);
        else
            return false;

        PDE = PT->Entries[Index.PIndex];
        if (PDE.GetFlag(Flag))
            return true;
        else
            return false;

        return false;
    }

    void Virtual::Map(void *VirtualAddress, void *PhysicalAddress, uint64_t Flags)
    {
        SmartLock(this->MemoryLock);
        if (!this->Table)
        {
            error("No page table");
            return;
        }
        PageMapIndexer Index = PageMapIndexer((uint64_t)VirtualAddress);
        PageDirectoryEntry PDE = this->Table->Entries[Index.PDPIndex];
        PageTable *PDP = nullptr;
        if (!PDE.GetFlag(PTFlag::P))
        {
            PDP = (PageTable *)KernelAllocator.RequestPage();
            memset(PDP, 0, PAGE_SIZE);
            PDE.SetAddress((uint64_t)PDP >> 12);
            PDE.SetFlag(PTFlag::P, true);
            PDE.AddFlag(Flags);
            this->Table->Entries[Index.PDPIndex] = PDE;
        }
        else
            PDP = (PageTable *)((uint64_t)PDE.GetAddress() << 12);

        PDE = PDP->Entries[Index.PDIndex];
        PageTable *PD = nullptr;
        if (!PDE.GetFlag(PTFlag::P))
        {
            PD = (PageTable *)KernelAllocator.RequestPage();
            memset(PD, 0, PAGE_SIZE);
            PDE.SetAddress((uint64_t)PD >> 12);
            PDE.SetFlag(PTFlag::P, true);
            PDE.AddFlag(Flags);
            PDP->Entries[Index.PDIndex] = PDE;
        }
        else
            PD = (PageTable *)((uint64_t)PDE.GetAddress() << 12);

        PDE = PD->Entries[Index.PTIndex];
        PageTable *PT = nullptr;
        if (!PDE.GetFlag(PTFlag::P))
        {
            PT = (PageTable *)KernelAllocator.RequestPage();
            memset(PT, 0, PAGE_SIZE);
            PDE.SetAddress((uint64_t)PT >> 12);
            PDE.SetFlag(PTFlag::P, true);
            PDE.AddFlag(Flags);
            PD->Entries[Index.PTIndex] = PDE;
        }
        else
            PT = (PageTable *)((uint64_t)PDE.GetAddress() << 12);

        PDE = PT->Entries[Index.PIndex];
        PDE.SetAddress((uint64_t)PhysicalAddress >> 12);
        PDE.SetFlag(PTFlag::P, true);
        PDE.AddFlag(Flags);
        PT->Entries[Index.PIndex] = PDE;
#if defined(__amd64__)
        CPU::x64::invlpg(VirtualAddress);
#elif defined(__i386__)
        CPU::x86::invlpg(VirtualAddress);
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
            warn("Failed to map %#lx with flags: " BYTE_TO_BINARY_PATTERN, VirtualAddress, BYTE_TO_BINARY(Flags));
#endif
    }

    void Virtual::Map(void *VirtualAddress, void *PhysicalAddress, uint64_t PageCount, uint64_t Flags)
    {
        for (uint64_t i = 0; i < PageCount; i++)
            this->Map((void *)((uint64_t)VirtualAddress + (i * PAGE_SIZE)), (void *)((uint64_t)PhysicalAddress + (i * PAGE_SIZE)), Flags);
    }

    void Virtual::Unmap(void *VirtualAddress)
    {
        SmartLock(this->MemoryLock);
        if (!this->Table)
        {
            error("No page table");
            return;
        }

        PageMapIndexer Index = PageMapIndexer((uint64_t)VirtualAddress);
        PageDirectoryEntry PDE = this->Table->Entries[Index.PDPIndex];
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

    Virtual::Virtual(PageTable *Table)
    {
        if (Table)
            this->Table = Table;
        else
            this->Table = (PageTable *)CPU::PageTable();
    }

    Virtual::~Virtual() {}
}
