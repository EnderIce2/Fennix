#include "kernel.h"

#include <boot/protocols/multiboot2.h>
#include <filesystem/ustar.hpp>
#include <interrupts.hpp>
#include <memory.hpp>
#include <convert.h>
#include <printf.h>
#include <lock.hpp>
#include <debug.h>
#include <smp.hpp>
#include <cargs.h>
#include <io.h>

#include "Core/smbios.hpp"
#include "Tests/t.h"

/**
 * Fennix Kernel
 * -------------
 * This is the main kernel file. It contains the main function and the kernel entry point.
 *
 * LOADING PROCEDURE:
 * [BOOT] -> [Bootloader] -> [Boot Info Parser] -> Entry() -> Main() -> KernelMainThread()
 * - Bootloader
 * - Entry() is the first function to be called by the boot info parser function after getting the boot info from the bootloader.
 * - Main() is the first function to be called by Entry().
 * - KernelMainThread() is the first function to be called by the task manager.
 *
 * TODO:
 * - [ ] Optimize SMP.
 * - [ ] Support IPv6.
 * - [ ] Endianess of the network stack (currently: [HOST](LSB)<=>[NETWORK](MSB)). Not sure if this is a standard or not.
 * - [ ] Support 32-bit applications (ELF, PE, etc).
 * - [ ] Do not map the entire memory. Map only the needed memory address at allocation time.
 * - [ ] Implementation of logging (beside serial) with log rotation.
 * - [ ] Implement a better task manager. (replace struct P/TCB with classes)
 * - [?] Rewrite virtual file system. (it's very bad, I don't know how I wrote it this bad)
 * - [ ] Colors in crash screen are not following the kernel color scheme.
 *
 * BUGS:
 * - [ ] Kernel crashes when receiving interrupts for drivers only if the system has one core and the tasking is running.
 *       - This bug is available only when the VMware mouse driver is loaded and receives an interrupt.
 * - [ ] After setting the new stack pointer, the kernel crashes with an invalid opcode.
 *
 * CREDITS AND REFERENCES:
 * - General:
 *    https://wiki.osdev.org/Main_Page
 *
 * - Font:
 *    http://www.fial.com/~scott/tamsyn-font/
 *
 * - CPU XCR0 structure:
 *    https://wiki.osdev.org/CPU_Registers_x86#XCR0
 *
 * - CPUID 0x7:
 *    https://en.wikipedia.org/wiki/CPUID
 *
 * - Network:
 *    https://web.archive.org/web/20051210132103/http://users.pcnet.ro/dmoroian/beej/Beej.html
 *    https://web.archive.org/web/20060229214053/http://www.cs.rutgers.edu/~pxk/417/notes/sockets/udp.html
 *    https://en.wikipedia.org/wiki/EtherType
 *    https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/6/html/performance_tuning_guide/s-network-packet-reception
 *    https://linux-kernel-labs.github.io/refs/heads/master/labs/networking.html
 *    https://github.com/smoltcp-rs/smoltcp
 *    https://www.ciscopress.com/articles/article.asp?p=3089352&seqNum=5
 *    https://www.cs.unh.edu/cnrg/people/gherrin/linux-net.html
 *    https://en.wikipedia.org/wiki/List_of_IP_protocol_numbers
 *    https://github.com/TheUltimateFoxOS/horizon
 *    https://en.wikipedia.org/wiki/Address_Resolution_Protocol
 *    https://en.cppreference.com/w/cpp/language/operators
 *    https://en.wikipedia.org/wiki/Dynamic_Host_Configuration_Protocol
 *    https://www.cs.usfca.edu/~cruse/cs326f04/RTL8139D_DataSheet.pdf
 *    https://www.javatpoint.com/arp-packet-format
 *    https://www.cs.usfca.edu/~cruse/cs326f04/RTL8139_ProgrammersGuide.pdf
 *    http://realtek.info/pdf/rtl8139cp.pdf
 *    https://en.wikipedia.org/wiki/IPv4
 *    https://www.iana.org/assignments/icmp-parameters/icmp-parameters.xhtml
 *
 * - Loading ELF shared libraries and dynamic linking:
 *    https://www.akkadia.org/drepper/dsohowto.pdf
 *    https://wiki.osdev.org/Dynamic_Linker
 *    https://github.com/tyler569/nightingale
 *    https://www.technovelty.org/linux/plt-and-got-the-key-to-code-sharing-and-dynamic-libraries.html
 *    https://www.youtube.com/watch?v=kUk5pw4w0h4
 *    https://docs.oracle.com/cd/E19683-01/817-3677/chapter6-42444/index.html
 *    https://ir0nstone.gitbook.io/notes/types/stack/aslr/plt_and_got
 *
 * - IPC:
 *    https://docs.oracle.com/cd/E19048-01/chorus5/806-6897/architecture-103/index.html
 *    https://www.scaler.com/topics/operating-system/inter-process-communication-in-os/
 *    https://en.wikipedia.org/wiki/Inter-process_communication
 *    https://www.geeksforgeeks.org/inter-process-communication-ipc/
 *
 */

#ifdef __amd64__
#if UINTPTR_MAX != UINT64_MAX
#error "uintptr_t is not 64-bit!"
#endif // UINTPTR_MAX != UINT64_MAX
#endif // __amd64__

#ifdef __i386__
#if UINTPTR_MAX != UINT32_MAX
#error "uintptr_t is not 32-bit!"
#endif // UINTPTR_MAX != UINT32_MAX
#endif // __i386__

#ifdef __aarch64__
#if UINTPTR_MAX != UINT64_MAX
#error "uintptr_t is not 64-bit!"
#endif // UINTPTR_MAX != UINT64_MAX
#endif // __aarch64__

NewLock(KernelLock);

using VirtualFileSystem::File;
using VirtualFileSystem::FileStatus;
using VirtualFileSystem::Node;
using VirtualFileSystem::NodeFlags;

BootInfo *bInfo = nullptr;
Video::Display *Display = nullptr;
SymbolResolver::Symbols *KernelSymbolTable = nullptr;
Power::Power *PowerManager = nullptr;
PCI::PCI *PCIManager = nullptr;
Tasking::Task *TaskManager = nullptr;
Time::time *TimeManager = nullptr;
VirtualFileSystem::Virtual *vfs = nullptr;

KernelConfig Config;
Time::Clock BootClock;

extern bool EnableProfiler;

// For the Display class. Printing on first buffer as default.
EXTERNC void putchar(char c) { Display->Print(c, 0); }

EXTERNC void KPrint(const char *Format, ...)
{
    SmartLock(KernelLock);
    Time::Clock tm = Time::ReadClock();
    printf("\eCCCCCC[\e00AEFF%02d:%02d:%02d\eCCCCCC] ", tm.Hour, tm.Minute, tm.Second);
    va_list args;
    va_start(args, Format);
    vprintf(Format, args);
    va_end(args);
    putchar('\n');
    Display->SetBuffer(0);
}

EXTERNC __no_instrument_function void Main(BootInfo *Info)
{
    BootClock = Time::ReadClock();
    bInfo = (BootInfo *)KernelAllocator.RequestPages(TO_PAGES(sizeof(BootInfo)));
    memcpy(bInfo, Info, sizeof(BootInfo));
    debug("BootInfo structure is at %p", bInfo);
    Display = new Video::Display(bInfo->Framebuffer[0]);
    printf("\eFFFFFF%s - %s [\e058C19%s\eFFFFFF]\n", KERNEL_NAME, KERNEL_VERSION, GIT_COMMIT_SHORT);
    /**************************************************************************************/
    KPrint("Time: \e8888FF%02d:%02d:%02d %02d/%02d/%02d UTC",
           BootClock.Hour, BootClock.Minute, BootClock.Second,
           BootClock.Day, BootClock.Month, BootClock.Year);
    KPrint("CPU: \e8822AA%s \e8888FF%s (\e058C19%s\e8888FF)", CPU::Vendor(), CPU::Name(), CPU::Hypervisor());
    KPrint("Initializing GDT and IDT");
    Interrupts::Initialize(0);
    KPrint("Initializing CPU Features");
    CPU::InitializeFeatures(0);
    KPrint("Loading Kernel Symbols");
    KernelSymbolTable = new SymbolResolver::Symbols((uintptr_t)Info->Kernel.FileBase);
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

    KPrint("Initializing Filesystem...");
    vfs = new VirtualFileSystem::Virtual;
    new VirtualFileSystem::USTAR((uintptr_t)bInfo->Modules[0].Address, vfs); // TODO: Detect initrd

    if (!vfs->PathExists("/system"))
        vfs->Create("/system", NodeFlags::DIRECTORY);

    if (!vfs->PathExists("/system/dev"))
        DevFS = vfs->Create("/system/dev", NodeFlags::DIRECTORY);
    else
    {
        shared_ptr<File> dev = vfs->Open("/system/dev");
        if (dev->node->Flags != NodeFlags::DIRECTORY)
        {
            KPrint("\eE85230/system/dev is not a directory! Halting...");
            CPU::Stop();
        }
        vfs->Close(dev);
        DevFS = dev->node;
    }

    if (!vfs->PathExists("/system/mnt"))
        MntFS = vfs->Create("/system/mnt", NodeFlags::DIRECTORY);
    else
    {
        shared_ptr<File> mnt = vfs->Open("/system/mnt");
        if (mnt->node->Flags != NodeFlags::DIRECTORY)
        {
            KPrint("\eE85230/system/mnt is not a directory! Halting...");
            CPU::Stop();
        }
        vfs->Close(mnt);
        MntFS = mnt->node;
    }

    if (!vfs->PathExists("/system/proc"))
        ProcFS = vfs->Create("/system/proc", NodeFlags::DIRECTORY);
    else
    {
        shared_ptr<File> proc = vfs->Open("/system/proc", nullptr);
        if (proc->node->Flags != NodeFlags::DIRECTORY)
        {
            KPrint("\eE85230/system/proc is not a directory! Halting...");
            CPU::Stop();
        }
        vfs->Close(proc);
        ProcFS = proc->node;
    }

    KPrint("\e058C19################################");
    TaskManager = new Tasking::Task((Tasking::IP)KernelMainThread);
    CPU::Halt(true);
}

typedef void (*CallPtr)(void);
extern CallPtr __init_array_start[0], __init_array_end[0];
extern CallPtr __fini_array_start[0], __fini_array_end[0];

EXTERNC __no_stack_protector __no_instrument_function void Entry(BootInfo *Info)
{
    trace("Hello, World!");

    // https://wiki.osdev.org/Calling_Global_Constructors
    for (CallPtr *func = __init_array_start; func != __init_array_end; func++)
        (*func)();

    InitializeMemoryManagement(Info);

    /* I had to do this because KernelAllocator
     * is a global constructor but we need
     * memory management to be initialized first.
     */
#ifdef DEBUG
    // Running tests
    TestString();
    TestMemoryAllocation();
#endif

    EnableProfiler = true;
    Main(Info);
}

#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"

EXTERNC __no_stack_protector __no_instrument_function void BeforeShutdown()
{
    /* TODO: Announce shutdown */

    trace("\n\n\n#################### SYSTEM SHUTTING DOWN ####################\n\n");
    delete NIManager;

    delete DiskManager;
    delete DriverManager;
    TaskManager->SignalShutdown();
    delete TaskManager;
    if (RecoveryScreen)
        delete RecoveryScreen;
    delete vfs;
    delete TimeManager;
    delete Display;
    // PowerManager should not be called

    // https://wiki.osdev.org/Calling_Global_Constructors
    debug("Calling destructors...");
    for (CallPtr *func = __fini_array_start; func != __fini_array_end; func++)
        (*func)();
    debug("Done.");
}

EXTERNC void TaskingPanic()
{
    if (TaskManager)
        TaskManager->Panic();
}
