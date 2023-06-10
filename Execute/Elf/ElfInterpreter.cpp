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
#include <debug.h>

#include "../../kernel.h"

using namespace Tasking;

namespace Execute
{
	struct InterpreterIPCDataLibrary
	{
		char Name[64];
	};

	typedef struct
	{
		char Path[256];
		void *MemoryImage;
		struct InterpreterIPCDataLibrary Libraries[64];
	} InterpreterIPCData;

	void ELFInterpreterIPCThread(PCB *TargetProcess,
								 std::string *TargetPath,
								 void *MemoryImage,
								 std::vector<const char *> *_NeededLibraries)
	{
		std::vector<const char *> NeededLibraries = *_NeededLibraries;
		delete _NeededLibraries;

		debug("Interpreter thread started for %s", TargetPath->c_str());
		// Interpreter will create an IPC with token "LOAD".
		char UniqueToken[16] = {'L', 'O', 'A', 'D', '\0'};
		InterProcessCommunication::IPCHandle *Handle = nullptr;
		while (Handle == nullptr)
		{
			debug("Searching for IPC with token %s", UniqueToken);
			Handle = TargetProcess->IPC->SearchByToken(UniqueToken);
			if (Handle == nullptr)
				debug("Failed");

			TaskManager->Sleep(200);
			if (Handle == nullptr)
				debug("Retrying...");
		}
		debug("IPC found, sending data...");
		InterpreterIPCData *TmpBuffer = new InterpreterIPCData;
		strncpy(TmpBuffer->Path, TargetPath->c_str(), sizeof(TmpBuffer->Path) - 1);
		TmpBuffer->MemoryImage = MemoryImage;
		size_t NeededLibsSize = NeededLibraries.size();

		for (size_t i = 0; i < NeededLibsSize; i++)
			strncpy(TmpBuffer->Libraries[i].Name, NeededLibraries[i],
					sizeof(TmpBuffer->Libraries[i].Name) - 1);

#ifdef DEBUG
		debug("Input:");
		debug("Path: %s", TargetPath->c_str());
		debug("MemoryImage: %p", MemoryImage);
		for (size_t i = 0; i < NeededLibsSize; i++)
			debug("Library: %s", NeededLibraries[i]);

		debug("Buffer:");
		debug("Path: %s", TmpBuffer->Path);
		debug("MemoryImage: %p", TmpBuffer->MemoryImage);
		for (size_t i = 0; i < 64; i++)
		{
			if (TmpBuffer->Libraries[i].Name[0] != '\0')
				break;
			debug("Library: %s", TmpBuffer->Libraries[i].Name);
		}
#endif

	RetryIPCWrite:
		InterProcessCommunication::IPCErrorCode ret =
			TargetProcess->IPC->Write(Handle->ID, TmpBuffer, sizeof(InterpreterIPCData));
		debug("Write returned %d", ret);
		if (ret == InterProcessCommunication::IPCErrorCode::IPCNotListening)
		{
			debug("IPC not listening, retrying...");
			TaskManager->Sleep(100);
			goto RetryIPCWrite;
		}
		delete TmpBuffer;

		while (!TargetProcess->IPC->SearchByToken(UniqueToken))
			TaskManager->Schedule();

		debug("Interpreter thread finished for %s", TargetPath->c_str());

		for (size_t i = 0; i < NeededLibsSize; i++)
			delete[] NeededLibraries[i];

		delete TargetPath;
		TEXIT(0);
	}

	uintptr_t LoadELFInterpreter(Memory::MemMgr *mem, Memory::Virtual &vmm, const char *Interpreter)
	{
		if (GetBinaryType((char *)Interpreter) != BinaryType::BinTypeELF)
		{
			error("Interpreter \"%s\" is not an ELF file.", Interpreter);
			return 0;
		}

		/* No need to check if it's valid because
		the function that calls this already checks it. */
		VirtualFileSystem::File ElfFile = vfs->Open(Interpreter);

		Elf64_Ehdr ELFHeader;
		vfs->Read(ElfFile, (uint8_t *)&ELFHeader, sizeof(Elf64_Ehdr));
		debug("Interpreter type: %#x", ELFHeader.e_type);

		uintptr_t BaseAddress;
		size_t ElfPHDRsSize;

		GetBaseAndSize(ElfFile, BaseAddress, ElfPHDRsSize);
		MmImage MemoryImage = ELFCreateMemoryImage(mem, vmm, ElfFile, ElfPHDRsSize);
		CopyLOADSegments(ElfFile, BaseAddress, (uintptr_t)MemoryImage.Physical);
		vfs->Close(ElfFile);

		bool IsPIC = ELFHeader.e_type == ET_DYN;
		debug("Elf %s PIC", IsPIC ? "is" : "is not");

		if (IsPIC)
		{
			debug("Interpreter entry point: %#lx (%#lx + %#lx)",
				  (uintptr_t)MemoryImage.Physical + ELFHeader.e_entry,
				  (uintptr_t)MemoryImage.Physical, ELFHeader.e_entry);

			return (uintptr_t)MemoryImage.Physical + ELFHeader.e_entry;
		}
		else
		{
			debug("Interpreter entry point: %#lx", ELFHeader.e_entry);
			return ELFHeader.e_entry;
		}
	}
}
