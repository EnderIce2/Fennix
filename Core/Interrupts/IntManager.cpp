#include <interrupts.hpp>

#include <syscalls.hpp>

#if defined(__amd64__)
#include "../Architecture/amd64/cpu/gdt.hpp"
#include "../Architecture/amd64/cpu/idt.hpp"
#include "../Architecture/amd64/acpi.hpp"
#include "../Architecture/amd64/cpu/apic.hpp"
#elif defined(__i386__)
#include "../Architecture/i686/cpu/gdt.hpp"
#include "../Architecture/i686/cpu/idt.hpp"
#elif defined(__aarch64__)
#endif

#include "../crashhandler.hpp"
#include "../kernel.h"

extern "C" void ExceptionHandler(void *Data) { CrashHandler::Handle(Data); }

namespace Interrupts
{
#if defined(__amd64__)
    /* APIC::APIC */ void *apic = nullptr;
#elif defined(__i386__)
    /* APIC::APIC */ void *apic = nullptr;
#elif defined(__aarch64__)
#endif

    void Initialize(int Core)
    {
#if defined(__amd64__)
        GlobalDescriptorTable::Init(0);
        InterruptDescriptorTable::Init(0);
        InitializeSystemCalls();
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
    }

    void Enable()
    {
#if defined(__amd64__)
        if (((ACPI::MADT *)PowerManager->GetMADT())->LAPICAddress != nullptr)
        {
            apic = new APIC::APIC;
            ((APIC::APIC *)apic)->RedirectIRQs(0);
        }
        else
        {
            error("LAPIC not found");
            // TODO: PIC
        }
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
    }

    void InitializeTimer()
    {
#if defined(__amd64__)
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
    }

    Vector<Handler *> RegisteredEvents;

    extern "C" void MainInterruptHandler(void *Data)
    {
#if defined(__amd64__)
        CPU::x64::TrapFrame *Frame = (CPU::x64::TrapFrame *)Data;
        if (RegisteredEvents[Frame->int_num])
            RegisteredEvents[Frame->int_num]->OnInterruptReceived(Frame);

        if (apic)
        {
            ((APIC::APIC *)Interrupts::apic)->EOI();
            return;
        }
        // TODO: PIC
#elif defined(__i386__)
        void *Frame = Data;
#elif defined(__aarch64__)
        void *Frame = Data;
#endif
        while (1)
            CPU::Stop();
    }

    Handler::Handler(int InterruptNumber)
    {
        debug("Handler::Handler(%d)", InterruptNumber);
        this->InterruptNumber = InterruptNumber;
        RegisteredEvents.push_back(this);
    }

    Handler::~Handler()
    {
        for (uint64_t i = 0; i < RegisteredEvents.size(); i++)
            if (RegisteredEvents[i] == this)
            {
                debug("Handler::~Handler(%d)", InterruptNumber);
                RegisteredEvents.remove(i);
                return;
            }
    }

#if defined(__amd64__)
    void Handler::OnInterruptReceived(CPU::x64::TrapFrame *Frame)
    {
        warn("Unhandled interrupt received %#lx", Frame->int_num);
#elif defined(__i386__)
    void Handler::OnInterruptReceived(void *Frame);
    {
        warn("Unhandled interrupt received");
#elif defined(__aarch64__)
    void Handler::OnInterruptReceived(void *Frame);
    {
        warn("Unhandled interrupt received");
#endif
    }
}
