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

#include <exec.hpp>

#include <memory.hpp>
#include <lock.hpp>
#include <msexec.h>
#include <cwalk.h>
#include <elf.h>
#include <abi.h>

#ifdef DEBUG
#include <dumper.hpp>
#endif

#include "../../kernel.h"
#include "../../Fex.hpp"

using namespace Tasking;

NewLock(ExecuteServiceLock);

namespace Execute
{
	Memory::MemMgr *mem = nullptr;
	std::vector<SharedLibrary> Libs;

	void LibraryManagerService()
	{
		mem = new Memory::MemMgr;

		while (true)
		{
			{
				SmartLock(ExecuteServiceLock);
				foreach (auto &Lib in Libs)
				{
					if (Lib.RefCount > 0)
					{
						Lib.Timeout = TimeManager->CalculateTarget(10, Time::Units::Minutes);
						debug("Reset timeout for %s", Lib.Identifier);
						continue;
					}
					if (Lib.Timeout < TimeManager->GetCounter())
					{
						// TODO: Remove library from memory
						fixme("Removed library %s because of timeout", Lib.Identifier);
					}
					else
					{
						debug("Timeout for %s is %ld", Lib.Identifier, Lib.Timeout);
					}
				}
				debug("Waiting 10 seconds...");
			}
			TaskManager->Sleep(10000);
		}
	}

	bool AddLibrary(char *Identifier,
					VirtualFileSystem::File &ExFile,
					const Memory::Virtual &vmm)
	{
		SmartLock(ExecuteServiceLock);
		SharedLibrary sl;

		foreach (auto lib in Libs)
		{
			if (strcmp(lib.Identifier, Identifier) == 0)
			{
				debug("Library %s already loaded", Identifier);
				lib.RefCount++;
				return true;
			}
		}

		PCB *Process = TaskManager->GetCurrentProcess();
		ELFObject *obj = new ELFObject(vfs->GetPathFromNode(ExFile.GetNode()).get(), Process, true);
		if (!obj->IsValid())
		{
			error("Failed to load dynamic ELF");
			return false;
		}
		ELFBaseLoad bl = obj->GetBaseLoadInfo();

		strncpy(sl.Identifier, Identifier, sizeof(sl.Identifier) - 1);
		char *AbsolutePath = vfs->GetPathFromNode(ExFile.GetNode()).get();
		strncpy(sl.Path, AbsolutePath, sizeof(sl.Path) - 1);
		sl.Timeout = TimeManager->CalculateTarget(10, Time::Units::Minutes);
		sl.RefCount = 0;
		sl.MemoryImage = (uintptr_t)bl.MemoryImage;
		sl.Length = ExFile.GetLength();
		Libs.push_back(sl);
		debug("Library %s loaded at %#lx", Identifier, sl.MemoryImage);

		if (bl.InstructionPointer)
		{
			TCB *Thread = TaskManager->CreateThread(Process,
													bl.InstructionPointer,
													nullptr,
													nullptr,
													bl.auxv,
													Process->Info.Architecture,
													Process->Info.Compatibility,
													true);
			Thread->Rename(Identifier);
			Thread->Status = TaskStatus::Ready;

			foreach (Memory::MemMgr::AllocatedPages p in bl.TmpMem->GetAllocatedPagesList())
			{
				Thread->Memory->Add(p.Address, p.PageCount);
				bl.TmpMem->DetachAddress(p.Address);
			}
		}
		else
		{
			foreach (Memory::MemMgr::AllocatedPages p in bl.TmpMem->GetAllocatedPagesList())
			{
				/* FIXME: MEMORY LEAK */
				// Process->Memory->Add(p.Address, p.PageCount);
				bl.TmpMem->DetachAddress(p.Address);
				fixme("Potential memory leak. (%#lx - %ld)",
					  p.Address, p.PageCount);
			}
		}
		return true;
	}

	SharedLibrary GetLibrary(char *Identifier)
	{
		SmartLock(ExecuteServiceLock);
		foreach (auto Lib in Libs)
		{
			if (strcmp(Lib.Identifier, Identifier) == 0)
			{
				Lib.RefCount++;
				debug("Library %s found at %#lx", Identifier, Lib.MemoryImage);
				return Lib;
			}
		}
		error("Library %s not found", Identifier);
		return SharedLibrary();
	}
}
