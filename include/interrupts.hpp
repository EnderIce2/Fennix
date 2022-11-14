#ifndef __FENNIX_KERNEL_INTERRUPTS_H__
#define __FENNIX_KERNEL_INTERRUPTS_H__

#include <types.h>
#include <cpu.hpp>

namespace Interrupts
{
#if defined(__amd64__)
    /* APIC::APIC */ extern void *apic[256]; // MAX_CPU
    /* APIC::Timer */ extern void *apicTimer[256]; // MAX_CPU
#elif defined(__i386__)
    /* APIC::APIC */ extern void *apic[256]; // MAX_CPU
    /* APIC::Timer */ extern void *apicTimer[256]; // MAX_CPU
#elif defined(__aarch64__)
#endif
    void Initialize(int Core);
    void Enable(int Core);
    void InitializeTimer(int Core);

    class Handler
    {
    private:
        int InterruptNumber;

    protected:
        void SetInterruptNumber(int InterruptNumber) { this->InterruptNumber = InterruptNumber; }
        int GetInterruptNumber() { return InterruptNumber; }
        Handler(int InterruptNumber);
        ~Handler();

    public:
#if defined(__amd64__)
        virtual void OnInterruptReceived(CPU::x64::TrapFrame *Frame);
#elif defined(__i386__)
        virtual void OnInterruptReceived(void *Frame);
#elif defined(__aarch64__)
        virtual void OnInterruptReceived(void *Frame);
#endif
    };
}

#endif // !__FENNIX_KERNEL_INTERRUPTS_H__
