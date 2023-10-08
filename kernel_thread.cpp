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
#include "tests/t.h"
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

#include "mapi.hpp"
#include "Fex.hpp"

using vfs::Node;
using vfs::NodeType;

Disk::Manager *DiskManager = nullptr;
Module::Module *ModuleManager = nullptr;
NetworkInterfaceManager::NetworkInterface *NIManager = nullptr;

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

void CleanupProcessesThreadWrapper()
{
	TaskManager->CleanupProcessesThread();
}

void KernelMainThread()
{
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
	// TaskManager->CreateThread(thisProcess, Tasking::IP(tasking_test_fb));
	// TaskManager->CreateThread(thisProcess, Tasking::IP(tasking_test_mutex));
	// ilp;
	TaskManager->CreateThread(thisProcess, Tasking::IP(TaskMgr));
	TaskManager->CreateThread(thisProcess, Tasking::IP(lsof));
	TreeFS(fs->GetRootNode(), 0);
#endif

	KPrint("Kernel Compiled at: %s %s with C++ Standard: %d",
		   __DATE__, __TIME__, CPP_LANGUAGE_STANDARD);
	KPrint("C++ Language Version (__cplusplus): %ld", __cplusplus);

	if (IsVirtualizedEnvironment())
		KPrint("Running in a virtualized environment");

	KPrint("Initializing Disk Manager...");
	DiskManager = new Disk::Manager;

	KPrint("Loading Modules...");
	ModuleManager = new Module::Module;
	ModuleManager->LoadModules();

	KPrint("Fetching Disks...");
	if (ModuleManager->GetModules().size() > 0)
	{
		foreach (auto mod in ModuleManager->GetModules())
			if (((FexExtended *)mod.ExtendedHeaderAddress)->Module.Type == FexModuleType::FexModuleType_Storage)
				DiskManager->FetchDisks(mod.modUniqueID);
	}
	else
		KPrint("\eE85230No disk modules found! Cannot fetch disks!");

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
	initThread->KeepInMemory = true;
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

	if (initThread)
		initThread->KeepInMemory = false;
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
