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
#include <std.hpp>
#include <errno.h>
#include <vector>
#include <elf.h>

namespace Execute
{
	enum BinaryType : int
	{
		BinTypeInvalid,
		BinTypeFex,
		BinTypeELF,
		BinTypePE,
		BinTypeNE,
		BinTypeMZ,
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

		void LoadPhdrs_x86_32(int fd,
							  Elf64_Ehdr ELFHeader,
							  Memory::VirtualMemoryArea *vma,
							  Tasking::PCB *TargetProcess);

		void LoadPhdrs_x86_64(int fd,
							  Elf64_Ehdr ELFHeader,
							  Memory::VirtualMemoryArea *vma,
							  Tasking::PCB *TargetProcess);

		void GenerateAuxiliaryVector_x86_32(Memory::VirtualMemoryArea *vma,
											int fd,
											Elf32_Ehdr ELFHeader,
											uint32_t EntryPoint,
											uint32_t BaseAddress);

		void GenerateAuxiliaryVector_x86_64(Memory::VirtualMemoryArea *vma,
											int fd,
											Elf64_Ehdr ELFHeader,
											uint64_t EntryPoint,
											uint64_t BaseAddress);

		void LoadExec_x86_32(int fd,
							 Tasking::PCB *TargetProcess);

		void LoadExec_x86_64(int fd,
							 Tasking::PCB *TargetProcess);

		void LoadDyn_x86_32(int fd,
							Tasking::PCB *TargetProcess);

		void LoadDyn_x86_64(int fd,
							Tasking::PCB *TargetProcess);

		bool LoadInterpreter(int fd,
							 Tasking::PCB *TargetProcess);

	public:
		decltype(IsElfValid) &IsValid = IsElfValid;
		decltype(ip) &InstructionPointer = ip;
		decltype(ELFargv) &argv = ELFargv;
		decltype(ELFenvp) &envp = ELFenvp;
		decltype(Elfauxv) &auxv = Elfauxv;

		ELFObject(char *AbsolutePath,
				  Tasking::PCB *TargetProcess,
				  const char **argv,
				  const char **envp);
		~ELFObject();
	};

	BinaryType GetBinaryType(const char *Path);

	int Spawn(char *Path, const char **argv, const char **envp,
			  Tasking::PCB *Parent = nullptr,
			  Tasking::TaskCompatibility Compatibility = Tasking::TaskCompatibility::Native,
			  bool Critical = false);

	bool ELFIs64(void *Header);
	Elf64_Shdr *GetELFSheader(Elf64_Ehdr *Header);
	Elf64_Shdr *GetELFSection(Elf64_Ehdr *Header, uint64_t Index);
	char *GetELFStringTable(Elf64_Ehdr *Header);
	char *ELFLookupString(Elf64_Ehdr *Header, uintptr_t Offset);
	Elf64_Sym *ELFLookupSymbol(Elf64_Ehdr *Header, const char *Name);
	Elf64_Sym ELFLookupSymbol(int fd, const char *Name);
	uintptr_t ELFGetSymbolValue(Elf64_Ehdr *Header, uint64_t Table, uint64_t Index);

	std::vector<Elf64_Phdr> ELFGetSymbolType_x86_64(int fd, SegmentTypes Tag);
	std::vector<Elf32_Phdr> ELFGetSymbolType_x86_32(int fd, SegmentTypes Tag);

	std::vector<Elf64_Shdr> ELFGetSections_x86_64(int fd, const char *SectionName);
	std::vector<Elf32_Shdr> ELFGetSections_x86_32(int fd, const char *SectionName);

	std::vector<Elf64_Dyn> ELFGetDynamicTag_x86_64(int fd, DynamicArrayTags Tag);
	std::vector<Elf32_Dyn> ELFGetDynamicTag_x86_32(int fd, DynamicArrayTags Tag);
}

#endif // !__FENNIX_KERNEL_FILE_EXECUTE_H__
