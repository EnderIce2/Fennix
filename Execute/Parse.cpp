#include <exec.hpp>

#include <msexec.h>

#include "../kernel.h"
#include "../Fex.hpp"

namespace Execute
{
    BinaryType GetBinaryType(char *Path)
    {
        BinaryType Type = BinaryType::BinTypeInvalid;
        FileSystem::FILE *ExFile = vfs->Open(Path);

        if (ExFile->Status == FileSystem::FileStatus::OK)
        {
            if (ExFile->Node->Flags == FileSystem::NodeFlags::FS_FILE)
            {
                Fex *FexHdr = (Fex *)ExFile->Node->Address;
                Elf64_Ehdr *ELFHeader = (Elf64_Ehdr *)ExFile->Node->Address;
                IMAGE_DOS_HEADER *MZHeader = (IMAGE_DOS_HEADER *)ExFile->Node->Address;
                if (FexHdr->Magic[0] == 'F' && FexHdr->Magic[1] == 'E' && FexHdr->Magic[2] == 'X' && FexHdr->Magic[3] == '\0')
                {
                    if (FexHdr->Type == FexFormatType::FexFormatType_Executable)
                    {
                        trace("%s - Fex", Path);
                        Type = BinaryType::BinTypeFex;
                        goto Exit;
                    }
                }
                else if (ELFHeader->e_ident[EI_MAG0] == ELFMAG0 &&
                         ELFHeader->e_ident[EI_MAG1] == ELFMAG1 &&
                         ELFHeader->e_ident[EI_MAG2] == ELFMAG2 &&
                         ELFHeader->e_ident[EI_MAG3] == ELFMAG3)
                {
                    trace("%s - ELF", Path);
                    Type = BinaryType::BinTypeELF;
                    goto Exit;
                }
                else if (MZHeader->e_magic == IMAGE_DOS_SIGNATURE)
                {
                    IMAGE_NT_HEADERS *PEHeader = (IMAGE_NT_HEADERS *)(((char *)ExFile->Node->Address) + MZHeader->e_lfanew);
                    IMAGE_OS2_HEADER *NEHeader = (IMAGE_OS2_HEADER *)(((char *)ExFile->Node->Address) + MZHeader->e_lfanew);
                    if (NEHeader->ne_magic == IMAGE_OS2_SIGNATURE)
                    {
                        trace("%s - NE", Path);
                        Type = BinaryType::BinTypeNE;
                    }
                    else if (PEHeader->Signature == IMAGE_NT_SIGNATURE)
                    {
                        trace("%s - PE", Path);
                        Type = BinaryType::BinTypePE;
                    }
                    else
                    {
                        trace("%s - MZ", Path);
                        Type = BinaryType::BinTypeMZ;
                    }
                    goto Exit;
                }

                /* ... */

                Type = BinaryType::BinTypeUnknown;
            }
        }
    Exit:
        vfs->Close(ExFile);
        return Type;
    }

    /* Originally from https://wiki.osdev.org/ELF_Tutorial */

    static inline Elf64_Shdr *GetElfSheader(Elf64_Ehdr *Header) { return (Elf64_Shdr *)((uintptr_t)Header + Header->e_shoff); }
    static inline Elf64_Shdr *GetElfSection(Elf64_Ehdr *Header, uint64_t Index) { return &GetElfSheader(Header)[Index]; }

    static inline char *GetElfStringTable(Elf64_Ehdr *Header)
    {
        if (Header->e_shstrndx == SHN_UNDEF)
            return nullptr;
        return (char *)Header + GetElfSection(Header, Header->e_shstrndx)->sh_offset;
    }

    static inline char *elf_lookup_string(Elf64_Ehdr *Header, uintptr_t Offset)
    {
        char *StringTable = GetElfStringTable(Header);
        if (StringTable == nullptr)
            return nullptr;
        return StringTable + Offset;
    }

    static void *ElfLookupSymbol(Elf64_Ehdr *Header, const char *Name)
    {
        Elf64_Shdr *SymbolTable = nullptr;
        Elf64_Shdr *StringTable = nullptr;
        Elf64_Sym *Symbol = nullptr;
        char *String = nullptr;

        for (Elf64_Half i = 0; i < Header->e_shnum; i++)
        {
            Elf64_Shdr *shdr = GetElfSection(Header, i);
            switch (shdr->sh_type)
            {
            case SHT_SYMTAB:
                SymbolTable = shdr;
                StringTable = GetElfSection(Header, shdr->sh_link);
                break;
            }
        }

        if (SymbolTable == nullptr || StringTable == nullptr)
            return nullptr;

        for (size_t i = 0; i < (SymbolTable->sh_size / sizeof(Elf64_Sym)); i++)
        {
            Symbol = (Elf64_Sym *)((uintptr_t)Header + SymbolTable->sh_offset + (i * sizeof(Elf64_Sym)));
            String = (char *)((uintptr_t)Header + StringTable->sh_offset + Symbol->st_name);
            if (strcmp(String, Name) == 0)
                return (void *)Symbol->st_value;
        }
        return nullptr;
    }

    static uintptr_t ELFGetSymbolValue(Elf64_Ehdr *Header, uint64_t Table, uint32_t Index)
    {
        if (Table == SHN_UNDEF || Index == SHN_UNDEF)
            return 0;
        Elf64_Shdr *SymbolTable = GetElfSection(Header, Table);

        uint32_t STEntries = SymbolTable->sh_size / SymbolTable->sh_entsize;
        if (Index >= STEntries)
        {
            error("Symbol index out of range %d-%u.", Table, Index);
            return 0xdead;
        }

        uint64_t SymbolAddress = (uint64_t)Header + SymbolTable->sh_offset;
        Elf32_Sym *Symbol = &((Elf32_Sym *)SymbolAddress)[Index];

        if (Symbol->st_shndx == SHN_UNDEF)
        {
            Elf64_Shdr *StringTable = GetElfSection(Header, SymbolTable->sh_link);
            const char *Name = (const char *)Header + StringTable->sh_offset + Symbol->st_name;

            void *Target = ElfLookupSymbol(Header, Name);
            if (Target == nullptr)
            {
                if (ELF32_ST_BIND(Symbol->st_info) & STB_WEAK)
                    return 0;
                else
                {
                    error("Undefined external symbol \"%s\".", Name);
                    return 0xdead;
                }
            }
            else
                return (uintptr_t)Target;
        }
        else if (Symbol->st_shndx == SHN_ABS)
            return Symbol->st_value;
        else
        {
            Elf64_Shdr *Target = GetElfSection(Header, Symbol->st_shndx);
            return (uintptr_t)Header + Symbol->st_value + Target->sh_offset;
        }
    }

    void *ELFLoadRel(Elf64_Ehdr *Header)
    {
        Elf64_Shdr *shdr = GetElfSheader(Header);
        for (uint64_t i = 0; i < Header->e_shnum; i++)
        {
            Elf64_Shdr *Section = &shdr[i];
            if (Section->sh_type == SHT_NOBITS)
            {
                if (!Section->sh_size)
                    continue;
                if (Section->sh_flags & SHF_ALLOC)
                {
                    void *Buffer = KernelAllocator.RequestPages(TO_PAGES(Section->sh_size));
                    memset(Buffer, 0, Section->sh_size);

                    Memory::Virtual pva = Memory::Virtual(/* TODO TODO TODO TODO TODO TODO */);
                    for (size_t i = 0; i < TO_PAGES(Section->sh_size); i++)
                        pva.Map((void *)((uintptr_t)Buffer + (i * PAGE_SIZE)), (void *)((uintptr_t)Buffer + (i * PAGE_SIZE)), Memory::PTFlag::RW | Memory::PTFlag::US);

                    Section->sh_offset = (uint64_t)Buffer - (uint64_t)Header;
                    debug("Section %ld", Section->sh_size);
                }
            }
        }

        for (size_t i = 0; i < Header->e_shnum; i++)
        {
            Elf64_Shdr *Section = &shdr[i];
            if (Section->sh_type == SHT_REL)
            {
                for (size_t Index = 0; Index < Section->sh_size / Section->sh_entsize; Index++)
                {
                    Elf64_Rel *RelTable = &((Elf64_Rel *)((uintptr_t)Header + Section->sh_offset))[Index];
                    Elf64_Shdr *Target = GetElfSection(Header, Section->sh_info);

                    uintptr_t *RelAddress = (uintptr_t *)(((uintptr_t)Header + Target->sh_offset) + RelTable->r_offset);
                    uint64_t SymbolValue = 0;

                    if (ELF64_R_SYM(RelTable->r_info) != SHN_UNDEF)
                    {
                        SymbolValue = ELFGetSymbolValue(Header, Section->sh_link, ELF64_R_SYM(RelTable->r_info));
                        if (SymbolValue == 0xdead)
                            return (void *)0xdeadbeef;
                    }

                    switch (ELF64_R_TYPE(RelTable->r_info))
                    {
                    case R_386_NONE:
                        break;
                    case R_386_32:
                        *RelAddress = DO_64_64(SymbolValue, *RelAddress);
                        break;
                    case R_386_PC32:
                        *RelAddress = DO_64_PC32(SymbolValue, *RelAddress, (uintptr_t)RelAddress);
                        break;
                    default:
                        error("Unsupported relocation type: %d", ELF64_R_TYPE(RelTable->r_info));
                        return (void *)0xdeadbeef;
                    }
                    debug("Symbol value: %#lx", SymbolValue);
                }
            }
        }
        return (void *)Header->e_entry;
    }
}
