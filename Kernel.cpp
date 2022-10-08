#include "kernel.h"

#include <memory.hpp>
#include <string.h>
#include <debug.h>

BootInfo *bInfo = nullptr;

EXTERNC void kernel_aarch64_entry(uint64_t dtb_ptr32, uint64_t x1, uint64_t x2, uint64_t x3)
{
    trace("Hello, World!");
    while (1)
        CPU::Halt();
}

EXTERNC void kernel_entry(BootInfo *Info)
{
    InitializeMemoryManagement(Info);
    trace("Hello, World!");
    bInfo = (BootInfo *)KernelAllocator.RequestPages(TO_PAGES(sizeof(BootInfo)));
    memcpy(bInfo, Info, sizeof(BootInfo));
    debug("BootInfo structure is at %p", bInfo);
    while (1)
        CPU::Halt();
}

// TODO: Implement screen printing
extern "C" void putchar(int a, int b)
{
}
