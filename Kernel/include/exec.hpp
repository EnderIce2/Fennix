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

    struct SharedLibraries
    {
        char Identifier[64];
        uint64_t Timeout;
        int RefCount;

        uintptr_t Address;
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
        void *Phyiscal;
        void *Virtual;
    };

    BinaryType GetBinaryType(void *Image);
    BinaryType GetBinaryType(char *Path);

    SpawnData Spawn(char *Path, const char **argv, const char **envp);

    ELFBaseLoad ELFLoad(char *Path, const char **argv, const char **envp,
                        Tasking::TaskCompatibility Compatibility = Tasking::TaskCompatibility::Native);

    Elf64_Shdr *GetELFSheader(Elf64_Ehdr *Header);
    Elf64_Shdr *GetELFSection(Elf64_Ehdr *Header, uint64_t Index);
    char *GetELFStringTable(Elf64_Ehdr *Header);
    char *ELFLookupString(Elf64_Ehdr *Header, uintptr_t Offset);
    Elf64_Sym *ELFLookupSymbol(Elf64_Ehdr *Header, const char *Name);
    uintptr_t ELFGetSymbolValue(Elf64_Ehdr *Header, uint64_t Table, uint64_t Index);
    Elf64_Dyn *ELFGetDynamicTag(void *ElfFile, enum DynamicArrayTags Tag);

    /**
     * @brief Create a ELF Memory Image
     *
     * @param mem The memory manager to use
     * @param pV Memory::Virtual object to use
     * @param ElfFile ELF file loaded in memory (FULL FILE)
     * @param Length Length of @p ElfFile
     * @return The Memory Image (Physical and Virtual)
     */
    MmImage ELFCreateMemoryImage(Memory::MemMgr *mem, Memory::Virtual &pV, void *ElfFile, size_t Length);

    uintptr_t LoadELFInterpreter(Memory::MemMgr *mem, Memory::Virtual &pV, const char *Interpreter);

    ELFBaseLoad ELFLoadRel(void *ElfFile,
                           VirtualFileSystem::File &ExFile,
                           Tasking::PCB *Process);

    ELFBaseLoad ELFLoadExec(void *ElfFile,
                            VirtualFileSystem::File &ExFile,
                            Tasking::PCB *Process);

    ELFBaseLoad ELFLoadDyn(void *ElfFile,
                           VirtualFileSystem::File &ExFile,
                           Tasking::PCB *Process);

    void StartExecuteService();
    bool AddLibrary(char *Identifier,
                    void *ElfImage,
                    size_t Length,
                    const Memory::Virtual &pV = Memory::Virtual());
    void SearchLibrary(char *Identifier);
    SharedLibraries GetLibrary(char *Identifier);
}

#endif // !__FENNIX_KERNEL_FILE_EXECUTE_H__
