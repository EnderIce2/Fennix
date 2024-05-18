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

#include <filesystem.hpp>
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

	struct SharedLibrary
	{
		char Identifier[64];
		char Path[256];
		uint64_t Timeout;
		int RefCount;

		uintptr_t MemoryImage;
		size_t Length;
	};

	struct MmImage
	{
		void *Physical;
		void *Virtual;
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

		void GenerateAuxiliaryVector_x86_32(Memory::VirtualMemoryArea *vma,
											FileNode *fd, Elf32_Ehdr ELFHeader,
											uint32_t EntryPoint,
											uint32_t BaseAddress);

		void GenerateAuxiliaryVector_x86_64(Memory::VirtualMemoryArea *vma,
											FileNode *fd, Elf64_Ehdr ELFHeader,
											uint64_t EntryPoint,
											uint64_t BaseAddress);

		void LoadExec_x86_32(FileNode *fd, Tasking::PCB *TargetProcess);
		void LoadExec_x86_64(FileNode *fd, Tasking::PCB *TargetProcess);
		void LoadDyn_x86_32(FileNode *fd, Tasking::PCB *TargetProcess);
		void LoadDyn_x86_64(FileNode *fd, Tasking::PCB *TargetProcess);
		bool LoadInterpreter(FileNode *fd, Tasking::PCB *TargetProcess);

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

	BinaryType GetBinaryType(FileNode *Path);
	BinaryType GetBinaryType(std::string Path);

	int Spawn(char *Path, const char **argv, const char **envp,
			  Tasking::PCB *Parent = nullptr, bool Fork = false,
			  Tasking::TaskCompatibility Compatibility = Tasking::Native,
			  bool Critical = false);

	bool ELFIs64(void *Header);
	Elf64_Shdr *GetELFSheader(Elf64_Ehdr *Header);
	Elf64_Shdr *GetELFSection(Elf64_Ehdr *Header, uint64_t Index);
	char *GetELFStringTable(Elf64_Ehdr *Header);
	char *ELFLookupString(Elf64_Ehdr *Header, uintptr_t Offset);
	Elf64_Sym *ELFLookupSymbol(Elf64_Ehdr *Header, std::string Name);
	Elf64_Sym ELFLookupSymbol(FileNode *fd, std::string Name);
	uintptr_t ELFGetSymbolValue(Elf64_Ehdr *Header, uint64_t Table, uint64_t Index);

	std::vector<Elf64_Phdr> ELFGetSymbolType_x86_64(FileNode *fd, SegmentTypes Tag);
	std::vector<Elf32_Phdr> ELFGetSymbolType_x86_32(FileNode *fd, SegmentTypes Tag);

	std::vector<Elf64_Shdr> ELFGetSections_x86_64(FileNode *fd, std::string SectionName);
	std::vector<Elf32_Shdr> ELFGetSections_x86_32(FileNode *fd, std::string SectionName);

	std::vector<Elf64_Dyn> ELFGetDynamicTag_x86_64(FileNode *fd, DynamicArrayTags Tag);
	std::vector<Elf32_Dyn> ELFGetDynamicTag_x86_32(FileNode *fd, DynamicArrayTags Tag);
}

#endif // !__FENNIX_KERNEL_FILE_EXECUTE_H__
