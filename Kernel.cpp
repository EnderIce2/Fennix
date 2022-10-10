#include "kernel.h"

#include <interrupts.hpp>
#include <memory.hpp>
#include <string.h>
#include <printf.h>
#include <lock.hpp>
#include <time.hpp>
#include <debug.h>

NEWLOCK(KernelLock);

BootInfo *bInfo = nullptr;
Video::Display *Display = nullptr;
SymbolResolver::Symbols *KernelSymbolTable = nullptr;
Power::Power *PowerManager = nullptr;
PCI::PCI *PCIManager = nullptr;

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
    trace("Hello, World!");
    InitializeMemoryManagement(Info);
    bInfo = (BootInfo *)KernelAllocator.RequestPages(TO_PAGES(sizeof(BootInfo)));
    memcpy(bInfo, Info, sizeof(BootInfo));
    debug("BootInfo structure is at %p", bInfo);
    Display = new Video::Display(bInfo->Framebuffer[0]);
    printf_("\eFFFFFF%s - %s [\e058C19%s\eFFFFFF]\n", KERNEL_NAME, KERNEL_VERSION, GIT_COMMIT_SHORT);
    /**************************************************************************************/
    KPrint("Initializing GDT and IDT");
    Interrupts::Initialize();
    KPrint("Loading kernel symbols");
    KernelSymbolTable = new SymbolResolver::Symbols((uint64_t)Info->Kernel.FileBase);
    KPrint("Initializing Power Manager");
    PowerManager = new Power::Power;
    KPrint("Initializing PCI Manager");
    PCIManager = new PCI::PCI;
    foreach (auto hdr in PCIManager->GetDevices())
    {
        KPrint("Found PCI device: %s / %s / %s / %s / %s",
               PCI::Descriptors::GetVendorName(hdr->VendorID),
               PCI::Descriptors::GetDeviceName(hdr->VendorID, hdr->DeviceID),
               PCI::Descriptors::DeviceClasses[hdr->Class],
               PCI::Descriptors::GetSubclassName(hdr->Class, hdr->Subclass),
               PCI::Descriptors::GetProgIFName(hdr->Class, hdr->Subclass, hdr->ProgIF));
    }
    while (1)
        CPU::Halt();
}
