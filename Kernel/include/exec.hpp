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

#ifndef __FENNIX_KERNEL_FILE_EXECUTE_H__
#define __FENNIX_KERNEL_FILE_EXECUTE_H__

#include <types.h>

#include <fs/vfs.hpp>
#include <task.hpp>
#include <errno.h>
#include <vector>
#include <elf.h>

namespace Execute
{
	enum BinaryType : int
	{
		BinTypeInvalid,
		BinTypeELF,
		BinTypePE,
		BinTypeNE,
		BinTypeMZ,
		BinTypeMachO,
		BinTypeUnknown
	};

	class ELFObject
	{
	private:
		bool IsElfValid;
		const char **ELFargv;
		const char **ELFenvp;
		std::vector<AuxiliaryVector> Elfauxv;
		Tasking::IP ip;
		void *ELFProgramHeaders;

		void GenerateAuxiliaryVector(Memory::VirtualMemoryArea *vma,
									 Node &fd, Elf_Ehdr ELFHeader,
									 uintptr_t EntryPoint,
									 uintptr_t BaseAddress);

		void LoadSegments(Node &fd, Tasking::PCB *TargetProcess, Elf_Ehdr &ELFHeader, uintptr_t &BaseAddress);

		void LoadExec(Node &fd, Tasking::PCB *TargetProcess);
		void LoadDyn(Node &fd, Tasking::PCB *TargetProcess);
		bool LoadInterpreter(Node &fd, Tasking::PCB *TargetProcess);

	public:
		decltype(IsElfValid) &IsValid = IsElfValid;
		decltype(ip) &InstructionPointer = ip;
		decltype(ELFargv) &argv = ELFargv;
		decltype(ELFenvp) &envp = ELFenvp;
		decltype(Elfauxv) &auxv = Elfauxv;

		ELFObject(std::string AbsolutePath,
				  Tasking::PCB *TargetProcess,
				  const char **argv,
				  const char **envp);
		~ELFObject();
	};

	BinaryType GetBinaryType(Node &Path);
	BinaryType GetBinaryType(std::string Path);

	int Spawn(const char *Path, const char **argv, const char **envp,
			  Tasking::PCB *Parent = nullptr, bool Fork = false,
			  Tasking::TaskCompatibility Compatibility = Tasking::Native,
			  bool Critical = false);

	bool ELFIs64(void *Header);
	Elf_Shdr *GetELFSheader(Elf_Ehdr *Header);
	Elf_Shdr *GetELFSection(Elf_Ehdr *Header, uintptr_t Index);
	char *GetELFStringTable(Elf_Ehdr *Header);
	char *ELFLookupString(Elf_Ehdr *Header, uintptr_t Offset);
	Elf_Sym *ELFLookupSymbol(Elf_Ehdr *Header, std::string Name);
	Elf_Sym ELFLookupSymbol(Node &fd, std::string Name);
	uintptr_t ELFGetSymbolValue(Elf_Ehdr *Header, uintptr_t Table, uintptr_t Index);

	std::vector<Elf_Phdr> ELFGetSymbolType(Node &fd, SegmentTypes Tag);
	std::vector<Elf_Shdr> ELFGetSections(Node &fd, std::string SectionName);
	std::vector<Elf_Dyn> ELFGetDynamicTag(Node &fd, DynamicArrayTags Tag);
}

#endif // !__FENNIX_KERNEL_FILE_EXECUTE_H__
