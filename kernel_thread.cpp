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

using vfs::Node;
using vfs::NodeType;

Disk::Manager *DiskManager = nullptr;
Driver::Manager *DriverManager = nullptr;
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
		// "--help",
		nullptr};

	Tasking::TaskCompatibility compat = Tasking::Native;
	if (Config.UseLinuxSyscalls)
		compat = Tasking::Linux;

	return Execute::Spawn(Config.InitPath, argv, envp,
						  nullptr, false, compat, true);
}

void KernelMainThread()
{
	thisThread->SetPriority(Tasking::Critical);

#ifdef DEBUG
	StressKernel();
	// TaskManager->CreateThread(thisProcess, Tasking::IP(tasking_test_fb));
	// TaskManager->CreateThread(thisProcess, Tasking::IP(tasking_test_mutex));
	// ilp;
	TaskManager->CreateThread(thisProcess, Tasking::IP(TaskMgr));
	TaskManager->CreateThread(thisProcess, Tasking::IP(lsof));
	TaskManager->CreateThread(thisProcess, Tasking::IP(TaskHeartbeat));
	TreeFS(fs->GetRootNode(), 0);
#endif

	KPrint("Kernel Compiled at: %s %s with C++ Standard: %d",
		   __DATE__, __TIME__, CPP_LANGUAGE_STANDARD);
	KPrint("C++ Language Version (__cplusplus): %ld", __cplusplus);

	if (IsVirtualizedEnvironment())
		KPrint("Running in a virtualized environment");

	KPrint("Initializing Disk Manager");
	DiskManager = new Disk::Manager;

	KPrint("Loading Drivers");
	DriverManager = new Driver::Manager;
	DriverManager->LoadAllDrivers();

	// KPrint("Fetching Disks");
	/* KernelCallback */
	// if (DriverManager->GetModules().size() > 0)
	// {
	// 	foreach (auto mod in DriverManager->GetModules())
	// 		if (((FexExtended *)mod.ExtendedHeaderAddress)->Driver.Type == FexDriverType::FexDriverType_Storage)
	// 			DiskManager->FetchDisks(mod.modUniqueID);
	// }
	// else
	// 	KPrint("\eE85230No disk driver found! Cannot fetch disks!");

	// KPrint("Initializing Network Interface Manager");
	// NIManager = new NetworkInterfaceManager::NetworkInterface;
	// KPrint("Starting Network Interface Manager");
	// NIManager->StartService();

#ifdef DEBUG
	// TaskManager->CreateThread(thisProcess,
	// 						  Tasking::IP(KShellThread))
	// 	->Rename("Kernel Shell");
#endif

	KPrint("Executing %s", Config.InitPath);
	int ExitCode = -1;
	Tasking::TCB *initThread;
	Tasking::PCB *initProc;
	int tid = SpawnInit();
	if (tid < 0)
	{
		KPrint("\eE85230Failed to start %s! Error: %s (%d)",
			   Config.InitPath, strerror(tid), tid);
		goto Exit;
	}

	KPrint("Waiting for \e22AAFF%s\eCCCCCC to start...",
		   Config.InitPath);
	thisThread->SetPriority(Tasking::Idle);

	initProc = TaskManager->GetProcessByID(tid);
	initThread = TaskManager->GetThreadByID(tid, initProc);
	TaskManager->WaitForThread(initThread);
	ExitCode = initThread->GetExitCode();
Exit:
	KPrint("\eE85230Userspace process exited with code %d (%#x)",
		   ExitCode, ExitCode < 0 ? -ExitCode : ExitCode);

	KPrint("Dropping to kernel shell");
	TaskManager->Sleep(1000);
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

	BeforeShutdown(Reboot);

	trace("%s...", Reboot ? "Rebooting" : "Shutting down");
	KPrint("Waiting for ACPI...");
	if (Reboot)
		PowerManager->Reboot();
	else
		PowerManager->Shutdown();
	KPrint("CPU Halted");
	CPU::Stop();
}

void KST_Reboot() { KernelShutdownThread(true); }
void KST_Shutdown() { KernelShutdownThread(false); }
