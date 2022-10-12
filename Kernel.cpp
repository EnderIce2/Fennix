#include "kernel.h"

#include <interrupts.hpp>
#include <memory.hpp>
#include <string.h>
#include <printf.h>
#include <lock.hpp>
#include <time.hpp>
#include <debug.h>
#include <smp.hpp>

NEWLOCK(KernelLock);

BootInfo *bInfo = nullptr;
Video::Display *Display = nullptr;
SymbolResolver::Symbols *KernelSymbolTable = nullptr;
Power::Power *PowerManager = nullptr;
PCI::PCI *PCIManager = nullptr;

Time BootClock;

// For the Display class. Printing on first buffer.
extern "C" void putchar(char c) { Display->Print(c, 0); }

#ifdef __debug_vscode__
extern "C" int printf_(const char *format, ...);
extern "C" int vprintf_(const char *format, va_list arg);
#endif

EXTERNC void KPrint(const char *format, ...)
{
    SMARTLOCK(KernelLock);
    Time tm = ReadClock();
    printf_("\eCCCCCC[\e00AEFF%02ld:%02ld:%02ld\eCCCCCC] ", tm.Hour, tm.Minute, tm.Second);
    va_list args;
    va_start(args, format);
    vprintf_(format, args);
    va_end(args);
    putchar('\n');
    Display->SetBuffer(0);
}

EXTERNC void aarch64Entry(uint64_t dtb_ptr32, uint64_t x1, uint64_t x2, uint64_t x3)
{
    trace("Hello, World!");
    while (1)
        CPU::Halt();
}

EXTERNC void Entry(BootInfo *Info)
{
    BootClock = ReadClock();
    trace("Hello, World!");
    InitializeMemoryManagement(Info);
    bInfo = (BootInfo *)KernelAllocator.RequestPages(TO_PAGES(sizeof(BootInfo)));
    memcpy(bInfo, Info, sizeof(BootInfo));
    debug("BootInfo structure is at %p", bInfo);
    Display = new Video::Display(bInfo->Framebuffer[0]);
    printf_("\eFFFFFF%s - %s [\e058C19%s\eFFFFFF]\n", KERNEL_NAME, KERNEL_VERSION, GIT_COMMIT_SHORT);
    /**************************************************************************************/
    KPrint("Time: \e8888FF%02d:%02d:%02d %02d/%02d/%02d UTC",
           BootClock.Hour, BootClock.Minute, BootClock.Second,
           BootClock.Day, BootClock.Month, BootClock.Year);
    KPrint("CPU: \e8822AA%s \e8888FF%s (\e058C19%s\e8888FF)", CPU::Vendor(), CPU::Name(), CPU::Hypervisor());
    GetCPU(0)->ID = 0;
    GetCPU(0)->IsActive = true;
    GetCPU(0)->Checksum = CPU_DATA_CHECKSUM;
    KPrint("Initializing GDT and IDT");
    Interrupts::Initialize(0);
    KPrint("Initializing CPU features");
#if defined(__amd64__)
    CPU::x64::CR0 cr0 = CPU::x64::readcr0();
    CPU::x64::CR4 cr4 = CPU::x64::readcr4();
    uint32_t rax, rbx, rcx, rdx;
    CPU::x64::cpuid(0x1, &rax, &rbx, &rcx, &rdx);
    if (rdx & CPU::x64::CPUID_FEAT_RDX_SSE)
    {
        debug("Enabling SSE support...");
        KPrint("SSE is supported.");
        cr0.EM = 0;
        cr0.MP = 1;
        cr4.OSFXSR = 1;
        cr4.OSXMMEXCPT = 1;
    }

    // Enable cpu cache but... how to use it?
    cr0.NW = 0;
    cr0.CD = 0;

    debug("Enabling UMIP, SMEP & SMAP support...");
    CPU::x64::cpuid(0x1, &rax, &rbx, &rcx, &rdx);
    if (rdx & CPU::x64::CPUID_FEAT_RDX_UMIP)
    {
        KPrint("UMIP is supported.");
        fixme("Not going to enable UMIP.");
        // cr4.UMIP = 1;
    }
    if (rdx & CPU::x64::CPUID_FEAT_RDX_SMEP)
    {
        KPrint("SMEP is supported.");
        cr4.SMEP = 1;
    }
    if (rdx & CPU::x64::CPUID_FEAT_RDX_SMAP)
    {
        KPrint("SMAP is supported.");
        cr4.SMAP = 1;
    }
    CPU::x64::writecr0(cr0);
    CPU::x64::writecr4(cr4);
    debug("Enabling PAT support...");
    CPU::x64::wrmsr(CPU::x64::MSR_CR_PAT, 0x6 | (0x0 << 8) | (0x1 << 16));
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
    KPrint("Loading kernel symbols");
    KernelSymbolTable = new SymbolResolver::Symbols((uint64_t)Info->Kernel.FileBase);
    KPrint("Initializing Power Manager");
    PowerManager = new Power::Power;
    KPrint("Initializing PCI Manager");
    PCIManager = new PCI::PCI;
    foreach (auto hdr in PCIManager->GetDevices())
    {
        KPrint("Found PCI device: \e8888FF%s \eCCCCCC/ \e8888FF%s \eCCCCCC/ \e8888FF%s \eCCCCCC/ \e8888FF%s \eCCCCCC/ \e8888FF%s",
               PCI::Descriptors::GetVendorName(hdr->VendorID),
               PCI::Descriptors::GetDeviceName(hdr->VendorID, hdr->DeviceID),
               PCI::Descriptors::DeviceClasses[hdr->Class],
               PCI::Descriptors::GetSubclassName(hdr->Class, hdr->Subclass),
               PCI::Descriptors::GetProgIFName(hdr->Class, hdr->Subclass, hdr->ProgIF));
    }
    KPrint("Enabling interrupts");
    Interrupts::Enable();
    KPrint("Initializing SMP");
    SMP::Initialize(PowerManager->GetMADT());
    KPrint("\e058C19######## \eE85230END \e058C19########");
    while (1)
        CPU::Halt();
}
