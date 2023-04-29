#include <memory.hpp>

Memory32::Physical KernelAllocator32{};
Memory32::PageTable *KernelPageTable32 = nullptr;

Memory64::Physical KernelAllocator64{};
Memory64::PageTable *KernelPageTable64 = nullptr;

void InitializeMemoryManagement(BootInfo *Info, bool is32)
{
    if (is32)
    {
        trace("Initializing Physical Memory Manager");
        // KernelAllocator32 = Physical(); <- Already called in the constructor
        KernelAllocator32.Init(Info);
        info("Memory Info: %lldMB / %lldMB (%lldMB reserved)",
              TO_MB(KernelAllocator32.UsedMemory),
              TO_MB(KernelAllocator32.TotalMemory),
              TO_MB(KernelAllocator32.ReservedMemory));

        trace("Initializing Virtual Memory Manager");

        KernelPageTable32 = (Memory32::PageTable *)KernelAllocator32.RequestPages(TO_PAGES(PAGE_SIZE + 1));
        memset(KernelPageTable32, 0, PAGE_SIZE);

        {
            Memory32::Virtual va = Memory32::Virtual(KernelPageTable32);
            debug("Mapping from 0x0 to %#llx", Info->Memory.Size);
            size_t MemSize = Info->Memory.Size;
            va.Map((void *)0, (void *)0, MemSize, PTFlag::RW);

            debug("Mapping Framebuffer");
            int itrfb = 0;
            while (true)
            {
                if (!Info->Framebuffer[itrfb].BaseAddress)
                    break;

                va.Map((void *)Info->Framebuffer[itrfb].BaseAddress,
                       (void *)Info->Framebuffer[itrfb].BaseAddress,
                       Info->Framebuffer[itrfb].Pitch * Info->Framebuffer[itrfb].Height,
                       PTFlag::RW | PTFlag::US | PTFlag::G);
                itrfb++;
            }
        }

        trace("Applying new page table from address %#lx", KernelPageTable32);
        asmv("mov %0, %%cr3" ::"r"(KernelPageTable32));
    }
}
