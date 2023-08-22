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
#ifdef DEBUG
#include "Tests/t.h"
#endif

#include <filesystem/ustar.hpp>
#include <kshell.hpp>
#include <power.hpp>
#include <lock.hpp>
#include <printf.h>
#include <exec.hpp>
#include <cwalk.h>
#include <vm.hpp>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_NO_LINEAR
#define STBI_NO_THREAD_LOCALS
#define STBI_NO_HDR
#define STBI_ONLY_TGA
#include <stb/image.h>

#include "DAPI.hpp"
#include "Fex.hpp"

using VirtualFileSystem::Node;
using VirtualFileSystem::NodeFlags;

Driver::Driver *DriverManager = nullptr;
Disk::Manager *DiskManager = nullptr;
NetworkInterfaceManager::NetworkInterface *NIManager = nullptr;
VirtualFileSystem::Node *DevFS = nullptr;
VirtualFileSystem::Node *MntFS = nullptr;
VirtualFileSystem::Node *ProcFS = nullptr;
VirtualFileSystem::Node *VarLogFS = nullptr;

#ifdef DEBUG
void TreeFS(Node *node, int Depth)
{
	return;
	foreach (auto Chld in node->Children)
	{
		printf("%*c %s\eFFFFFF\n", Depth, ' ', Chld->Name);

		if (!Config.BootAnimation)
			Display->SetBuffer(0);
		TaskManager->Sleep(100);
		TreeFS(Chld, Depth + 1);
	}
}

const char *Statuses[] = {
	"FF0000", /* Unknown */
	"AAFF00", /* Ready */
	"00AA00", /* Running */
	"FFAA00", /* Sleeping */
	"FFAA00", /* Blocked */
	"FF0088", /* Zombie */
	"FF0000", /* Terminated */
};

const char *StatusesSign[] = {
	"Unknown",
	"Ready",
	"Run",
	"Sleep",
	"Wait",
	"Stop",
	"Terminated",
};

const char *SuccessSourceStrings[] = {
	"Unknown",
	"GetNextAvailableThread",
	"GetNextAvailableProcess",
	"SchedulerSearchProcessThread",
};

void TaskMgr_Dummy100Usage()
{
	while (1)
		;
}

void TaskMgr_Dummy0Usage()
{
	while (1)
		TaskManager->Sleep(1000000);
}

uint64_t GetUsage(uint64_t OldSystemTime, Tasking::TaskInfo *Info)
{
	/* https://github.com/reactos/reactos/blob/560671a784c1e0e0aa7590df5e0598c1e2f41f5a/base/applications/taskmgr/perfdata.c#L347 */
	if (Info->OldKernelTime || Info->OldUserTime)
	{
		uint64_t SystemTime = TimeManager->GetCounter() - OldSystemTime;
		uint64_t CurrentTime = Info->KernelTime + Info->UserTime;
		uint64_t OldTime = Info->OldKernelTime + Info->OldUserTime;
		uint64_t CpuUsage = (CurrentTime - OldTime) / SystemTime;
		CpuUsage = CpuUsage * 100;

		// debug("CurrentTime: %ld OldTime: %ld Time Diff: %ld Usage: %ld%%",
		//       CurrentTime, OldTime, SystemTime, CpuUsage);

		Info->OldKernelTime = Info->KernelTime;
		Info->OldUserTime = Info->UserTime;
		return CpuUsage;
	}
	Info->OldKernelTime = Info->KernelTime;
	Info->OldUserTime = Info->UserTime;
	return 0;
}

static int ShowTaskManager = 0;

void TaskMgr()
{
	thisThread->Rename("Debug Task Manager");
	thisThread->SetPriority(Tasking::Idle);

	while (ShowTaskManager == 0)
		CPU::Pause();

	thisThread->SetPriority(Tasking::Idle);

	TaskManager->CreateThread(thisProcess, Tasking::IP(TaskMgr_Dummy100Usage))->Rename("Dummy 100% Usage");
	TaskManager->CreateThread(thisProcess, Tasking::IP(TaskMgr_Dummy0Usage))->Rename("Dummy 0% Usage");

	while (true)
	{
		while (ShowTaskManager == 0)
			CPU::Pause();

		static int sanity = 0;
		Video::ScreenBuffer *sb = Display->GetBuffer(0);
		for (short i = 0; i < 1000; i++)
		{
			for (short j = 0; j < 500; j++)
			{
				uint32_t *Pixel = (uint32_t *)((uintptr_t)sb->Buffer + (j * sb->Width + i) * (bInfo.Framebuffer[0].BitsPerPixel / 8));
				*Pixel = 0x222222;
			}
		}

		uint32_t tmpX, tmpY;
		Display->GetBufferCursor(0, &tmpX, &tmpY);
		Display->SetBufferCursor(0, 0, 0);
		printf("\eF02C21Task Manager\n");
		static uint64_t OldSystemTime = 0;
		foreach (auto Proc in TaskManager->GetProcessList())
		{
			if (!Proc)
				continue;
			int Status = Proc->Status.load();
			uint64_t ProcessCpuUsage = GetUsage(OldSystemTime, &Proc->Info);
			printf("\e%s-> \eAABBCC%s \e00AAAA%s %ld%% (KT: %ld UT: %ld)\n",
				   Statuses[Status], Proc->Name, StatusesSign[Status], ProcessCpuUsage, Proc->Info.KernelTime, Proc->Info.UserTime);

			foreach (auto Thd in Proc->Threads)
			{
				if (!Thd)
					continue;
				Status = Thd->Status.load();
				uint64_t ThreadCpuUsage = GetUsage(OldSystemTime, &Thd->Info);
#if defined(a64)
				printf("  \e%s-> \eAABBCC%s \e00AAAA%s %ld%% (KT: %ld UT: %ld, IP: \e24FF2B%#lx \eEDFF24%s\e00AAAA)\n\eAABBCC",
					   Statuses[Status], Thd->Name, StatusesSign[Status], ThreadCpuUsage, Thd->Info.KernelTime,
					   Thd->Info.UserTime, Thd->Registers.rip,
					   Thd->Parent->ELFSymbolTable ? Thd->Parent->ELFSymbolTable->GetSymbolFromAddress(Thd->Registers.rip) : "unknown");
#elif defined(a32)
				printf("  \e%s-> \eAABBCC%s \e00AAAA%s %lld%% (KT: %lld UT: %lld, IP: \e24FF2B%#lx \eEDFF24%s\e00AAAA)\n\eAABBCC",
					   Statuses[Status], Thd->Name, StatusesSign[Status], ThreadCpuUsage, Thd->Info.KernelTime,
					   Thd->Info.UserTime, Thd->Registers.eip,
					   Thd->Parent->ELFSymbolTable ? Thd->Parent->ELFSymbolTable->GetSymbolFromAddress(Thd->Registers.eip) : "unknown");
#elif defined(aa64)
#endif
			}
		}
		OldSystemTime = TimeManager->GetCounter();
#if defined(a64)
		register uintptr_t CurrentStackAddress asm("rsp");
#elif defined(a32)
		register uintptr_t CurrentStackAddress asm("esp");
#elif defined(aa64)
		register uintptr_t CurrentStackAddress asm("sp");
#endif
		printf("Sanity: %d, Stack: %#lx", sanity++, CurrentStackAddress);
		if (sanity > 1000)
			sanity = 0;
		Display->SetBufferCursor(0, tmpX, tmpY);
		if (!Config.BootAnimation)
			Display->SetBuffer(0);

		TaskManager->Sleep(100);
	}
}

static int ShowOpenFiles = 0;

void lsof()
{
	thisThread->Rename("Debug File List");
	thisThread->SetPriority(Tasking::Idle);

	while (ShowOpenFiles == 0)
		CPU::Pause();

	thisThread->SetPriority(Tasking::High);

	vfs->Create("/dummy_lsof_file", NodeFlags::FILE);
	fopen("/dummy_lsof_file", "r");

	while (true)
	{
		while (ShowOpenFiles == 0)
			CPU::Pause();

		Video::ScreenBuffer *sb = Display->GetBuffer(0);
		for (short i = 0; i < 500; i++)
		{
			for (short j = 0; j < 500; j++)
			{
				uint32_t *Pixel = (uint32_t *)((uintptr_t)sb->Buffer + (j * sb->Width + i) * (bInfo.Framebuffer[0].BitsPerPixel / 8));
				*Pixel = 0x222222;
			}
		}

		uint32_t tmpX, tmpY;
		Display->GetBufferCursor(0, &tmpX, &tmpY);
		Display->SetBufferCursor(0, 0, 0);
		printf("\eF02C21Open Files (%ld):\e00AAAA\n",
			   TaskManager->GetProcessList().size());
		foreach (auto Proc in TaskManager->GetProcessList())
		{
			if (!Proc)
				continue;

			printf("%s:\n", Proc->Name);

			std::vector<VirtualFileSystem::FileDescriptorTable::Fildes> fds_array =
				Proc->FileDescriptors->GetFileDescriptors();
			foreach (auto fd in fds_array)
				printf("  %d: %s\n", fd.Descriptor, fd.Handle->AbsolutePath.c_str());
		}
		Display->SetBufferCursor(0, tmpX, tmpY);
		if (!Config.BootAnimation)
			Display->SetBuffer(0);
	}
}

#include <mutex>
std::mutex test_mutex;

void mutex_test_long()
{
	while (true)
	{
		test_mutex.lock();
		debug("Long Thread %d got mutex",
			  thisThread->ID);
		// TaskManager->Sleep(2000);
		test_mutex.unlock();
	}
}

void mutex_test()
{
	while (true)
	{
		test_mutex.lock();
		debug("Thread %d got mutex",
			  thisThread->ID);
		// TaskManager->Sleep(200);
		test_mutex.unlock();
	}
}
#endif

int SpawnInit()
{
	const char *envp[5] = {
		"PATH=/bin:/usr/bin",
		"TERM=tty",
		"HOME=/root",
		"USER=root",
		nullptr};

	const char *argv[4] = {
		Config.InitPath,
		"--init",
		"--critical",
		nullptr};

	return Execute::Spawn(Config.InitPath, argv, envp,
						  nullptr,
						  Tasking::TaskCompatibility::Native,
						  true);
}

/* Files: 0.tga 1.tga ... 26.tga */
uint8_t *Frames[27];
uint32_t FrameSizes[27];
size_t FrameCount = 1;

void BootLogoAnimationThread()
{
	char BootAnimPath[16];
	while (FrameCount < 27)
	{
		sprintf(BootAnimPath, "/etc/boot/%ld.tga", FrameCount);
		RefNode *frame = vfs->Open(BootAnimPath);
		if (!frame)
		{
			debug("Failed to load boot animation frame %s", BootAnimPath);
			break;
		}

		FrameSizes[FrameCount] = s_cst(uint32_t, frame->Length);
		Frames[FrameCount] = new uint8_t[frame->Length];
		frame->Read(Frames[FrameCount], frame->Length);
		delete frame;
		FrameCount++;
	}

	uint32_t DispX = Display->GetBuffer(1)->Width;
	uint32_t DispY = Display->GetBuffer(1)->Height;

	for (size_t i = 1; i < FrameCount; i++)
	{
		int x, y, channels;

		if (!stbi_info_from_memory((uint8_t *)Frames[i], FrameSizes[i],
								   &x, &y, &channels))
			continue;

		uint8_t *img = stbi_load_from_memory((uint8_t *)Frames[i],
											 FrameSizes[i], &x, &y,
											 &channels, STBI_rgb_alpha);

		if (img == NULL)
			continue;

		int offsetX = DispX / 2 - x / 2;
		int offsetY = DispY / 2 - y / 2;

		for (int i = 0; i < x * y; i++)
		{
			uint32_t pixel = ((uint32_t *)img)[i];
			int r = (pixel >> 16) & 0xFF;
			int g = (pixel >> 8) & 0xFF;
			int b = (pixel >> 0) & 0xFF;
			int a = (pixel >> 24) & 0xFF;

			if (a != 0xFF)
			{
				r = (r * a) / 0xFF;
				g = (g * a) / 0xFF;
				b = (b * a) / 0xFF;
			}

			Display->SetPixel((i % x) + offsetX, (i / x) + offsetY,
							  (r << 16) | (g << 8) | (b << 0), 1);
		}

		free(img);
		Display->SetBuffer(1);
		TaskManager->Sleep(50);
	}

	int brightness = 100;
	while (brightness >= 0)
	{
		brightness -= 10;
		Display->SetBrightness(brightness, 1);
		Display->SetBuffer(1);
		TaskManager->Sleep(5);
	}
}

void ExitLogoAnimationThread()
{
	Display->SetBrightness(100, 1);
	Display->SetBuffer(1);

	/* Files: 26.tga 25.tga ... 1.tga */
	uint32_t DispX = Display->GetBuffer(1)->Width;
	uint32_t DispY = Display->GetBuffer(1)->Height;

	for (size_t i = FrameCount - 1; i > 0; i--)
	{
		int x, y, channels;

		if (!stbi_info_from_memory((uint8_t *)Frames[i], FrameSizes[i],
								   &x, &y, &channels))
			continue;

		uint8_t *img = stbi_load_from_memory((uint8_t *)Frames[i],
											 FrameSizes[i], &x, &y,
											 &channels, STBI_rgb_alpha);

		if (img == NULL)
			continue;

		int offsetX = DispX / 2 - x / 2;
		int offsetY = DispY / 2 - y / 2;

		for (int i = 0; i < x * y; i++)
		{
			uint32_t pixel = ((uint32_t *)img)[i];
			int r = (pixel >> 16) & 0xFF;
			int g = (pixel >> 8) & 0xFF;
			int b = (pixel >> 0) & 0xFF;
			int a = (pixel >> 24) & 0xFF;

			if (a != 0xFF)
			{
				r = (r * a) / 0xFF;
				g = (g * a) / 0xFF;
				b = (b * a) / 0xFF;
			}

			Display->SetPixel((i % x) + offsetX, (i / x) + offsetY,
							  (r << 16) | (g << 8) | (b << 0), 1);
		}

		free(img);
		Display->SetBuffer(1);
		TaskManager->Sleep(50);
	}

	int brightness = 100;
	while (brightness >= 0)
	{
		brightness -= 10;
		Display->SetBrightness(brightness, 1);
		Display->SetBuffer(1);
		TaskManager->Sleep(5);
	}
}

void CleanupProcessesThreadWrapper()
{
	TaskManager->CleanupProcessesThread();
}

void KernelMainThread()
{
	// TaskManager->CreateThread(thisProcess, Tasking::IP(mutex_test_long));
	// TaskManager->Yield();
	// TaskManager->CreateThread(thisProcess, Tasking::IP(mutex_test));
	// TaskManager->CreateThread(thisProcess, Tasking::IP(mutex_test));
	// TaskManager->CreateThread(thisProcess, Tasking::IP(mutex_test));
	// TaskManager->CreateThread(thisProcess, Tasking::IP(mutex_test));
	// TaskManager->CreateThread(thisProcess, Tasking::IP(mutex_test));
	// TaskManager->CreateThread(thisProcess, Tasking::IP(mutex_test));
	// TaskManager->CreateThread(thisProcess, Tasking::IP(mutex_test));
	// TaskManager->CreateThread(thisProcess, Tasking::IP(mutex_test));
	// TaskManager->CreateThread(thisProcess, Tasking::IP(mutex_test));
	// ilp;

	Tasking::TCB *clnThd =
		TaskManager->CreateThread(thisProcess,
								  Tasking::IP(CleanupProcessesThreadWrapper));
	clnThd->SetPriority(Tasking::Idle);
	TaskManager->SetCleanupThread(clnThd);
	thisThread->SetPriority(Tasking::Critical);

	Tasking::TCB *blaThread = nullptr;

	if (Config.BootAnimation)
	{
		blaThread =
			TaskManager->CreateThread(thisProcess,
									  Tasking::IP(BootLogoAnimationThread));
		blaThread->Rename("Logo Animation");
	}

#ifdef DEBUG
	TaskManager->CreateThread(thisProcess, Tasking::IP(TaskMgr));
	TaskManager->CreateThread(thisProcess, Tasking::IP(lsof));
	TreeFS(vfs->GetRootNode(), 0);
#endif

	KPrint("Kernel Compiled at: %s %s with C++ Standard: %d",
		   __DATE__, __TIME__, CPP_LANGUAGE_STANDARD);
	KPrint("C++ Language Version (__cplusplus): %ld", __cplusplus);

	if (IsVirtualizedEnvironment())
		KPrint("Running in Virtualized Environment");

	KPrint("Initializing Disk Manager...");
	DiskManager = new Disk::Manager;

	KPrint("Loading Modules...");
	DriverManager = new Driver::Driver;
	DriverManager->LoadDrivers();

	KPrint("Fetching Disks...");
	if (DriverManager->GetDrivers().size() > 0)
	{
		foreach (auto Driver in DriverManager->GetDrivers())
			if (((FexExtended *)Driver.ExtendedHeaderAddress)->Driver.Type == FexDriverType::FexDriverType_Storage)
				DiskManager->FetchDisks(Driver.DriverUID);
	}
	else
		KPrint("\eE85230No disk drivers found! Cannot fetch disks!");

	KPrint("Initializing Network Interface Manager...");
	NIManager = new NetworkInterfaceManager::NetworkInterface;
	KPrint("Starting Network Interface Manager...");
	NIManager->StartService();

	KPrint("Setting up userspace");
	int ExitCode = -1;
	Tasking::TCB *initThread = nullptr;
	int tid = SpawnInit();
	if (tid < 0)
	{
		KPrint("\eE85230Failed to start %s! Code: %d", Config.InitPath, tid);
		goto Exit;
	}

	KPrint("Waiting for \e22AAFF%s\eCCCCCC to start...", Config.InitPath);
	thisThread->SetPriority(Tasking::Idle);

	initThread = TaskManager->GetThreadByID(tid);
	TaskManager->WaitForThread(initThread);
	ExitCode = initThread->GetExitCode();
Exit:
	if (ExitCode == 0)
	{
		KPrint("\eFF7900%s process exited with code %d and it didn't invoked the shutdown function.",
			   Config.InitPath, ExitCode);
		KPrint("System Halted");
		CPU::Halt(true);
	}

	KPrint("\eE85230Userspace process exited with code %d (%#x)",
		   ExitCode, ExitCode < 0 ? -ExitCode : ExitCode);
	KPrint("Dropping to kernel shell...");
	TaskManager->Sleep(1000);
	TaskManager->WaitForThread(blaThread);
	TaskManager->CreateThread(thisProcess,
							  Tasking::IP(KShellThread))
		->Rename("Kernel Shell");
	CPU::Halt(true);
}

NewLock(ShutdownLock);
void __no_stack_protector KernelShutdownThread(bool Reboot)
{
	SmartLock(ShutdownLock);
	debug("KernelShutdownThread(%s)", Reboot ? "true" : "false");
	if (Config.BootAnimation && TaskManager)
	{
		Tasking::TCB *elaThread =
			TaskManager->CreateThread(thisProcess,
									  Tasking::IP(ExitLogoAnimationThread));
		elaThread->Rename("Logo Animation");
		TaskManager->WaitForThread(elaThread);
	}

	BeforeShutdown(Reboot);

	trace("%s...", Reboot ? "Rebooting" : "Shutting down");
	if (Reboot)
		PowerManager->Reboot();
	else
		PowerManager->Shutdown();
	CPU::Stop();
}

void KST_Reboot() { KernelShutdownThread(true); }
void KST_Shutdown() { KernelShutdownThread(false); }
