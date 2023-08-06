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

#include <task.hpp>

#include <dumper.hpp>
#include <convert.h>
#include <lock.hpp>
#include <printf.h>
#include <smp.hpp>
#include <io.h>

#include "../kernel.h"

#if defined(a64)
#include "../Architecture/amd64/cpu/apic.hpp"
#include "../Architecture/amd64/cpu/gdt.hpp"
#elif defined(a32)
#include "../Architecture/i386/cpu/apic.hpp"
#elif defined(aa64)
#endif

// #define DEBUG_TASKING 1

#ifdef DEBUG_TASKING
#define tskdbg(m, ...)       \
	debug(m, ##__VA_ARGS__); \
	__sync
#else
#define tskdbg(m, ...)
#endif

using namespace InterProcessCommunication;
using namespace VirtualFileSystem;
using VirtualFileSystem::NodeFlags;

namespace Tasking
{
	void PCB::Rename(const char *name)
	{
		assert(name != nullptr);
		assert(strlen(name) > 0);

		trace("Renaming thread %s to %s", this->Name, name);
		if (this->Name)
			delete[] this->Name;
		this->Name = new char[strlen(name) + 1];
		strcpy((char *)this->Name, name);
	}

	void PCB::SetWorkingDirectory(VirtualFileSystem::Node *node)
	{
		debug("Setting working directory of process %s to %#lx (%s)",
			  this->Name, node, node->Name);
		CurrentWorkingDirectory = node;
	}

	PCB::PCB(Task *ctx, PCB *Parent, const char *Name,
			 TaskExecutionMode ExecutionMode, void *Image,
			 bool DoNotCreatePageTable,
			 uint16_t UserID, uint16_t GroupID)
	{
		assert(ctx != nullptr);
		assert(ExecutionMode >= _ExecuteModeMin);
		assert(ExecutionMode <= _ExecuteModeMax);

		this->ctx = ctx;
		this->ID = ctx->NextPID++;
		if (this->Name)
			delete[] this->Name;
		this->Name = new char[strlen(Name) + 1];
		strcpy((char *)this->Name, Name);
		this->ExitCode = KILL_CRASH;
		this->Security.ExecutionMode = ExecutionMode;

		if (Parent == nullptr)
			this->Parent = ctx->GetCurrentProcess();
		else
			this->Parent = Parent;

		if (this->Parent &&
			UserID == UINT16_MAX &&
			GroupID == UINT16_MAX)
		{
			UserID = this->Parent->Security.Real.UserID;
			GroupID = this->Parent->Security.Real.GroupID;
		}

		this->Security.Real.UserID = UserID;
		this->Security.Real.GroupID = GroupID;
		this->Security.Effective.UserID = UserID;
		this->Security.Effective.GroupID = GroupID;

		char ProcFSName[16];
		sprintf(ProcFSName, "%d", this->ID);
		this->ProcessDirectory = vfs->Create(ProcFSName, DIRECTORY, ProcFS);
		this->memDirectory = vfs->Create("mem", DIRECTORY, this->ProcessDirectory);
		this->FileDescriptors = new FileDescriptorTable(this);
		this->IPC = new class IPC((void *)this);

		if (!DoNotCreatePageTable)
		{
			OwnPageTable = true;
			switch (ExecutionMode)
			{
			case TaskExecutionMode::System:
				fixme("Mode not supported.");
				[[fallthrough]];
			case TaskExecutionMode::Kernel:
			{
				this->Security.IsCritical = true;
#if defined(a64)
				this->PageTable = (Memory::PageTable *)CPU::x64::readcr3().raw;
#elif defined(a32)
				this->PageTable = (Memory::PageTable *)CPU::x32::readcr3().raw;
#elif defined(aa64)
#endif
				debug("Process %s(%d) has page table at %#lx",
					  this->Name, this->ID, this->PageTable);
				break;
			}
			case TaskExecutionMode::User:
			{
#if defined(a64)
				this->PageTable = (Memory::PageTable *)KernelAllocator.RequestPages(TO_PAGES(sizeof(Memory::PageTable) + 1));
				memcpy(this->PageTable,
					   KernelPageTable,
					   sizeof(Memory::PageTable));
#elif defined(a32)
#elif defined(aa64)
#endif
				debug("Process %s(%d) has page table at %#lx",
					  this->Name, this->ID, this->PageTable);
				break;
			}
			default:
				assert(false);
			}
		}

		this->Memory = new Memory::MemMgr(this->PageTable, this->memDirectory);

		if (Image)
		{
			this->ELFSymbolTable = new SymbolResolver::Symbols((uintptr_t)Image);
		}
		else
		{
			debug("No image provided for process \"%s\"(%d)",
				  this->Name, this->ID);
		}

		if (Parent)
			Parent->Children.push_back(this);

		debug("Process page table: %#lx", this->PageTable);
		debug("Created process \"%s\"(%d). Parent \"%s\"(%d)",
			  this->Name, this->ID,
			  Parent ? this->Parent->Name : "None",
			  Parent ? this->Parent->ID : 0);

		this->Info.SpawnTime = TimeManager->GetCounter();
		ctx->ProcessList.push_back(this);
	}

	PCB::~PCB()
	{
		debug("Destroying process \"%s\"(%d)",
			  this->Name, this->ID);

		ctx->ProcessList.erase(std::find(ctx->ProcessList.begin(),
										 ctx->ProcessList.end(),
										 this));

		if (this->ELFSymbolTable)
			delete this->ELFSymbolTable;

		delete this->IPC;
		delete this->FileDescriptors;
		delete this->Memory;
		delete[] this->Name;

		if (this->PageTable && OwnPageTable)
		{
			size_t PTPgs = TO_PAGES(sizeof(Memory::PageTable) + 1);
			KernelAllocator.FreePages(this->PageTable, PTPgs);
		}

		foreach (auto pcb in this->Children)
			delete pcb;

		foreach (auto tcb in this->Threads)
			delete tcb;

		vfs->Delete(this->ProcessDirectory, true);

		if (this->Parent)
		{
			std::vector<Tasking::PCB *> &pChild = this->Parent->Children;

			pChild.erase(std::find(pChild.begin(),
								   pChild.end(),
								   this));
		}
	}
}
