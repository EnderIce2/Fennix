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

#include "core/smbios.hpp"
#include "tests/t.h"

bool DebuggerIsAttached = false;
extern bool EnableProfiler;
NewLock(KernelLock);

using vfs::Node;
using vfs::NodeType;

__aligned(16) BootInfo bInfo{};

struct KernelConfig Config = {
	.AllocatorType = Memory::liballoc11,
	.SchedulerType = Multi,
	.ModuleDirectory = {'/', 'm', 'o', 'd', 'u', 'l', 'e', 's', '\0'},
	.InitPath = {'/', 'b', 'i', 'n', '/', 'i', 'n', 'i', 't', '\0'},
	.UseLinuxSyscalls = false,
	.InterruptsOnCrash = true,
	.Cores = 0,
	.IOAPICInterruptCore = 0,
	.UnlockDeadLock = false,
	.SIMD = false,
	.BootAnimation = false,
};

Video::Display *Display = nullptr;
SymbolResolver::Symbols *KernelSymbolTable = nullptr;
Power::Power *PowerManager = nullptr;
Time::time *TimeManager = nullptr;
PCI::PCI *PCIManager = nullptr;
vfs::Virtual *fs = nullptr;
vfs::Node *DevFS = nullptr;
vfs::Node *MntFS = nullptr;
vfs::Node *ProcFS = nullptr;
vfs::Node *VarLogFS = nullptr;
Tasking::Task *TaskManager = nullptr;

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

	if (Display->GetFramebufferStruct().BitsPerPixel != 32)
		KPrint("\eFF5500Framebuffer is not 32 bpp. This may cause issues.");

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

	KPrint("Display: %dx%d %d bpp \eFF0000R:%d %d \e00FF00G: %d %d \e0000FFB: %d %d",
		   Display->GetFramebufferStruct().Width,
		   Display->GetFramebufferStruct().Height,
		   Display->GetFramebufferStruct().BitsPerPixel,
		   Display->GetFramebufferStruct().RedMaskSize,
		   Display->GetFramebufferStruct().RedMaskShift,
		   Display->GetFramebufferStruct().GreenMaskSize,
		   Display->GetFramebufferStruct().GreenMaskShift,
		   Display->GetFramebufferStruct().BlueMaskSize,
		   Display->GetFramebufferStruct().BlueMaskShift);
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
			   PCI::Descriptors::GetVendorName(Device.Header->VendorID),
			   PCI::Descriptors::GetDeviceName(Device.Header->VendorID,
											   Device.Header->DeviceID),
			   PCI::Descriptors::DeviceClasses[Device.Header->Class],
			   PCI::Descriptors::GetSubclassName(Device.Header->Class,
												 Device.Header->Subclass),
			   PCI::Descriptors::GetProgIFName(Device.Header->Class,
											   Device.Header->Subclass,
											   Device.Header->ProgIF));
	}

	KPrint("Initializing Bootstrap Processor Timer");
	Interrupts::InitializeTimer(0);

	KPrint("Initializing SMP");
	SMP::Initialize(PowerManager->GetMADT());

	KPrint("Initializing Filesystem...");
	fs = new vfs::Virtual;
	vfs::Node *root = fs->GetRootNode();
	if (root->Children.size() == 0)
		fs->nRoot = new vfs::vfsRoot("/", fs);

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
				vfs::USTAR *ustar = new vfs::USTAR;
				ustar->ReadArchive(initrdAddress, fs);
			}
		}
	}

	if (!fs->PathExists("/dev"))
		DevFS = new vfs::Node(fs->nRoot, "dev", vfs::DIRECTORY);
	else
	{
		vfs::RefNode *dev = fs->Open("/dev");
		if (dev->node->Type != NodeType::DIRECTORY)
		{
			KPrint("\eE85230/dev is not a directory! Halting...");
			CPU::Stop();
		}
		DevFS = dev->node;
		delete dev;
	}

	new vfs::NullDevice();
	new vfs::RandomDevice();
	new vfs::ZeroDevice();

	if (!fs->PathExists("/mnt"))
		MntFS = new vfs::Node(fs->nRoot, "mnt", vfs::DIRECTORY);
	else
	{
		vfs::RefNode *mnt = fs->Open("/mnt");
		if (mnt->node->Type != NodeType::DIRECTORY)
		{
			KPrint("\eE85230/mnt is not a directory! Halting...");
			CPU::Stop();
		}
		MntFS = mnt->node;
		delete mnt;
	}

	if (!fs->PathExists("/proc"))
		ProcFS = new vfs::Node(fs->nRoot, "proc", vfs::DIRECTORY);
	else
	{
		vfs::RefNode *proc = fs->Open("/proc", nullptr);
		if (proc->node->Type != NodeType::DIRECTORY)
		{
			KPrint("\eE85230/proc is not a directory! Halting...");
			CPU::Stop();
		}
		ProcFS = proc->node;
		delete proc;
	}

	if (!fs->PathExists("/var"))
	{
		vfs::Node *var = new vfs::Node(fs->nRoot, "var", vfs::DIRECTORY);
		VarLogFS = new vfs::Node(var, "log", vfs::DIRECTORY);
	}
	else
	{
		vfs::RefNode *var = fs->Open("/var", nullptr);
		if (var->node->Type != NodeType::DIRECTORY)
		{
			KPrint("\eE85230/var is not a directory! Halting...");
			CPU::Stop();
		}
		VarLogFS = var->node;
		delete var;

		if (!fs->PathExists("/var/log"))
			VarLogFS = new vfs::Node(VarLogFS, "log", vfs::DIRECTORY);
		else
		{
			vfs::RefNode *var_log = fs->Open("/var/log", nullptr);
			if (var_log->node->Type != NodeType::DIRECTORY)
			{
				KPrint("\eE85230/var/log is not a directory! Halting...");
				CPU::Stop();
			}
			VarLogFS = var_log->node;
			delete var_log;
		}
	}

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
	trace("There are %d constructors to call",
		  __init_array_end - __init_array_start);
	for (CallPtr *func = __init_array_start; func != __init_array_end; func++)
		(*func)();

#ifdef a86
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
	/* TODO: Announce shutdown */

	trace("\n\n\n#################### SYSTEM SHUTTING DOWN ####################\n\n");

	KPrint("%s...", Reboot ? "Rebooting" : "Shutting down");

	if (NIManager)
		delete NIManager, NIManager = nullptr;

	if (DiskManager)
		delete DiskManager, DiskManager = nullptr;

	if (ModuleManager)
		delete ModuleManager, ModuleManager = nullptr;

	if (TaskManager && !TaskManager->IsPanic())
	{
		TaskManager->SignalShutdown();
		delete TaskManager, TaskManager = nullptr;
	}

	if (fs)
		delete fs, fs = nullptr;

	if (TimeManager)
		delete TimeManager, TimeManager = nullptr;

	// if (Display)
	// 	delete Display, Display = nullptr;
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
