#include <smp.hpp>

#include <cpu.hpp>

#include "../../../kernel.h"

extern "C" uint64_t _trampoline_start, _trampoline_end;

#define TRAMPOLINE_START 0x2000

enum SMPTrampolineAddress
{
    PAGE_TABLE = 0x500,
    START_ADDR = 0x520,
    STACK = 0x570,
    GDT = 0x580,
    IDT = 0x590,
};

volatile bool CPUEnabled = false;

extern "C" void StartCPU()
{
    CPUEnabled = true;
    CPU::Stop();
}
