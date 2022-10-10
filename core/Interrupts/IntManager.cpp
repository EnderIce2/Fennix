#include <interrupts.hpp>

#if defined(__amd64__)
#include "../arch/amd64/cpu/gdt.hpp"
#include "../arch/amd64/cpu/idt.hpp"
#elif defined(__i386__)
#include "../arch/i686/cpu/gdt.hpp"
#include "../arch/i686/cpu/idt.hpp"
#elif defined(__aarch64__)
#include "../arch/aarch64/cpu/gdt.hpp"
#include "../arch/aarch64/cpu/idt.hpp"
#endif

namespace Interrupts
{
    void Initialize()
    {
#if defined(__amd64__)
        GlobalDescriptorTable::Init(0);
        InterruptDescriptorTable::Init(0);
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
    }
}
