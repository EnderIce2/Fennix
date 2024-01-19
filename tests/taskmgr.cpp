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

#ifdef DEBUG

#include "t.h"

#include "../kernel.h"

const char *Statuses[] = {
	"FF0000", /* Unknown */
	"AAFF00", /* Ready */
	"00AA00", /* Running */
	"FFAA11", /* Sleeping */
	"FFAA0F", /* Blocked */
	"FFAA0F", /* Stopped */
	"FFAA5F", /* Waiting */

	"FF0088", /* Zombie */
	"FF0000", /* Terminated */
};

const char *StatusesSign[] = {
	"Unknown",
	"Ready",
	"Run",
	"Sleep",
	"Block",
	"Stop",
	"Wait",

	"Core",
	"Zombie",
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

	thisThread->SetPriority(Tasking::High);

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
			int State = Proc->State.load();
			uint64_t ProcessCpuUsage = GetUsage(OldSystemTime, &Proc->Info);
#if defined(a64)
			printf("\e%s-> \eAABBCC%s \e00AAAA%s %ld%% (KT: %ld UT: %ld)\n",
				   Statuses[State], Proc->Name, StatusesSign[State],
				   ProcessCpuUsage, Proc->Info.KernelTime, Proc->Info.UserTime);
#elif defined(a32)
			printf("\e%s-> \eAABBCC%s \e00AAAA%s %lld%% (KT: %lld UT: %lld)\n",
				   Statuses[State], Proc->Name, StatusesSign[State],
				   ProcessCpuUsage, Proc->Info.KernelTime, Proc->Info.UserTime);
#elif defined(aa64)
#endif

			foreach (auto Thd in Proc->Threads)
			{
				if (!Thd)
					continue;
				State = Thd->State.load();
				uint64_t ThreadCpuUsage = GetUsage(OldSystemTime, &Thd->Info);
#if defined(a64)
				printf("  \e%s-> \eAABBCC%s \e00AAAA%s %ld%% (KT: %ld UT: %ld, IP: \e24FF2B%#lx \eEDFF24%s\e00AAAA)\n\eAABBCC",
					   Statuses[State], Thd->Name, StatusesSign[State], ThreadCpuUsage, Thd->Info.KernelTime,
					   Thd->Info.UserTime, Thd->Registers.rip,
					   Thd->Parent->ELFSymbolTable ? Thd->Parent->ELFSymbolTable->GetSymbol(Thd->Registers.rip) : "unknown");
#elif defined(a32)
				printf("  \e%s-> \eAABBCC%s \e00AAAA%s %lld%% (KT: %lld UT: %lld, IP: \e24FF2B%#x \eEDFF24%s\e00AAAA)\n\eAABBCC",
					   Statuses[State], Thd->Name, StatusesSign[State], ThreadCpuUsage, Thd->Info.KernelTime,
					   Thd->Info.UserTime, Thd->Registers.eip,
					   Thd->Parent->ELFSymbolTable ? Thd->Parent->ELFSymbolTable->GetSymbol(Thd->Registers.eip) : "unknown");
#elif defined(aa64)
#endif
			}
		}
		OldSystemTime = TimeManager->GetCounter();
#if defined(a64)
		register uintptr_t CurrentStackAddress asm("rsp");
		printf("Sanity: %d, Stack: %#lx", sanity++, CurrentStackAddress);
#elif defined(a32)
		register uintptr_t CurrentStackAddress asm("esp");
		printf("Sanity: %d, Stack: %#x", sanity++, CurrentStackAddress);
#elif defined(aa64)
		register uintptr_t CurrentStackAddress asm("sp");
		printf("Sanity: %d, Stack: %#lx", sanity++, CurrentStackAddress);
#endif
		if (sanity > 1000)
			sanity = 0;
		Display->SetBufferCursor(0, tmpX, tmpY);
		if (!Config.Quiet)
			Display->SetBuffer(0);

		TaskManager->Sleep(100);
	}
}

#endif // DEBUG
