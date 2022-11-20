#include "kernel.h"

#include <boot/protocols/multiboot2.h>
#include <interrupts.hpp>
#include <memory.hpp>
#include <convert.h>
#include <printf.h>
#include <lock.hpp>
#include <debug.h>
#include <smp.hpp>
#include <cargs.h>
#include <io.h>
#include <dumper.hpp>

#include "Core/smbios.hpp"

NewLock(KernelLock);

BootInfo *bInfo = nullptr;
Video::Display *Display = nullptr;
SymbolResolver::Symbols *KernelSymbolTable = nullptr;
Power::Power *PowerManager = nullptr;
PCI::PCI *PCIManager = nullptr;
Tasking::Task *TaskManager = nullptr;
Time::time *TimeManager = nullptr;
FileSystem::Virtual *vfs = nullptr;

KernelConfig Config;
Time::Clock BootClock;

// For the Display class. Printing on first buffer as default.
EXTERNC void putchar(char c) { Display->Print(c, 0); }

EXTERNC void KPrint(const char *Format, ...)
{
    SmartLock(KernelLock);
    Time::Clock tm = Time::ReadClock();
    printf_("\eCCCCCC[\e00AEFF%02ld:%02ld:%02ld\eCCCCCC] ", tm.Hour, tm.Minute, tm.Second);
    va_list args;
    va_start(args, Format);
    vprintf_(Format, args);
    va_end(args);
    putchar('\n');
    Display->SetBuffer(0);
}

EXTERNC void Entry(BootInfo *Info)
{
    trace("Hello, World!");
    InitializeMemoryManagement(Info);
    BootClock = Time::ReadClock();
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
    KPrint("Initializing GDT and IDT");
    Interrupts::Initialize(0);
    KPrint("Initializing CPU Features");
    CPU::InitializeFeatures();
    KPrint("Loading Kernel Symbols");
    KernelSymbolTable = new SymbolResolver::Symbols((uint64_t)Info->Kernel.FileBase);
    KPrint("Reading Kernel Parameters");
    Config = ParseConfig((char *)bInfo->Kernel.CommandLine);
    KPrint("Initializing Power Manager");
    PowerManager = new Power::Power;
    KPrint("Initializing PCI Manager");
    PCIManager = new PCI::PCI;
    foreach (auto Device in PCIManager->GetDevices())
    {
        KPrint("PCI: \e8888FF%s \eCCCCCC/ \e8888FF%s \eCCCCCC/ \e8888FF%s \eCCCCCC/ \e8888FF%s \eCCCCCC/ \e8888FF%s",
               PCI::Descriptors::GetVendorName(Device->VendorID),
               PCI::Descriptors::GetDeviceName(Device->VendorID, Device->DeviceID),
               PCI::Descriptors::DeviceClasses[Device->Class],
               PCI::Descriptors::GetSubclassName(Device->Class, Device->Subclass),
               PCI::Descriptors::GetProgIFName(Device->Class, Device->Subclass, Device->ProgIF));
    }
    KPrint("Enabling Interrupts on Bootstrap Processor");
    Interrupts::Enable(0);
#if defined(__amd64__)
    PowerManager->InitDSDT();
#elif defined(__i386__)
    // FIXME: Add ACPI support for i386
#elif defined(__aarch64__)
#endif
    KPrint("Initializing Timers");
#if defined(__amd64__)
    TimeManager = new Time::time(PowerManager->GetACPI());
#elif defined(__i386__)
    TimeManager = new Time::time(PowerManager->GetACPI());
#elif defined(__aarch64__)
    TimeManager = new Time::time(nullptr);
#endif
    KPrint("Initializing Bootstrap Processor Timer");
    Interrupts::InitializeTimer(0);
    KPrint("Initializing SMP");
    SMP::Initialize(PowerManager->GetMADT());

    if (SMBIOS::CheckSMBIOS())
    {
        SMBIOS::SMBIOSEntryPoint *smbios = SMBIOS::GetSMBIOSEntryPoint();
        SMBIOS::SMBIOSBIOSInformation *bios = SMBIOS::GetBIOSInformation();
        SMBIOS::SMBIOSSystemInformation *system = SMBIOS::GetSystemInformation();
        SMBIOS::SMBIOSBaseBoardInformation *baseboard = SMBIOS::GetBaseBoardInformation();

        debug("SMBIOS: %p", smbios);
        debug("BIOS: %p", bios);
        debug("System: %p", system);
        debug("Baseboard: %p", baseboard);
        DumpData("SMBIOS ALL DUMP", (void *)(uint64_t)smbios->TableAddress, smbios->TableLength);
        if (smbios)
            DumpData("SMBIOS", smbios, sizeof(SMBIOS::SMBIOSEntryPoint));
        if (bios)
            DumpData("BIOS", bios, sizeof(SMBIOS::SMBIOSBIOSInformation));
        if (system)
            DumpData("System", system, sizeof(SMBIOS::SMBIOSSystemInformation));
        if (baseboard)
            DumpData("Baseboard", baseboard, sizeof(SMBIOS::SMBIOSBaseBoardInformation));

        if (smbios)
            KPrint("SMBIOS: \eCCCCCCString:\e8888FF%.4s \eCCCCCCVersion (Major Minor):\e8888FF%d %d \eCCCCCCTable:\e8888FF%#x \eCCCCCCLength:\e8888FF%d",
                   smbios->EntryPointString, smbios->MajorVersion, smbios->MinorVersion,
                   smbios->TableAddress, smbios->TableLength);
        else
            KPrint("SMBIOS: \e8888FFSMBIOS found but not supported?");

        if (bios)
        {
            const char *BIOSVendor = bios->GetString(bios->Vendor);
            const char *BIOSVersion = bios->GetString(bios->Version);
            const char *BIOSReleaseDate = bios->GetString(bios->ReleaseDate);
            debug("%d %d %d", bios->Vendor, bios->Version, bios->ReleaseDate);
            KPrint("BIOS: \e8888FF%s \eCCCCCC/ \e8888FF%s \eCCCCCC/ \e8888FF%s",
                   BIOSVendor, BIOSVersion, BIOSReleaseDate);
        }

        if (system)
        {
            const char *SystemManufacturer = system->GetString(system->Manufacturer);
            const char *SystemProductName = system->GetString(system->ProductName);
            const char *SystemVersion = system->GetString(system->Version);
            const char *SystemSerialNumber = system->GetString(system->SerialNumber);
            const char *SystemSKU = system->GetString(system->SKU);
            const char *SystemFamily = system->GetString(system->Family);
            debug("%d %d %d %d %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c %d %d", system->Manufacturer, system->ProductName, system->Version,
                  system->SerialNumber,
                  system->UUID[0], system->UUID[1], system->UUID[2], system->UUID[3],
                  system->UUID[4], system->UUID[5], system->UUID[6], system->UUID[7],
                  system->UUID[8], system->UUID[9], system->UUID[10], system->UUID[11],
                  system->UUID[12], system->UUID[13], system->UUID[14], system->UUID[15],
                  system->SKU, system->Family);
            KPrint("System: \e8888FF%s \eCCCCCC/ \e8888FF%s \eCCCCCC/ \e8888FF%s \eCCCCCC/ \e8888FF%s \eCCCCCC/ \e8888FF%s \eCCCCCC/ \e8888FF%s",
                   SystemManufacturer, SystemProductName, SystemVersion, SystemSerialNumber, SystemSKU, SystemFamily);
        }

        if (baseboard)
        {
            const char *Manufacturer = baseboard->GetString(baseboard->Manufacturer);
            const char *Product = baseboard->GetString(baseboard->Product);
            const char *Version = baseboard->GetString(baseboard->Version);
            const char *SerialNumber = baseboard->GetString(baseboard->SerialNumber);
            debug("%d %d %d %d", baseboard->Manufacturer, baseboard->Product, baseboard->Version, baseboard->SerialNumber);
            KPrint("Baseboard: \eCCCCCC\e8888FF%s \eCCCCCC/ \e8888FF%s \eCCCCCC/ \e8888FF%s \eCCCCCC/ \e8888FF%s",
                   Manufacturer, Product, Version, SerialNumber);
        }
    }
    else
        KPrint("SMBIOS: \eFF0000Not Found");

    TaskManager = new Tasking::Task((Tasking::IP)KernelMainThread);
    KPrint("\e058C19################################");
    CPU::Halt(true);
}

EXTERNC void TaskingPanic()
{
    if (TaskManager)
        TaskManager->Panic();
}
