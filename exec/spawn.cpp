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
	int Spawn(char *Path, const char **argv, const char **envp,
			  Tasking::PCB *Parent, bool Fork,
			  Tasking::TaskCompatibility Compatibility,
			  bool Critical)
	{
		int fd = fopen(Path, "r");
		if (fd < 0)
			return fd;

		struct stat statbuf;
		fstat(fd, &statbuf);
		if (!S_ISREG(statbuf.st_mode))
		{
			fclose(fd);
			return -EISDIR;
		}

		switch (GetBinaryType(Path))
		{
		case BinaryType::BinTypeELF:
		{
			TaskArchitecture Arch = TaskArchitecture::UnknownArchitecture;
			const char *BaseName;
			cwk_path_get_basename(Path, &BaseName, nullptr);
			Elf32_Ehdr ELFHeader;
			fread(fd, (uint8_t *)&ELFHeader, sizeof(Elf32_Ehdr));

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

			void *ElfFile = KernelAllocator.RequestPages(TO_PAGES(statbuf.st_size + 1));
			fread(fd, (uint8_t *)ElfFile, statbuf.st_size);
			debug("Loaded elf %s at %#lx with the length of %ld",
				  Path, ElfFile, statbuf.st_size);

			PCB *Process;
			if (Fork)
			{
				assert(Parent != nullptr);
				CriticalSection cs;

				Process = Parent;
				foreach (auto tcb in Process->Threads)
				{
					debug("Deleting thread %d", tcb->ID);
					// delete tcb;
					tcb->SetState(Tasking::Terminated);
				}

				fixme("free allocated memory");
				fixme("change symbol table");
				// Process->vma->FreeAllPages();
				delete Process->ELFSymbolTable;
				Process->ELFSymbolTable = new SymbolResolver::Symbols((uintptr_t)ElfFile);
			}
			else
			{
				if (Parent == nullptr)
					Parent = thisProcess;

				Process = TaskManager->CreateProcess(Parent,
													 BaseName,
													 TaskExecutionMode::User,
													 ElfFile, false,
													 0, 0);
				Process->Info.Compatibility = Compatibility;
				Process->Info.Architecture = Arch;
			}

			Process->SetWorkingDirectory(fs->GetNodeFromPath(Path)->Parent);
			Process->SetExe(Path);

			KernelAllocator.FreePages(ElfFile, TO_PAGES(statbuf.st_size + 1));

			ELFObject *obj = new ELFObject(Path, Process, argv, envp);
			if (!obj->IsValid)
			{
				error("Failed to load ELF object");
				fclose(fd);
				delete Process;
				return -ENOEXEC;
			}

			vfs::FileDescriptorTable *pfdt = Parent->FileDescriptors;
			vfs::FileDescriptorTable *fdt = Process->FileDescriptors;

			auto ForkStdio = [pfdt, fdt](Node *SearchNode)
			{
				if (unlikely(SearchNode == nullptr))
					return false;

				std::vector<FileDescriptorTable::Fildes>
					pfds = pfdt->GetFileDescriptors();

				foreach (auto ffd in pfds)
				{
					if (ffd.Flags & O_CLOEXEC)
						continue;

					if (ffd.Handle->node == SearchNode)
					{
						fdt->_open(ffd.Handle->node->FullPath,
								   ffd.Flags, ffd.Mode);
						return true;
					}
				}
				return false;
			};

			if (!ForkStdio(Parent->stdin))
				fdt->_open("/dev/kcon", O_RDWR, 0666);

			if (!ForkStdio(Parent->stdout))
				fdt->_open("/dev/kcon", O_RDWR, 0666);

			if (!ForkStdio(Parent->stderr))
				fdt->_open("/dev/kcon", O_RDWR, 0666);

			TCB *Thread = nullptr;
			{
				CriticalSection cs;
				Thread = TaskManager->CreateThread(Process,
												   obj->InstructionPointer,
												   obj->argv, obj->envp, obj->auxv,
												   Arch,
												   Compatibility);
				Thread->SetCritical(Critical);
			}
			fclose(fd);
			return Thread->ID;
		}
		default:
		{
			debug("Unknown binary type: %d",
				  GetBinaryType(Path));
			fclose(fd);
			return -ENOEXEC;
		}
		}

		fclose(fd);
		return -ENOEXEC;
	}
}
