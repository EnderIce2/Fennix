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
#include <kcon.hpp>
#include <cwalk.h>
#include <vm.hpp>
#include <vector>

int SpawnInit()
{
	const char *envp[6] = {
		"PATH=/bin:/usr/bin",
		"TERM=tty",
		"HOME=/root",
		"USER=root",
		"TZ=UTC",
		nullptr};

	const char *argv[4] = {
		Config.InitPath,
		"--kernel",
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
	TaskManager->CreateThread(thisProcess, Tasking::IP(TaskHeartbeat));
	TreeFS(fs->GetRoot(0), 0);
	coroutineTest();
#endif

	KPrint("Kernel compiled using GCC %d.%d.%d as of %s %s with Standard C++ %dL",
		   __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__,
		   __DATE__, __TIME__,
		   __cplusplus);

	if (IsVirtualizedEnvironment())
		KPrint("Running in a virtualized environment");

	KPrint("Initializing Driver Manager");
	DriverManager = new Driver::Manager;
	TaskManager->CreateThread(thisProcess, Tasking::IP(Driver::ManagerDaemonWrapper))
		->Rename("Device Service");

	KPrint("Loading Drivers");
	DriverManager->PreloadDrivers();
	DriverManager->LoadAllDrivers();

	KernelConsole::LateInit();
#ifdef DEBUG
	// TaskManager->CreateThread(thisProcess,
	// 						  Tasking::IP(KShellThread))
	// 	->Rename("Kernel Shell");
#endif

	KPrint("Executing %s", Config.InitPath);
	int ExitCode = -1;
	Tasking::PCB *initProc;
	Tasking::TCB *initThread;
	int tid = SpawnInit();
	if (tid < 0)
	{
		KPrint("\x1b[1;37;41mFailed to start %s! Error: %s (%d)",
			   Config.InitPath, strerror(tid), tid);
		goto Exit;
	}

	KPrint("Waiting for \x1b[32m%s\x1b[0m to start...",
		   Config.InitPath);
	thisThread->SetPriority(Tasking::Idle);

	initProc = TaskManager->GetProcessByID(tid);
	initThread = TaskManager->GetThreadByID(tid, initProc);
	TaskManager->WaitForThread(initThread);
	if (initThread)
		ExitCode = initThread->GetExitCode();
	else
		ExitCode = 0xEBAD;
Exit:
	KPrint("\x1b[31mUserspace process exited with code %d (%#x)",
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
