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
#include <vector>
#include <elf.h>

namespace Execute
{
	enum BinaryType
	{
		BinTypeInvalid,
		BinTypeFex,
		BinTypeELF,
		BinTypePE,
		BinTypeNE,
		BinTypeMZ,
		BinTypeUnknown
	};

	enum ExStatus
	{
		Unknown,
		OK,
		Unsupported,
		GenericError,
		LoadingProcedureFailed,
		InvalidFile,
		InvalidFileFormat,
		InvalidFileHeader,
		InvalidFileData,
		InvalidFileEntryPoint,
		InvalidFilePath
	};

	struct SpawnData
	{
		ExStatus Status;
		Tasking::PCB *Process;
		Tasking::TCB *Thread;
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

	struct ELFBaseLoad
	{
		bool Success;
		bool Interpreter;
		SpawnData sd;
		Tasking::IP InstructionPointer;

		std::vector<const char *> NeededLibraries;
		void *MemoryImage;
		void *VirtualMemoryImage;

		/* This should be deleted after copying the allocated pages to the thread
		   Intended to be used only inside BaseLoad.cpp */
		Memory::MemMgr *TmpMem;

		/* Same as above, for BaseLoad.cpp only */
		std::vector<AuxiliaryVector> auxv;
	};

	struct MmImage
	{
		void *Physical;
		void *Virtual;
	};

	class ELFObject
	{
	private:
		ELFBaseLoad BaseLoadInfo{};

		ELFBaseLoad LoadExec_x86_32(VirtualFileSystem::File &ElfFile,
									Tasking::PCB *TargetProcess);

		ELFBaseLoad LoadExec_x86_64(VirtualFileSystem::File &ElfFile,
									Tasking::PCB *TargetProcess);

		ELFBaseLoad LoadDyn_x86_32(VirtualFileSystem::File &ElfFile,
								   Tasking::PCB *TargetProcess,
								   bool IsLibrary);

		ELFBaseLoad LoadDyn_x86_64(VirtualFileSystem::File &ElfFile,
								   Tasking::PCB *TargetProcess,
								   bool IsLibrary);

	public:
		ELFBaseLoad GetBaseLoadInfo() { return BaseLoadInfo; }
		bool IsValid() { return BaseLoadInfo.Success; }

		ELFObject(char *AbsolutePath,
				  Tasking::PCB *TargetProcess,
				  bool IsLibrary = false);
		~ELFObject();
	};

	/* Full binary size. */
	BinaryType GetBinaryType(void *Image);

	BinaryType GetBinaryType(char *Path);

	SpawnData Spawn(char *Path, const char **argv, const char **envp);

	ELFBaseLoad ELFLoad(char *Path, const char **argv, const char **envp,
						Tasking::TaskCompatibility Compatibility = Tasking::TaskCompatibility::Native);

	void ELFInterpreterIPCThread(Tasking::PCB *TargetProcess,
								 std::string *TargetPath,
								 void *MemoryImage,
								 std::vector<const char *> *NeededLibraries);

	bool ELFIs64(void *Header);
	Elf64_Shdr *GetELFSheader(Elf64_Ehdr *Header);
	Elf64_Shdr *GetELFSection(Elf64_Ehdr *Header, uint64_t Index);
	char *GetELFStringTable(Elf64_Ehdr *Header);
	char *ELFLookupString(Elf64_Ehdr *Header, uintptr_t Offset);
	Elf64_Sym *ELFLookupSymbol(Elf64_Ehdr *Header, const char *Name);
	Elf64_Sym ELFLookupSymbol(VirtualFileSystem::File &ElfFile, const char *Name);
	uintptr_t ELFGetSymbolValue(Elf64_Ehdr *Header, uint64_t Table, uint64_t Index);

	std::vector<Elf64_Phdr> ELFGetSymbolType_x86_64(VirtualFileSystem::File &ElfFile, SegmentTypes Tag);
	std::vector<Elf32_Phdr> ELFGetSymbolType_x86_32(VirtualFileSystem::File &ElfFile, SegmentTypes Tag);

	std::vector<Elf64_Shdr> ELFGetSections_x86_64(VirtualFileSystem::File &ElfFile, const char *SectionName);
	std::vector<Elf32_Shdr> ELFGetSections_x86_32(VirtualFileSystem::File &ElfFile, const char *SectionName);

	std::vector<Elf64_Dyn> ELFGetDynamicTag_x86_64(VirtualFileSystem::File &ElfFile, DynamicArrayTags Tag);
	std::vector<Elf32_Dyn> ELFGetDynamicTag_x86_32(VirtualFileSystem::File &ElfFile, DynamicArrayTags Tag);

	void CopyLOADSegments(VirtualFileSystem::File &ElfFile, uintptr_t HdrsBase, uintptr_t PhysicalBase);
	void GetBaseAndSize(VirtualFileSystem::File &ElfFile, uintptr_t &Base, size_t &Size);

	/**
	 * @brief Create a ELF Memory Image
	 *
	 * @param mem The memory manager to use
	 * @param vmm Memory::Virtual object to use
	 * @param ElfFile The ELF File
	 * @param Length Length of @p ElfFile
	 * @return The Memory Image (Physical and Virtual)
	 */
	MmImage ELFCreateMemoryImage(Memory::MemMgr *mem,
								 Memory::Virtual &vmm,
								 VirtualFileSystem::File &ElfFile,
								 size_t Length);

	uintptr_t LoadELFInterpreter(Memory::MemMgr *mem,
								 Memory::Virtual &vmm,
								 const char *Interpreter);

	void LibraryManagerService();
	bool AddLibrary(char *Identifier,
					VirtualFileSystem::File &ExFile,
					const Memory::Virtual &vmm = Memory::Virtual());
	SharedLibrary GetLibrary(char *Identifier);
}

#endif // !__FENNIX_KERNEL_FILE_EXECUTE_H__
