/*
   This file is part of Fennix Kernel.

   Fennix Kernel is free software: you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of
   the License, or (at your option) any later version.

   Fennix Kernel is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Fennix Kernel. If not, see <https://www.gnu.org/licenses/>.
*/

#include "kernel.h"

#include <filesystem/mounts.hpp>
#include <filesystem/ustar.hpp>
#include <memory.hpp>
#include <convert.h>
#include <ints.hpp>
#include <printf.h>
#include <lock.hpp>
#include <uart.hpp>
#include <debug.h>
#include <smp.hpp>
#include <cargs.h>
#include <io.h>

#include "Core/smbios.hpp"
#include "Tests/t.h"

bool DebuggerIsAttached = false;

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
 * - [x] Optimize SMP.
 * - [ ] Support IPv6.
 * - [ ] Endianess of the network stack (currently: [HOST](LSB)<=>[NETWORK](MSB)). Not sure if this is a standard or not.
 * - [ ] Support 32-bit applications (ELF, PE, etc).
 * - [ ] Do not map the entire memory. Map only the needed memory address at allocation time.
 * - [ ] Implementation of logging (beside serial) with log rotation.
 * - [ ] Implement a better task manager. (replace struct P/TCB with classes)
 * - [?] Rewrite virtual file system. (it's very bad, I don't know how I wrote it this bad)
 * - [ ] Colors in crash screen are not following the kernel color scheme.
 * - [x] Find a way to add intrinsics.
 * - [ ] Rework PSF1 font loader.
 * - [x] The cleanup should be done by a thread (tasking). This is done to avoid a deadlock.
 * - [ ] Implement a better Display::SetBrightness() function.
 * - [ ] Fix memcpy, memset and memcmp functions (they are not working properly with SIMD).
 * - [ ] Fully support i386.
 * - [ ] Support Aarch64.
 * - [ ] SMP trampoline shouldn't be hardcoded at 0x2000.
 * - [ ] Rework the stack guard.
 * - [x] Mutex implementation.
 * - [ ] Update SMBIOS functions to support newer versions and actually use it.
 * - [ ] COW (Copy On Write) for the virtual memory. (https://en.wikipedia.org/wiki/Copy-on-write)
 * - [ ] Bootstrap should have a separate bss section + PHDR.
 * - [ ] Reimplement the driver conflict detection.
 * - [ ] Elf loader shouldn't create a full copy of the elf binary. Copy only the needed sections.
 * - [ ] Use NX-bit.
 *
 * ISSUES:
 * - [x] Kernel stack is smashed when an interrupt occurs. (this bug it occurs when an interrupt like IRQ1 or IRQ12 occurs)
 * - [x] After setting the new stack pointer, the kernel crashes with an invalid opcode.
 * - [?] Somewhere in the kernel, the memory is wrongly freed or memcpy/memset.
 * - [ ] GlobalDescriptorTable::SetKernelStack() is not working properly.
 * - [ ] Sometimes while the kernel is inside BeforeShutdown(), we end up in a deadlock.
 * - [ ] CPU usage is not working properly.
 * - [x] fork() syscall is not working.
 *
 * CREDITS AND REFERENCES:
 * - General:
 *    https://wiki.osdev.org/Main_Page
 *    https://gcc.gnu.org/onlinedocs/gcc/x86-Built-in-Functions.html#x86-Built-in-Functions
 *    https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#Common-Function-Attributes
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
 * - PCI:
 *    https://wiki.osdev.org/PCI
 *    https://en.wikipedia.org/wiki/PCI_configuration_space
 *
 * - Audio:
 *    https://trac.ffmpeg.org/wiki/audio%20types
 *    https://wiki.osdev.org/AC97
 *    https://github.com/LemonOSProject/LemonOS
 *    https://inst.eecs.berkeley.edu//~cs150/Documents/ac97_r23.pdf
 *
 * - Intrinsics:
 *    https://learn.microsoft.com/en-us/cpp/intrinsics/x86-intrinsics-list
 *    https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html
 *
 * - CPUID lists:
 *    https://www.amd.com/system/files/TechDocs/40332.pdf
 *    https://www.scss.tcd.ie/~jones/CS4021/processor-identification-cpuid-instruction-note.pdf
 *
 * - SMBIOS:
 *    https://www.dmtf.org/dsp/DSP0134
 *    https://www.dmtf.org/sites/default/files/standards/documents/DSP0134_3.6.0.pdf
 *
 * - UMIP, SMAP and SMEP:
 *    https://en.wikipedia.org/wiki/Control_register
 *    https://web.archive.org/web/20160312223150/http://ncsi.com/nsatc11/presentations/wednesday/emerging_technologies/fischer.pdf
 *    https://en.wikipedia.org/wiki/Supervisor_Mode_Access_Prevention
 *
 * - Atomic operations:
 *    https://en.cppreference.com/w/cpp/atomic/atomic
 *
 * - ELF:
 *    https://www.sco.com/developers/gabi/latest/ch4.eheader.html
 *    https://refspecs.linuxfoundation.org/elf/elf.pdf
 *    https://docs.oracle.com/cd/E19683-01/817-3677/chapter6-42444/index.html
 *    https://docs.oracle.com/cd/E19683-01/816-1386/chapter6-83432/index.html
 *    https://www.youtube.com/watch?v=nC1U1LJQL8o
 *    https://stevens.netmeister.org/631/elf.html
 *    https://github.com/torvalds/linux/blob/master/include/uapi/linux/elf.h
 *
 * - C++ ABI:
 *    https://github.com/gcc-mirror/gcc/tree/master/libstdc%2B%2B-v3
 *    https://itanium-cxx-abi.github.io/cxx-abi/abi.html
 *
 * - Keyboard:
 *    https://www.win.tue.nl/~aeb/linux/kbd/scancodes-11.html
 *    https://wiki.osdev.org/PS/2_Keyboard
 *
 */

#ifdef a64
#if UINTPTR_MAX != UINT64_MAX
#error "uintptr_t is not 64-bit!"
#endif // UINTPTR_MAX != UINT64_MAX
#endif // a64

#ifdef a32
#if UINTPTR_MAX != UINT32_MAX
#error "uintptr_t is not 32-bit!"
#endif // UINTPTR_MAX != UINT32_MAX
#endif // a32

#ifdef aa64
#if UINTPTR_MAX != UINT64_MAX
#error "uintptr_t is not 64-bit!"
#endif // UINTPTR_MAX != UINT64_MAX
#endif // aa64

NewLock(KernelLock);

#include <intrin.hpp>

using VirtualFileSystem::Node;
using VirtualFileSystem::NodeFlags;

__aligned(16) BootInfo bInfo{};
Video::Display *Display = nullptr;
SymbolResolver::Symbols *KernelSymbolTable = nullptr;
Power::Power *PowerManager = nullptr;
PCI::PCI *PCIManager = nullptr;
Tasking::Task *TaskManager = nullptr;
Time::time *TimeManager = nullptr;
VirtualFileSystem::Virtual *vfs = nullptr;

KernelConfig Config = {
	.AllocatorType = Memory::MemoryAllocatorType::liballoc11,
	.SchedulerType = Multi,
	.DriverDirectory = {'/', 'm', 'o', 'd', 'u', 'l', 'e', 's', '\0'},
	.InitPath = {'/', 'b', 'i', 'n', '/', 'i', 'n', 'i', 't', '\0'},
	.UseLinuxSyscalls = false,
	.InterruptsOnCrash = true,
	.Cores = 0,
	.IOAPICInterruptCore = 0,
	.UnlockDeadLock = false,
	.SIMD = false,
	.BootAnimation = false,
};

extern bool EnableProfiler;

// For the Display class. Printing on first buffer as default.
int PutCharBufferIndex = 0;
EXTERNC void putchar(char c)
{
	if (Display)
		Display->Print(c, PutCharBufferIndex);
	else
		UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM1).Write(c);
}

EXTERNC void KPrint(const char *Format, ...)
{
	SmartLock(KernelLock);

	if (TimeManager)
	{
		uint64_t Nanoseconds = TimeManager->GetNanosecondsSinceClassCreation();
		if (Nanoseconds != 0)
		{
#if defined(a64)
			printf("\eCCCCCC[\e00AEFF%lu.%07lu\eCCCCCC] ",
				   Nanoseconds / 10000000, Nanoseconds % 10000000);
#elif defined(a32)
			printf("\eCCCCCC[\e00AEFF%llu.%07llu\eCCCCCC] ",
				   Nanoseconds / 10000000, Nanoseconds % 10000000);
#elif defined(aa64)
			printf("\eCCCCCC[\e00AEFF%lu.%07lu\eCCCCCC] ",
				   Nanoseconds / 10000000, Nanoseconds % 10000000);
#endif
		}
	}

	va_list args;
	va_start(args, Format);
	vprintf(Format, args);
	va_end(args);

	printf("\eCCCCCC\n");
	if (!Config.BootAnimation && Display)
		Display->SetBuffer(0);
}

EXTERNC NIF void Main()
{
	Display = new Video::Display(bInfo.Framebuffer[0]);

	KPrint("%s - %s [\e058C19%s\eFFFFFF]",
		   KERNEL_NAME, KERNEL_VERSION, GIT_COMMIT_SHORT);
	KPrint("CPU: \e058C19%s \e8822AA%s \e8888FF%s",
		   CPU::Hypervisor(), CPU::Vendor(), CPU::Name());

	debug("CPU: %s %s %s",
		  CPU::Hypervisor(), CPU::Vendor(), CPU::Name());

	if (DebuggerIsAttached)
		KPrint("\eFFA500Kernel debugger detected.");

#if defined(a86) && defined(DEBUG)
	uint8_t lpt1 = inb(0x378);
	uint8_t lpt2 = inb(0x278);
	uint8_t lpt3 = inb(0x3BC);

	uint8_t com1 = inb(0x3F8);
	uint8_t com2 = inb(0x2F8);
	uint8_t com3 = inb(0x3E8);
	uint8_t com4 = inb(0x2E8);

	if (lpt1 != 0xFF)
		KPrint("LPT1 is present.");

	if (lpt2 != 0xFF)
		KPrint("LPT2 is present.");

	if (lpt3 != 0xFF)
		KPrint("LPT3 is present.");

	if (com1 != 0xFF)
		KPrint("COM1 is present.");

	if (com2 != 0xFF)
		KPrint("COM2 is present.");

	if (com3 != 0xFF)
		KPrint("COM3 is present.");

	if (com4 != 0xFF)
		KPrint("COM4 is present.");
#endif

	/**************************************************************************************/

	KPrint("Initializing GDT and IDT");
	Interrupts::Initialize(0);

	KPrint("Loading Kernel Symbols");
	KernelSymbolTable = new SymbolResolver::Symbols((uintptr_t)bInfo.Kernel.FileBase);

	if (!KernelSymbolTable->SymTableExists)
		KernelSymbolTable->AddSymbolInfoFromGRUB(bInfo.Kernel.Symbols.Num,
												 bInfo.Kernel.Symbols.EntSize,
												 bInfo.Kernel.Symbols.Shndx,
												 bInfo.Kernel.Symbols.Sections);

	KPrint("Reading Kernel Parameters");
	ParseConfig((char *)bInfo.Kernel.CommandLine, &Config);

	if (Config.BootAnimation)
	{
		Display->CreateBuffer(0, 0, 1);

		Display->SetDoNotScroll(true, 1);
		Video::ScreenBuffer *buf = Display->GetBuffer(1);
		Video::FontInfo fi = Display->GetCurrentFont()->GetInfo();
		Display->SetBufferCursor(1, 0, buf->Height - fi.Height);
		PutCharBufferIndex = 1;
		printf("Fennix Operating System - %s [\e058C19%s\eFFFFFF]\n",
			   KERNEL_VERSION, GIT_COMMIT_SHORT);
		Display->SetBuffer(1);
		PutCharBufferIndex = 0;
	}

	KPrint("Initializing CPU Features");
	CPU::InitializeFeatures(0);

	KPrint("Initializing Power Manager");
	PowerManager = new Power::Power;

	KPrint("Enabling Interrupts on Bootstrap Processor");
	Interrupts::Enable(0);

#if defined(a86)
	PowerManager->InitDSDT();
#elif defined(aa64)
#endif

	KPrint("Initializing Timers");
	TimeManager = new Time::time;
	TimeManager->FindTimers(PowerManager->GetACPI());

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

	for (size_t i = 0; i < MAX_MODULES; i++)
	{
		if (!bInfo.Modules[i].Address)
			continue;

		if (strcmp(bInfo.Modules[i].CommandLine, "initrd") == 0)
		{
			debug("Found initrd at %p", bInfo.Modules[i].Address);
			static char initrd = 0;
			if (!initrd++)
			{
				uintptr_t initrdAddress = (uintptr_t)bInfo.Modules[i].Address;
				VirtualFileSystem::USTAR *ustar = new VirtualFileSystem::USTAR;
				ustar->ReadArchive(initrdAddress, vfs);
			}
		}
	}

	if (vfs->GetRootNode()->Children.size() == 0)
	{
		VirtualFileSystem::FileSystemOperations null_op = {
			.Name = "null",
		};

		vfs->CreateRoot("/", &null_op);
	}

	if (!vfs->PathExists("/dev"))
		DevFS = vfs->Create("/dev", NodeFlags::DIRECTORY);
	else
	{
		RefNode *dev = vfs->Open("/dev");
		if (dev->GetNode()->Flags != NodeFlags::DIRECTORY)
		{
			KPrint("\eE85230/dev is not a directory! Halting...");
			CPU::Stop();
		}
		DevFS = dev->GetNode();
		delete dev;
	}

	if (!vfs->PathExists("/mnt"))
		MntFS = vfs->Create("/mnt", NodeFlags::DIRECTORY);
	else
	{
		RefNode *mnt = vfs->Open("/mnt");
		if (mnt->GetNode()->Flags != NodeFlags::DIRECTORY)
		{
			KPrint("\eE85230/mnt is not a directory! Halting...");
			CPU::Stop();
		}
		MntFS = mnt->GetNode();
		delete mnt;
	}

	if (!vfs->PathExists("/proc"))
		ProcFS = vfs->Create("/proc", NodeFlags::DIRECTORY);
	else
	{
		RefNode *proc = vfs->Open("/proc", nullptr);
		if (proc->GetNode()->Flags != NodeFlags::DIRECTORY)
		{
			KPrint("\eE85230/proc is not a directory! Halting...");
			CPU::Stop();
		}
		ProcFS = proc->GetNode();
		delete proc;
	}

	if (!vfs->PathExists("/var"))
		VarLogFS = vfs->Create("/var", NodeFlags::DIRECTORY);
	else
	{
		RefNode *var = vfs->Open("/var", nullptr);
		if (var->GetNode()->Flags != NodeFlags::DIRECTORY)
		{
			KPrint("\eE85230/var is not a directory! Halting...");
			CPU::Stop();
		}
		VarLogFS = var->GetNode();
		delete var;

		if (!vfs->PathExists("/var/log"))
			VarLogFS = vfs->Create("/var/log", NodeFlags::DIRECTORY);
		else
		{
			RefNode *var_log = vfs->Open("/var/log", nullptr);
			if (var_log->GetNode()->Flags != NodeFlags::DIRECTORY)
			{
				KPrint("\eE85230/var/log is not a directory! Halting...");
				CPU::Stop();
			}
			VarLogFS = var_log->GetNode();
			delete var_log;
		}
	}

	Init_Null(vfs);
	Init_Random(vfs);
	Init_Teletype(vfs);
	Init_Zero(vfs);

	KPrint("\e058C19################################");
	TaskManager = new Tasking::Task(Tasking::IP(KernelMainThread));
	CPU::Halt(true);
}

typedef void (*CallPtr)(void);
extern CallPtr __init_array_start[0], __init_array_end[0];
extern CallPtr __fini_array_start[0], __fini_array_end[0];

EXTERNC __no_stack_protector NIF void Entry(BootInfo *Info)
{
	trace("Hello, World!");

	if (strcmp(CPU::Hypervisor(), x86_CPUID_VENDOR_TCG) == 0)
	{
		debug("\n\n----------------------------------------\nDEBUGGER DETECTED\n----------------------------------------\n\n");
		DebuggerIsAttached = true;
	}

	memcpy(&bInfo, Info, sizeof(BootInfo));
	debug("BootInfo structure is at %p", &bInfo);

	// https://wiki.osdev.org/Calling_Global_Constructors
	trace("There are %d constructors to call", __init_array_end - __init_array_start);
	for (CallPtr *func = __init_array_start; func != __init_array_end; func++)
		(*func)();

	InitializeMemoryManagement();

	void *KernelStackAddress = KernelAllocator.RequestPages(TO_PAGES(STACK_SIZE));
	uintptr_t KernelStack = (uintptr_t)KernelStackAddress + STACK_SIZE - 0x10;
	debug("Kernel stack: %#lx-%#lx", KernelStackAddress, KernelStack);
#if defined(a64)
	asmv("mov %0, %%rsp"
		 :
		 : "r"(KernelStack)
		 : "memory");
	asmv("mov $0, %rbp");
#elif defined(a32)
	asmv("mov %0, %%esp"
		 :
		 : "r"(KernelStack)
		 : "memory");
	asmv("mov $0, %ebp");
#elif defined(aa64)
#warning "Kernel stack is not set!"
#endif

#ifdef DEBUG
	/* I had to do this because KernelAllocator
	 * is a global constructor but we need
	 * memory management to be initialized first.
	 */
	TestMemoryAllocation();
	TestString();
	Test_std();
#endif
	EnableProfiler = true;
	Main();
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
extern "C" void __cxa_finalize(void *);
EXTERNC __no_stack_protector void BeforeShutdown(bool Reboot)
{
	UNUSED(Reboot);
	/* TODO: Announce shutdown */

	trace("\n\n\n#################### SYSTEM SHUTTING DOWN ####################\n\n");

	if (NIManager)
		delete NIManager, NIManager = nullptr;

	if (DiskManager)
		delete DiskManager, DiskManager = nullptr;

	if (DriverManager)
		delete DriverManager, DriverManager = nullptr;

	if (TaskManager && !TaskManager->IsPanic())
	{
		TaskManager->SignalShutdown();
		delete TaskManager, TaskManager = nullptr;
	}

	if (vfs)
		delete vfs, vfs = nullptr;

	if (TimeManager)
		delete TimeManager, TimeManager = nullptr;

	if (Display)
		delete Display, Display = nullptr;
	// PowerManager should not be called

	// https://wiki.osdev.org/Calling_Global_Constructors
	debug("Calling destructors...");
	for (CallPtr *func = __fini_array_start; func != __fini_array_end; func++)
		(*func)();
	__cxa_finalize(nullptr);
	debug("Done.");
}
#pragma GCC diagnostic pop

EXTERNC void TaskingPanic()
{
	if (TaskManager)
		TaskManager->Panic();
}
