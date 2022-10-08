#include <cpu.hpp>

namespace CPU
{
    void Pause()
    {
#if defined(__amd64__) || defined(__i386__)
        asmv("pause");
#elif defined(__aarch64__)
        asmv("yield");
#endif
    }

    void Halt()
    {
#if defined(__amd64__) || defined(__i386__)
        asmv("hlt");
#elif defined(__aarch64__)
        asmv("wfe");
#endif
    }

    bool Interrupts(InterruptsType Type)
    {
        switch (Type)
        {
        case Check:
        {
#if defined(__amd64__) || defined(__i386__)
            uint64_t rflags;
            asmv("pushfq");
            asmv("popq %0"
                 : "=r"(rflags));
            return rflags & (1 << 9);
#elif defined(__aarch64__)
            uint64_t daif;
            asmv("mrs %0, daif"
                 : "=r"(daif));
            return !(daif & (1 << 2));
#endif
        }
        case Enable:
        {
#if defined(__amd64__) || defined(__i386__)
            asmv("sti");
#elif defined(__aarch64__)
            asmv("msr daifclr, #2");
#endif
            return true;
        }
        case Disable:
        {
#if defined(__amd64__) || defined(__i386__)
            asmv("cli");
#elif defined(__aarch64__)
            asmv("msr daifset, #2");
#endif
            return true;
        }
        }
        return false;
    }
}
