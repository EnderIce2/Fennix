#include <interrupts.hpp>

#include <syscalls.hpp>
#include <hashmap.hpp>

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

extern "C" __attribute__((no_stack_protector)) void ExceptionHandler(void *Data) { CrashHandler::Handle(Data); }

namespace Interrupts
{
    HashMap<int, uint64_t> *RegisteredEvents;

#if defined(__amd64__)
    /* APIC::APIC */ void *apic = nullptr;
#elif defined(__i386__)
    /* APIC::APIC */ void *apic = nullptr;
#elif defined(__aarch64__)
#endif

    void Initialize(int Core)
    {
        static int once = 0;
        if (!once++)
            RegisteredEvents = new HashMap<int, uint64_t>;

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

    extern "C" void MainInterruptHandler(void *Data)
    {
#if defined(__amd64__)
        CPU::x64::TrapFrame *Frame = (CPU::x64::TrapFrame *)Data;

        Handler *handler = (Handler *)RegisteredEvents->Get(Frame->InterruptNumber);
        if (handler != (Handler *)0xdeadbeef)
            handler->OnInterruptReceived(Frame);
        else
            error("Unhandled IRQ%d on CPU %d", Frame->InterruptNumber - 32, ((APIC::APIC *)Interrupts::apic)->Read(APIC::APIC::APIC_ID) >> 24);

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
        debug("Registering interrupt handler for IRQ%d", InterruptNumber - 32);
        this->InterruptNumber = InterruptNumber;
        RegisteredEvents->AddNode(InterruptNumber, (uint64_t)this);
    }

    Handler::~Handler()
    {
        debug("Unregistering interrupt handler for IRQ%d", InterruptNumber - 32);
        if (RegisteredEvents->DeleteNode(InterruptNumber) == 0xdeadbeef)
            warn("Node %d not found", InterruptNumber);
    }

#if defined(__amd64__)
    void Handler::OnInterruptReceived(CPU::x64::TrapFrame *Frame)
    {
        trace("Unhandled interrupt IRQ%d", Frame->InterruptNumber - 32);
#elif defined(__i386__)
    void Handler::OnInterruptReceived(void *Frame);
    {
        trace("Unhandled interrupt received");
#elif defined(__aarch64__)
    void Handler::OnInterruptReceived(void *Frame);
    {
        trace("Unhandled interrupt received");
#endif
    }
}
