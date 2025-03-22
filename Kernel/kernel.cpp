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

#include <filesystem/ustar.hpp>
#include <memory.hpp>
#include <convert.h>
#include <ints.hpp>
#include <printf.h>
#include <lock.hpp>
#include <kcon.hpp>
#include <debug.h>
#include <smp.hpp>
#include <cargs.h>
#include <io.h>

#include "core/smbios.hpp"
#include "tests/t.h"

bool DebuggerIsAttached = false;
extern bool EnableProfiler;
NewLock(KernelLock);

__aligned(16) BootInfo bInfo{};

struct KernelConfig Config = {
	.AllocatorType = Memory::liballoc11,
	.SchedulerType = Multi,
	.DriverDirectory = {'/', 'u', 's', 'r', '/', 'l', 'i', 'b', '/', 'd', 'r', 'i', 'v', 'e', 'r', 's', '\0'},
	.InitPath = {'/', 'b', 'i', 'n', '/', 'i', 'n', 'i', 't', '\0'},
	.UseLinuxSyscalls = false,
	.InterruptsOnCrash = true,
	.Cores = 0,
	.IOAPICInterruptCore = 0,
	.UnlockDeadLock = false,
	.SIMD = false,
	.Quiet = false,
};

Video::Display *Display = nullptr;
SymbolResolver::Symbols *KernelSymbolTable = nullptr;
Power::Power *PowerManager = nullptr;
Time::time *TimeManager = nullptr;
Tasking::Task *TaskManager = nullptr;
PCI::Manager *PCIManager = nullptr;
Driver::Manager *DriverManager = nullptr;
UART::Driver uart;

EXTERNC void putchar(char c)
{
	KernelConsole::VirtualTerminal *vt = KernelConsole::CurrentTerminal.load(std::memory_order_acquire);
	if (vt != nullptr)
		vt->Process(c);
	else
		uart.DebugWrite(c);
}

EXTERNC void _KPrint(const char *Format, va_list Args)
{
	SmartLock(KernelLock);

	if (TimeManager)
	{
		uint64_t Nanoseconds = TimeManager->GetNanosecondsSinceClassCreation();
		if (Nanoseconds != 0)
		{
#if defined(__amd64__)
			printf("\x1b[1;30m[\x1b[1;34m%lu.%07lu\x1b[1;30m]\x1b[0m ",
				   Nanoseconds / 10000000, Nanoseconds % 10000000);
#elif defined(__i386__)
			printf("\x1b[1;30m[\x1b[1;34m%llu.%07llu\x1b[1;30m]\x1b[0m ",
				   Nanoseconds / 10000000, Nanoseconds % 10000000);
#elif defined(__aarch64__)
			printf("\x1b[1;30m[\x1b[1;34m%lu.%07lu\x1b[1;30m]\x1b[0m ",
				   Nanoseconds / 10000000, Nanoseconds % 10000000);
#endif
		}
	}

	vprintf(Format, Args);
	printf("\x1b[0m\n");
	if (!Config.Quiet && Display)
		Display->UpdateBuffer();
}

EXTERNC void KPrint(const char *Format, ...)
{
	va_list args;
	va_start(args, Format);
	_KPrint(Format, args);
	va_end(args);

#ifdef DEBUG
	va_start(args, Format);
	vfctprintf(uart_wrapper, nullptr, "PRINT| ", args);
	va_end(args);

	va_start(args, Format);
	vfctprintf(uart_wrapper, nullptr, Format, args);
	va_end(args);
	uart_wrapper('\n', nullptr);
#endif
}

EXTERNC NIF cold void Main()
{
	Display = new Video::Display(bInfo.Framebuffer[0]);
	KernelConsole::EarlyInit();

	printf("\x1b[H\x1b[2J");
	KPrint("%s - %s [\x1b[32m%s\x1b[0m]",
		   KERNEL_NAME, KERNEL_VERSION, GIT_COMMIT_SHORT);
	KPrint("CPU: \x1b[32m%s \x1b[31m%s \x1b[37m%s",
		   CPU::Hypervisor(), CPU::Vendor(), CPU::Name());

	if (Display->GetFramebufferStruct().BitsPerPixel != 32)
		KPrint("\x1b[1;31mFramebuffer is not 32 bpp. This may cause issues.");

	if (Display->GetWidth < 640 || Display->GetHeight < 480)
	{
		KPrint("\x1b[1;31mMinimum supported resolution is 640x480!");
		KPrint("\x1b[1;31mSome elements may not be displayed correctly.");
	}

	debug("CPU: %s %s %s",
		  CPU::Hypervisor(), CPU::Vendor(), CPU::Name());

	if (DebuggerIsAttached)
		KPrint("Kernel debugger detected.");

#if defined(__amd64__) || defined(__i386__) && defined(DEBUG)
	uint8_t lpt1 = inb(0x378);
	uint8_t lpt2 = inb(0x278);
	uint8_t lpt3 = inb(0x3BC);

	uint8_t com1 = inb(0x3F8);
	uint8_t com2 = inb(0x2F8);
	uint8_t com3 = inb(0x3E8);
	uint8_t com4 = inb(0x2E8);
	uint8_t com5 = inb(0x5F8);
	uint8_t com6 = inb(0x4F8);
	uint8_t com7 = inb(0x5E8);
	uint8_t com8 = inb(0x4E8);

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

	if (com5 != 0xFF)
		KPrint("COM5 is present.");

	if (com6 != 0xFF)
		KPrint("COM6 is present.");

	if (com7 != 0xFF)
		KPrint("COM7 is present.");

	if (com8 != 0xFF)
		KPrint("COM8 is present.");

	KPrint("Display: %dx%d %d bpp R:%d %d G:%d %d B:%d %d",
		   Display->GetFramebufferStruct().Width,
		   Display->GetFramebufferStruct().Height,
		   Display->GetFramebufferStruct().BitsPerPixel,
		   Display->GetFramebufferStruct().RedMaskSize,
		   Display->GetFramebufferStruct().RedMaskShift,
		   Display->GetFramebufferStruct().GreenMaskSize,
		   Display->GetFramebufferStruct().GreenMaskShift,
		   Display->GetFramebufferStruct().BlueMaskSize,
		   Display->GetFramebufferStruct().BlueMaskShift);

	KPrint("%lld MiB / %lld MiB (%lld MiB reserved)",
		   TO_MiB(KernelAllocator.GetUsedMemory()),
		   TO_MiB(KernelAllocator.GetTotalMemory()),
		   TO_MiB(KernelAllocator.GetReservedMemory()));
#endif

	/**************************************************************************************/

	KPrint("Reading Kernel Parameters");
	ParseConfig((char *)bInfo.Kernel.CommandLine, &Config);

	KPrint("Initializing CPU Features");
	CPU::InitializeFeatures(0);

	KPrint("Initializing GDT and IDT");
	Interrupts::Initialize(0);

	KPrint("Loading Kernel Symbols");
	KernelSymbolTable =
		new SymbolResolver::Symbols((uintptr_t)bInfo.Kernel.FileBase);

	if (!KernelSymbolTable->SymTableExists)
		KernelSymbolTable->AddSymbolInfoFromGRUB(bInfo.Kernel.Symbols.Num,
												 bInfo.Kernel.Symbols.EntSize,
												 bInfo.Kernel.Symbols.Shndx,
												 bInfo.Kernel.Symbols.Sections);

	KPrint("Initializing Power Manager");
	PowerManager = new Power::Power;

	KPrint("Enabling Interrupts on Bootstrap Processor");
	Interrupts::Enable(0);

#if defined(__amd64__) || defined(__i386__)
	PowerManager->InitDSDT();
#elif defined(__aarch64__)
#endif

	KPrint("Initializing Timers");
	TimeManager = new Time::time;
	TimeManager->FindTimers(PowerManager->GetACPI());

	KPrint("Initializing PCI Manager");
	PCIManager = new PCI::Manager;

	KPrint("Initializing Bootstrap Processor Timer");
	Interrupts::InitializeTimer(0);

	KPrint("Initializing SMP");
	SMP::Initialize(PowerManager->GetMADT());

	KPrint("Initializing Filesystem");
	KernelVFS();

	KPrint("\x1b[1;32m################################");
	TaskManager = new Tasking::Task(Tasking::IP(KernelMainThread));
	TaskManager->StartScheduler();
	CPU::Halt(true);
}

typedef void (*CallPtr)(void);
extern CallPtr __init_array_start[0], __init_array_end[0];
extern CallPtr __fini_array_start[0], __fini_array_end[0];

EXTERNC __no_stack_protector NIF cold void Entry(BootInfo *Info)
{
	trace("Hello, World!");

	if (strcmp(CPU::Hypervisor(), x86_CPUID_VENDOR_TCG) == 0)
	{
		info("\n\n----------------------------------------\nDEBUGGER DETECTED\n----------------------------------------\n\n");
		DebuggerIsAttached = true;
	}

	memcpy(&bInfo, Info, sizeof(BootInfo));
	debug("BootInfo structure is at %p", &bInfo);

	// https://wiki.osdev.org/Calling_Global_Constructors
	trace("There are %d constructors to call",
		  __init_array_end - __init_array_start);
	for (CallPtr *fct = __init_array_start; fct != __init_array_end; fct++)
		(*fct)();

#if defined(__amd64__) || defined(__i386__)
	if (!bInfo.SMBIOSPtr)
	{
		trace("SMBIOS was not provided by the bootloader. Trying to find it manually.");
		for (uintptr_t i = 0xF0000; i < 0x100000; i += 16)
		{
			if (memcmp((void *)i, "_SM_", 4) == 0 ||
				memcmp((void *)i, "_SM3_", 5) == 0)
			{
				bInfo.SMBIOSPtr = (void *)i;
				trace("Found SMBIOS at %#lx", i);
			}
		}
	}

	if (!bInfo.RSDP)
	{
		trace("RSDP was not provided by the bootloader. Trying to find it manually.");
		/* FIXME: Not always shifting by 4 will work. */
		uintptr_t EBDABase = (uintptr_t)mminw((void *)0x40E) << 4;

		for (uintptr_t ptr = EBDABase;
			 ptr < 0x100000; /* 1MB */
			 ptr += 16)
		{
			if (unlikely(ptr == EBDABase + 0x400))
			{
				trace("EBDA is full. Trying to find RSDP in the BIOS area.");
				break;
			}

			BootInfo::RSDPInfo *rsdp = (BootInfo::RSDPInfo *)ptr;
			if (memcmp(rsdp->Signature, "RSD PTR ", 8) == 0)
			{
				bInfo.RSDP = (BootInfo::RSDPInfo *)rsdp;
				trace("Found RSDP at %#lx", rsdp);
			}
		}

		for (uintptr_t ptr = 0xE0000;
			 ptr < 0x100000; /* 1MB */
			 ptr += 16)
		{
			BootInfo::RSDPInfo *rsdp = (BootInfo::RSDPInfo *)ptr;
			if (memcmp(rsdp->Signature, "RSD PTR ", 8) == 0)
			{
				bInfo.RSDP = (BootInfo::RSDPInfo *)rsdp;
				trace("Found RSDP at %#lx", rsdp);
			}
		}
	}
#endif

	InitializeMemoryManagement();

	void *KernelStackAddress = KernelAllocator.RequestPages(TO_PAGES(STACK_SIZE));
	// void *KernelStackAddress = StackManager.Allocate(STACK_SIZE); /* FIXME: This breaks stl tests, how? */
	uintptr_t KernelStack = (uintptr_t)KernelStackAddress + STACK_SIZE - 0x10;
	debug("Kernel stack: %#lx-%#lx", KernelStackAddress, KernelStack);
#if defined(__amd64__)
	asmv("mov %0, %%rsp" : : "r"(KernelStack) : "memory");
	asmv("mov $0, %rbp");
#elif defined(__i386__)
	asmv("mov %0, %%esp" : : "r"(KernelStack) : "memory");
	asmv("mov $0, %ebp");
#else
#warning "Kernel stack is not set!"
	UNUSED(KernelStack);
#endif

#ifdef DEBUG
	/* I had to do this because KernelAllocator
	 * is a global constructor but we need
	 * memory management to be initialized first.
	 */
	TestMemoryAllocation();
#if defined(__amd64__)
	Test_stl();
#else
#warning "FIXME: Test_stl() is not implemented for other architectures"
#endif
#endif // DEBUG
	EnableProfiler = true;
	Main();
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
extern "C" void __cxa_finalize(void *);
EXTERNC __no_stack_protector void BeforeShutdown(bool Reboot)
{
	/* TODO: Announce shutdown */

	trace("\n\n\n#################### SYSTEM SHUTTING DOWN ####################\n\n");

	KPrint("%s...", Reboot ? "Rebooting" : "Shutting down");

	KPrint("Stopping network interfaces");

	KPrint("Unloading all drivers");
	if (DriverManager)
		DriverManager->UnloadAllDrivers();

	KPrint("Stopping scheduling");
	if (TaskManager && !TaskManager->IsPanic())
	{
		TaskManager->SignalShutdown();
		delete TaskManager, TaskManager = nullptr;
	}

	KPrint("Unloading filesystems");
	if (fs)
		delete fs, fs = nullptr;

	KPrint("Stopping timers");
	if (TimeManager)
		delete TimeManager, TimeManager = nullptr;

	// PowerManager should not be called

	// https://wiki.osdev.org/Calling_Global_Constructors
	KPrint("Calling destructors");
	for (CallPtr *fct = __fini_array_start; fct != __fini_array_end; fct++)
		(*fct)();
	__cxa_finalize(nullptr);
	debug("Done.");
}
#pragma GCC diagnostic pop
