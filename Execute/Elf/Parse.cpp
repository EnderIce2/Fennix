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

#include <msexec.h>

#include "../../kernel.h"
#include "../../Fex.hpp"

namespace Execute
{
    /* Originally from https://wiki.osdev.org/ELF_Tutorial */

    Elf64_Shdr *GetELFSheader(Elf64_Ehdr *Header)
    {
        return (Elf64_Shdr *)((uintptr_t)Header + Header->e_shoff);
    }

    Elf64_Shdr *GetELFSection(Elf64_Ehdr *Header, uint64_t Index)
    {
        return &GetELFSheader(Header)[Index];
    }

    char *GetELFStringTable(Elf64_Ehdr *Header)
    {
        if (Header->e_shstrndx == SHN_UNDEF)
            return nullptr;
        return (char *)Header + GetELFSection(Header, Header->e_shstrndx)->sh_offset;
    }

    char *ELFLookupString(Elf64_Ehdr *Header, uintptr_t Offset)
    {
        char *StringTable = GetELFStringTable(Header);
        if (StringTable == nullptr)
            return nullptr;
        return StringTable + Offset;
    }

    Elf64_Sym *ELFLookupSymbol(Elf64_Ehdr *Header, const char *Name)
    {
        Elf64_Shdr *SymbolTable = nullptr;
        Elf64_Shdr *StringTable = nullptr;
        Elf64_Sym *Symbol = nullptr;
        char *String = nullptr;

        for (Elf64_Half i = 0; i < Header->e_shnum; i++)
        {
            Elf64_Shdr *shdr = GetELFSection(Header, i);
            switch (shdr->sh_type)
            {
            case SHT_SYMTAB:
                SymbolTable = shdr;
                StringTable = GetELFSection(Header, shdr->sh_link);
                break;
            default:
            {
                break;
            }
            }
        }

        if (SymbolTable == nullptr || StringTable == nullptr)
            return nullptr;

        for (size_t i = 0; i < (SymbolTable->sh_size / sizeof(Elf64_Sym)); i++)
        {
            Symbol = (Elf64_Sym *)((uintptr_t)Header + SymbolTable->sh_offset + (i * sizeof(Elf64_Sym)));
            String = (char *)((uintptr_t)Header + StringTable->sh_offset + Symbol->st_name);
            if (strcmp(String, Name) == 0)
                return Symbol;
        }
        return nullptr;
    }

    uintptr_t ELFGetSymbolValue(Elf64_Ehdr *Header, uint64_t Table, uint64_t Index)
    {
        if (Table == SHN_UNDEF || Index == SHN_UNDEF)
            return 0;
        Elf64_Shdr *SymbolTable = GetELFSection(Header, Table);

        uint64_t STEntries = SymbolTable->sh_size / SymbolTable->sh_entsize;
        if (Index >= STEntries)
        {
            error("Symbol index out of range %d-%u.", Table, Index);
            return 0xdead;
        }

        uint64_t SymbolAddress = (uint64_t)Header + SymbolTable->sh_offset;
        Elf64_Sym *Symbol = &((Elf64_Sym *)SymbolAddress)[Index];

        if (Symbol->st_shndx == SHN_UNDEF)
        {
            Elf64_Shdr *StringTable = GetELFSection(Header, SymbolTable->sh_link);
            const char *Name = (const char *)Header + StringTable->sh_offset + Symbol->st_name;

            void *Target = (void *)ELFLookupSymbol(Header, Name)->st_value;
            if (Target == nullptr)
            {
                if (ELF64_ST_BIND(Symbol->st_info) & STB_WEAK)
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
            Elf64_Shdr *Target = GetELFSection(Header, Symbol->st_shndx);
            return (uintptr_t)Header + Symbol->st_value + Target->sh_offset;
        }
    }

    Elf64_Dyn *ELFGetDynamicTag(void *ElfFile, enum DynamicArrayTags Tag)
    {
        Elf64_Ehdr *ELFHeader = (Elf64_Ehdr *)ElfFile;

        Elf64_Phdr ItrPhdr;
        for (Elf64_Half i = 0; i < ELFHeader->e_phnum; i++)
        {
            memcpy(&ItrPhdr, (uint8_t *)ElfFile + ELFHeader->e_phoff + ELFHeader->e_phentsize * i, sizeof(Elf64_Phdr));
            if (ItrPhdr.p_type == PT_DYNAMIC)
            {
                Elf64_Dyn *Dynamic = (Elf64_Dyn *)((uint8_t *)ElfFile + ItrPhdr.p_offset);
                for (size_t i = 0; i < ItrPhdr.p_filesz / sizeof(Elf64_Dyn); i++)
                {
                    if (Dynamic[i].d_tag == Tag)
                    {
                        debug("Found dynamic tag %d at %#lx [d_val: %#lx].", Tag, &Dynamic[i], Dynamic[i].d_un.d_val);
                        return &Dynamic[i];
                    }
                    if (Dynamic[i].d_tag == DT_NULL)
                    {
                        debug("Reached end of dynamic tag list for tag %d.", Tag);
                        return nullptr;
                    }
                }
            }
        }
        debug("Dynamic tag %d not found.", Tag);
        return nullptr;
    }

    MmImage ELFCreateMemoryImage(Memory::MemMgr *mem, Memory::Virtual &pV, void *ElfFile, size_t Length)
    {
        void *MemoryImage = nullptr;
        Elf64_Ehdr *ELFHeader = (Elf64_Ehdr *)ElfFile;
        bool IsPIC = ELFHeader->e_type == ET_DYN;
        debug("Elf %s PIC", IsPIC ? "is" : "is not");

        /* TODO: Not sure what I am supposed to do with this.
         * It is supposed to detect if it's PIC or not but I
         * don't know if it's right. */
        if (ELFGetDynamicTag(ElfFile, DT_TEXTREL))
        {
            fixme("Text relocation is not(?) tested yet!");
            MemoryImage = (uint8_t *)mem->RequestPages(TO_PAGES(Length + 1), true);
            memset(MemoryImage, 0, Length);
            return {MemoryImage, 0x0};
        }

        Elf64_Phdr ItrPhdr;
        uintptr_t FirstProgramHeaderVirtualAddress = 0x0;

        bool FirstProgramHeader = false;
        for (Elf64_Half i = 0; i < ELFHeader->e_phnum; i++)
        {
            memcpy(&ItrPhdr,
                   (uint8_t *)ElfFile + ELFHeader->e_phoff + ELFHeader->e_phentsize * i,
                   sizeof(Elf64_Phdr));

            if (ItrPhdr.p_type == PT_LOAD && !FirstProgramHeader)
            {
                FirstProgramHeaderVirtualAddress = ItrPhdr.p_vaddr;
                FirstProgramHeader = true;
            }

            if (ItrPhdr.p_type == PT_LOAD && ItrPhdr.p_vaddr == 0)
            {
                debug("p_vaddr is 0, allocating %ld pages for image (size: %#lx)", TO_PAGES(Length), Length);
                MemoryImage = mem->RequestPages(TO_PAGES(Length), true);
                debug("MemoryImage: %#lx-%#lx", MemoryImage, (uintptr_t)MemoryImage + Length);
                memset(MemoryImage, 0, Length);
                return {MemoryImage, (void *)FirstProgramHeaderVirtualAddress};
            }
        }

        debug("Allocating %ld pages for image (size: %#lx)", TO_PAGES(Length), Length);
        MemoryImage = mem->RequestPages(TO_PAGES(Length));
        debug("MemoryImage: %#lx-%#lx", MemoryImage, (uintptr_t)MemoryImage + Length);
        memset(MemoryImage, 0, Length);

        if (FirstProgramHeaderVirtualAddress != 0)
            FirstProgramHeaderVirtualAddress &= 0xFFFFFFFFFFFFF000;
        else
            FirstProgramHeaderVirtualAddress = (uintptr_t)MemoryImage;

        for (size_t i = 0; i < TO_PAGES(Length); i++)
        {
            pV.Remap((void *)((uintptr_t)FirstProgramHeaderVirtualAddress + (i * PAGE_SIZE)), (void *)((uintptr_t)MemoryImage + (i * PAGE_SIZE)), Memory::PTFlag::RW | Memory::PTFlag::US);
            debug("Remapped: %#lx -> %#lx", (uintptr_t)FirstProgramHeaderVirtualAddress + (i * PAGE_SIZE), (uintptr_t)MemoryImage + (i * PAGE_SIZE));
        }
        return {MemoryImage, (void *)FirstProgramHeaderVirtualAddress};
    }

    uintptr_t LoadELFInterpreter(Memory::MemMgr *mem, Memory::Virtual &pV, const char *Interpreter)
    {
        if (GetBinaryType((char *)Interpreter) != BinaryType::BinTypeELF)
        {
            error("Interpreter \"%s\" is not an ELF file.", Interpreter);
            return 0;
        }

        /* No need to check if it's valid, the GetBinaryType() call above does that. */
        std::shared_ptr<VirtualFileSystem::File> File = vfs->Open(Interpreter);

        Elf64_Ehdr *ELFHeader = (Elf64_Ehdr *)File->node->Address;

#ifdef DEBUG
        const char *InterpreterType[6] = {
            "ET_NONE",
            "ET_REL",
            "ET_EXEC",
            "ET_DYN",
            "ET_CORE",
            "ET_LOPROC - ET_HIPROC"};
        Elf64_Half IntType = ELFHeader->e_type;
        if (IntType > 5)
            IntType = 5;
        debug("Interpreter type: %s - %#x", InterpreterType[IntType], ELFHeader->e_type);
#endif

        uintptr_t BaseAddress = UINTPTR_MAX;
        uint64_t ElfAppSize = 0;

        Elf64_Phdr ItrPhdr;

        /* Get base address */
        for (Elf64_Half i = 0; i < ELFHeader->e_phnum; i++)
        {
            memcpy(&ItrPhdr,
                   (uint8_t *)File->node->Address + ELFHeader->e_phoff + ELFHeader->e_phentsize * i,
                   sizeof(Elf64_Phdr));

            BaseAddress = MIN(BaseAddress, ItrPhdr.p_vaddr);
        }

        /* Get size */
        for (Elf64_Half i = 0; i < ELFHeader->e_phnum; i++)
        {
            memcpy(&ItrPhdr,
                   (uint8_t *)File->node->Address + ELFHeader->e_phoff + ELFHeader->e_phentsize * i,
                   sizeof(Elf64_Phdr));

            uintptr_t SegmentEnd;
            SegmentEnd = ItrPhdr.p_vaddr - BaseAddress + ItrPhdr.p_memsz;
            ElfAppSize = MAX(ElfAppSize, SegmentEnd);
        }

        MmImage MemoryImage = ELFCreateMemoryImage(mem, pV, (void *)File->node->Address, ElfAppSize);

        for (Elf64_Half i = 0; i < ELFHeader->e_phnum; i++)
        {
            memcpy(&ItrPhdr,
                   (uint8_t *)File->node->Address + ELFHeader->e_phoff + ELFHeader->e_phentsize * i,
                   sizeof(Elf64_Phdr));

            if (ItrPhdr.p_type == PT_LOAD)
            {
                debug("PT_LOAD - Offset: %#lx, VirtAddr: %#lx, FileSiz: %ld, MemSiz: %ld, Align: %#lx",
                      ItrPhdr.p_offset, ItrPhdr.p_vaddr,
                      ItrPhdr.p_filesz, ItrPhdr.p_memsz, ItrPhdr.p_align);
                uintptr_t MAddr = (ItrPhdr.p_vaddr - BaseAddress) + (uintptr_t)MemoryImage.Phyiscal;
                fixme("Address: %#lx %s%s%s", MAddr,
                      (ItrPhdr.p_flags & PF_R) ? "R" : "",
                      (ItrPhdr.p_flags & PF_W) ? "W" : "",
                      (ItrPhdr.p_flags & PF_X) ? "X" : "");

                memcpy((void *)MAddr, (uint8_t *)File->node->Address + ItrPhdr.p_offset, ItrPhdr.p_filesz);
                debug("memcpy: %#lx => %#lx (%ld bytes)", (uint8_t *)File->node->Address + ItrPhdr.p_offset, MAddr, ItrPhdr.p_filesz);
            }
        }

        vfs->Close(File);
        debug("Interpreter entry point: %#lx (%#lx + %#lx)", (uintptr_t)MemoryImage.Phyiscal + ELFHeader->e_entry,
              (uintptr_t)MemoryImage.Phyiscal, ELFHeader->e_entry);
        return (uintptr_t)MemoryImage.Phyiscal + ELFHeader->e_entry;
    }
}
