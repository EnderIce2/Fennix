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

void killChildren(Tasking::PCB *pcb)
{
	if (pcb->Children.empty())
	{
		KPrint("Process %s(%d) has no children", pcb->Name, pcb->ID);
		return;
	}

	std::vector<Tasking::PCB *> children = pcb->Children;

	for (auto child : children)
	{
		if (child->State.load() == Tasking::Terminated)
		{
			KPrint("Process %s(%d) is already dead", child->Name, child->ID);
			continue;
		}

		KPrint("Killing %s(%d)", child->Name, child->ID);
		killChildren(child);
		child->SetState(Tasking::Terminated);
		debug("killed %s(%d)", child->Name, child->ID);
	}
}

constexpr size_t chunk = 10 * 1024 * 1024; /* 10 MiB */
std::atomic_size_t totalAllocated = 0;
std::atomic_size_t highestScore = 0;
std::atomic_bool halt_fork = false;
std::vector<void *> allocatedChunks;
Tasking::PCB *baseProc = nullptr;
Tasking::PCB *lastProc = nullptr;
std::atomic_bool hold = false;
void StressKernel()
{
	return;

	static int once = 0;
	if (!once++)
	{
		debug("We have %d GiB of free memory",
			  TO_GiB(KernelAllocator.GetFreeMemory()));
		assert(TO_GiB(KernelAllocator.GetFreeMemory()) >= 1);
	}

	while (true)
	{
		while (hold.exchange(true, std::memory_order_acquire))
			;

		if (!halt_fork.load() && TaskManager->GetProcessList().size() > 100)
			halt_fork.store(true);

		void *ptr;
		Tasking::PCB *pcb = nullptr;
		if (TO_MiB(KernelAllocator.GetFreeMemory()) < 20)
		{
			KPrint("\x1b[1;31;41mNot enough memory left!");
			goto End;
		}

		ptr = KernelAllocator.RequestPages(TO_PAGES(chunk));
		if (ptr == nullptr)
		{
			KPrint("\x1b[1;31;41mFailed to allocate memory!");
			KPrint("Score is: %d MiB (current is %d MiB)",
				   TO_MiB(highestScore.load()), TO_MiB(totalAllocated.load()));
			continue;
		}
		KPrint("Allocated %d bytes at %#lx", chunk, ptr);
		allocatedChunks.push_back(ptr);
		totalAllocated.fetch_add(chunk);
		if (totalAllocated.load() > highestScore.load())
			highestScore.store(totalAllocated.load());
		KPrint("Total allocated: %d MiB [KERNEL: %d MiB free]",
			   TO_MiB(totalAllocated.load()), TO_MiB(KernelAllocator.GetFreeMemory()));

		if (lastProc == nullptr)
			lastProc = thisProcess;

		if (halt_fork.load() == false)
		{
			KPrint("Forking...");
			pcb = TaskManager->CreateProcess(lastProc, "STRESS TEST", Tasking::Kernel);
			lastProc = pcb;
			if (baseProc == nullptr)
				baseProc = pcb;
			TaskManager->CreateThread(pcb, Tasking::IP(StressKernel));
			KPrint("There are %d processes", TaskManager->GetProcessList().size());
		}

	End:
		hold.store(true);
		if (TO_GiB(totalAllocated.load()) >= 1)
		{
			KPrint("Freeing memory...");
			forItr(itr, allocatedChunks)
			{
				KPrint("Freeing %#lx", *itr);
				KernelAllocator.FreePages(*itr, TO_PAGES(chunk));
				allocatedChunks.erase(itr);
			}
			totalAllocated.store(0);
		}

		// if (TaskManager->GetProcessList().size() >= 100)
		// {
		// 	KPrint("Killing processes...");
		// 	killChildren(baseProc->Children.front());
		// 	KPrint("All processes killed.");
		// 	baseProc = nullptr;
		// }
		hold.store(false);
	}
}

#endif // DEBUG
