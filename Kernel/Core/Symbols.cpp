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

#include <symbols.hpp>
#include <memory.hpp>
#include <convert.h>
#include <debug.h>
#include <elf.h>

// #pragma GCC diagnostic ignored "-Wignored-qualifiers"

namespace SymbolResolver
{
    Symbols::Symbols(uintptr_t ImageAddress)
    {
        debug("Solving symbols for address: %#llx", ImageAddress);

        if (ImageAddress == 0)
        {
            error("Invalid image address");
            return;
        }

        Elf64_Ehdr *Header = (Elf64_Ehdr *)ImageAddress;
        if (Header->e_ident[0] != 0x7F &&
            Header->e_ident[1] != 'E' &&
            Header->e_ident[2] != 'L' &&
            Header->e_ident[3] != 'F')
        {
            error("Invalid ELF header");
            return;
        }
        Elf64_Shdr *ElfSections = (Elf64_Shdr *)(ImageAddress + Header->e_shoff);
        Elf64_Sym *ElfSymbols = nullptr;
        char *strtab = nullptr;

        for (uint16_t i = 0; i < Header->e_shnum; i++)
            switch (ElfSections[i].sh_type)
            {
            case SHT_SYMTAB:
                ElfSymbols = (Elf64_Sym *)(ImageAddress + ElfSections[i].sh_offset);
                this->TotalEntries = ElfSections[i].sh_size / sizeof(Elf64_Sym);
                if (this->TotalEntries >= 0x10000)
                    this->TotalEntries = 0x10000 - 1;

                debug("Symbol table found, %d entries", this->TotalEntries);
                break;
            case SHT_STRTAB:
                if (Header->e_shstrndx == i)
                {
                    debug("String table found, %d entries", ElfSections[i].sh_size);
                }
                else
                {
                    strtab = (char *)(ImageAddress + ElfSections[i].sh_offset);
                    debug("String table found, %d entries", ElfSections[i].sh_size);
                }
                break;
            default:
                break;
            }

        if (ElfSymbols != nullptr && strtab != nullptr)
        {
            uintptr_t Index, MinimumIndex;
            for (uintptr_t i = 0; i < this->TotalEntries - 1; i++)
            {
                MinimumIndex = i;
                for (Index = i + 1; Index < this->TotalEntries; Index++)
                    if (ElfSymbols[Index].st_value < ElfSymbols[MinimumIndex].st_value)
                        MinimumIndex = Index;
                Elf64_Sym tmp = ElfSymbols[MinimumIndex];
                ElfSymbols[MinimumIndex] = ElfSymbols[i];
                ElfSymbols[i] = tmp;
            }

            while (ElfSymbols[0].st_value == 0)
            {
                ElfSymbols++;
                this->TotalEntries--;
            }

#ifdef DEBUG
            static int once = 0;
#endif

            trace("Symbol table loaded, %d entries (%ldKB)", this->TotalEntries, TO_KB(this->TotalEntries * sizeof(SymbolTable)));
            for (uintptr_t i = 0, g = this->TotalEntries; i < g; i++)
            {
                this->SymTable[i].Address = ElfSymbols[i].st_value;
                this->SymTable[i].FunctionName = &strtab[ElfSymbols[i].st_name];
#ifdef DEBUG
                if (once)
                    debug("Symbol %d: %#llx %s", i, this->SymTable[i].Address, this->SymTable[i].FunctionName);
#endif
            }

#ifdef DEBUG
            if (!once)
                once++;
#endif
        }
    }

    Symbols::~Symbols() {}

    const NIF char *Symbols::GetSymbolFromAddress(uintptr_t Address)
    {
        Symbols::SymbolTable Result{0, (char *)"<unknown>"};
        for (uintptr_t i = 0; i < this->TotalEntries; i++)
            if (this->SymTable[i].Address <= Address && this->SymTable[i].Address > Result.Address)
                Result = this->SymTable[i];
        return Result.FunctionName;
    }

    NIF void Symbols::AddSymbol(uintptr_t Address, const char *Name)
    {
        if (this->TotalEntries >= 0x10000)
        {
            error("Symbol table is full");
            return;
        }

        this->SymTable[this->TotalEntries].Address = Address;
        strcpy(this->SymTable[this->TotalEntries].FunctionName, Name);
        this->TotalEntries++;
    }
}
