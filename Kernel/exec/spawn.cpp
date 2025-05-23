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

#include "../kernel.h"

using namespace Tasking;

namespace Execute
{
	int Spawn(const char *Path, const char **argv, const char **envp,
			  Tasking::PCB *Parent, bool Fork,
			  Tasking::TaskCompatibility Compatibility,
			  bool Critical)
	{
		if (Parent == nullptr)
		{
			debug("no parent specified, using current process");
			Parent = thisProcess;
		}

		Node fd = fs->Lookup(Parent->Info.RootNode, Path);
		if (fd == nullptr)
			return -ENOENT;

		if (!fd->IsRegularFile())
		{
			if (fd->IsSymbolicLink())
			{
				char buffer[512];
				fs->ReadLink(fd, buffer, sizeof(buffer));
				fd = fs->Lookup(fd->Parent, buffer);
				if (fd == nullptr)
					return -ENOENT;
			}
			else
				return -ENOEXEC;
		}

		switch (GetBinaryType(fd))
		{
		case BinaryType::BinTypeELF:
		{
			TaskArchitecture Arch = TaskArchitecture::UnknownArchitecture;
			const char *BaseName;
			cwk_path_get_basename(Path, &BaseName, nullptr);
			Elf32_Ehdr ELFHeader;
			fs->Read(fd, &ELFHeader, sizeof(Elf32_Ehdr), 0);

			switch (ELFHeader.e_machine)
			{
			case EM_386:
				Arch = TaskArchitecture::x32;
				break;
			case EM_X86_64:
				Arch = TaskArchitecture::x64;
				break;
			case EM_ARM:
				Arch = TaskArchitecture::ARM32;
				break;
			case EM_AARCH64:
				Arch = TaskArchitecture::ARM64;
				break;
			default:
				error("Unknown ELF architecture %d",
					  ELFHeader.e_machine);
				break;
			}

			// TODO: This shouldn't be ignored
			if (ELFHeader.e_ident[EI_CLASS] == ELFCLASS32)
				fixme("32-bit ELF");
			else if (ELFHeader.e_ident[EI_CLASS] == ELFCLASS64)
				fixme("64-bit ELF");
			else
				fixme("Unknown class %d", ELFHeader.e_ident[EI_CLASS]);

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
			if (ELFHeader.e_ident[EI_DATA] != ELFDATA2LSB)
			{
				fixme("ELF32 LSB expected, got %d", ELFHeader.e_ident[EI_DATA]);
			}
#else
			if (ELFHeader.e_ident[EI_DATA] != ELFDATA2MSB)
			{
				fixme("ELF32 MSB expected, got %d", ELFHeader.e_ident[EI_DATA]);
			}
#endif

			/* ------------------------------------------------------------------------------------------------------------------------------ */

			PCB *Process;
			if (Fork)
			{
				CriticalSection cs;

				Process = Parent;
				for (auto tcb : Process->Threads)
				{
					debug("Deleting thread %d", tcb->ID);
					// delete tcb;
					tcb->SetState(Tasking::Terminated);
				}

				fixme("free allocated memory");
				// Process->vma->FreeAllPages();
			}
			else
			{
				Process = TaskManager->CreateProcess(Parent, BaseName, User, false, 0, 0);
				Process->Info.Compatibility = Compatibility;
				Process->Info.Architecture = Arch;
			}

			Node cwdNode = fs->Lookup(Parent->Info.RootNode, Path);
			Process->SetWorkingDirectory(fs->Convert(cwdNode->Parent));
			Process->SetExe(Path);

			ELFObject *obj = new ELFObject(Path, Process, argv, envp);
			if (!obj->IsValid)
			{
				error("Failed to load ELF object");
				delete Process;
				return -ENOEXEC;
			}

			vfs::FileDescriptorTable *pfdt = Parent->FileDescriptors;
			vfs::FileDescriptorTable *fdt = Process->FileDescriptors;

			auto ForkStdio = [pfdt, fdt](Node SearchNode)
			{
				if (unlikely(SearchNode.get() == nullptr))
					return false;

				for (const auto &ffd : pfdt->FileMap)
				{
					if (ffd.second.Flags & O_CLOEXEC)
						continue;

					if (ffd.second.node.get() != SearchNode.get())
						continue;

					fdt->usr_open(ffd.second.node->Path.c_str(), ffd.second.Flags, ffd.second.Mode);
					return true;
				}
				return false;
			};

			fixme("remove workarounds for stdio and tty");
			if (!Parent->tty)
				Process->tty = KernelConsole::CurrentTerminal.load()->Term;

			if (!ForkStdio(Parent->stdin))
				fdt->usr_open("/dev/console", O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

			if (!ForkStdio(Parent->stdout))
				fdt->usr_open("/dev/console", O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

			if (!ForkStdio(Parent->stderr))
				fdt->usr_open("/dev/console", O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

			TCB *Thread = nullptr;
			{
				CriticalSection cs;
				Thread = TaskManager->CreateThread(Process, obj->InstructionPointer,
												   obj->argv, obj->envp, obj->auxv,
												   Arch, Compatibility);
				Thread->SetCritical(Critical);
			}
			return Thread->ID;
		}
		default:
		{
			debug("Unknown binary type: %d", GetBinaryType(Path));
			return -ENOEXEC;
		}
		}

		return -ENOEXEC;
	}
}
