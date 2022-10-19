#include <types.h>

#include <debug.h>
#include <cpu.hpp>

EXTERNC void arm64Entry(uint64_t dtb_ptr32, uint64_t x1, uint64_t x2, uint64_t x3)
{
    trace("Hello, World!");
    while (1)
        CPU::Halt();
}
