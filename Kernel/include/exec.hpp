#ifndef __FENNIX_KERNEL_FILE_EXECUTE_H__
#define __FENNIX_KERNEL_FILE_EXECUTE_H__

#include <types.h>

#include <filesystem.hpp>
#include <string.hpp>
#include <task.hpp>
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
        char Identifier[256];
        uint64_t Timeout;
        long RefCount;

        void *Address;
        void *MemoryImage;
        size_t Length;
    };

    struct ELFBaseLoad
    {
        bool Success;
        bool Interpreter;
        SpawnData sd;
        Tasking::IP InstructionPointer;

        Vector<String> NeededLibraries;
        void *MemoryImage;

        /* This should be deleted after copying the allocated pages to the thread
           Intended to be used only inside BaseLoad.cpp */
        Memory::MemMgr *TmpMem;

        /* Same as above, for BaseLoad.cpp only */
        Vector<AuxiliaryVector> auxv;
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
    void *ELFLookupSymbol(Elf64_Ehdr *Header, const char *Name);
    uintptr_t ELFGetSymbolValue(Elf64_Ehdr *Header, uint64_t Table, uint64_t Index);
    Elf64_Dyn *ELFGetDynamicTag(void *ElfFile, enum DynamicArrayTags Tag);

    /**
     * @brief Create a ELF Memory Image
     *
     * @param mem The memory manager to use
     * @param pV Memory::Virtual object to use
     * @param ElfFile ELF file loaded in memory (FULL FILE)
     * @param Length Length of @p ElfFile
     * @return void* The Memory Image
     */
    void *ELFCreateMemoryImage(Memory::MemMgr *mem, Memory::Virtual &pV, void *ElfFile, size_t Length);

    uintptr_t LoadELFInterpreter(Memory::MemMgr *mem, Memory::Virtual &pV, const char *Interpreter);

    ELFBaseLoad ELFLoadRel(void *ElfFile,
                           VirtualFileSystem::File *ExFile,
                           Tasking::PCB *Process);

    ELFBaseLoad ELFLoadExec(void *ElfFile,
                            VirtualFileSystem::File *ExFile,
                            Tasking::PCB *Process);

    ELFBaseLoad ELFLoadDyn(void *ElfFile,
                           VirtualFileSystem::File *ExFile,
                           Tasking::PCB *Process);

    void StartExecuteService();
    SharedLibraries *AddLibrary(char *Identifier,
                                void *ElfImage,
                                size_t Length,
                                const Memory::Virtual &pV = Memory::Virtual());
    void SearchLibrary(char *Identifier);
}

#endif // !__FENNIX_KERNEL_FILE_EXECUTE_H__
