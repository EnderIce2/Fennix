#include <symbols.hpp>
#include <memory.hpp>
#include <string.h>
#include <debug.h>

// #pragma GCC diagnostic ignored "-Wignored-qualifiers"

typedef struct
{
    unsigned char e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf64_Ehdr;

typedef struct
{
    uint32_t sh_name;
    uint32_t sh_type;
    uint64_t sh_flags;
    uint64_t sh_addr;
    uint64_t sh_offset;
    uint64_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint64_t sh_addralign;
    uint64_t sh_entsize;
} Elf64_Shdr;

typedef struct
{
    uint32_t st_name;
    unsigned char st_info;
    unsigned char st_other;
    uint16_t st_shndx;
    uint64_t st_value;
    uint64_t st_size;
} Elf64_Sym;

#define SHT_SYMTAB 2
#define SHT_STRTAB 3

namespace SymbolResolver
{
    Symbols::SymbolTable *SymTable = nullptr;
    uint64_t TotalEntries = 0;

    Symbols::Symbols(uint64_t Address)
    {
        debug("Solving symbols for address: %#lx", Address);
        Elf64_Ehdr *Header = (Elf64_Ehdr *)Address;
        if (Header->e_ident[0] != 0x7F &&
            Header->e_ident[1] != 'E' &&
            Header->e_ident[2] != 'L' &&
            Header->e_ident[3] != 'F')
        {
            error("Invalid ELF header");
            return;
        }
        Elf64_Shdr *ElfSections = (Elf64_Shdr *)(Address + Header->e_shoff);
        Elf64_Sym *ElfSymbols = nullptr;
        char *strtab = nullptr;

        for (uint64_t i = 0; i < Header->e_shnum; i++)
            switch (ElfSections[i].sh_type)
            {
            case SHT_SYMTAB:
                ElfSymbols = (Elf64_Sym *)(Address + ElfSections[i].sh_offset);
                TotalEntries = ElfSections[i].sh_size / sizeof(Elf64_Sym);
                debug("Symbol table found, %d entries", TotalEntries);
                break;
            case SHT_STRTAB:
                if (Header->e_shstrndx == i)
                {
                    debug("String table found, %d entries", ElfSections[i].sh_size);
                }
                else
                {
                    strtab = (char *)Address + ElfSections[i].sh_offset;
                    debug("String table found, %d entries", ElfSections[i].sh_size);
                }
                break;
            }

        if (ElfSymbols != nullptr && strtab != nullptr)
        {
            size_t Index, MinimumIndex;
            for (size_t i = 0; i < TotalEntries - 1; i++)
            {
                MinimumIndex = i;
                for (Index = i + 1; Index < TotalEntries; Index++)
                    if (ElfSymbols[Index].st_value < ElfSymbols[MinimumIndex].st_value)
                        MinimumIndex = Index;
                Elf64_Sym tmp = ElfSymbols[MinimumIndex];
                ElfSymbols[MinimumIndex] = ElfSymbols[i];
                ElfSymbols[i] = tmp;
            }

            while (ElfSymbols[0].st_value == 0)
            {
                ElfSymbols++;
                TotalEntries--;
            }

            trace("Symbol table loaded, %d entries (%ldKB)", TotalEntries, TO_KB(TotalEntries * sizeof(SymbolTable)));
            // TODO: broken?
            // SymTable = new SymbolTable[TotalEntries];
            SymTable = (SymbolTable *)KernelAllocator.RequestPages((TotalEntries * sizeof(SymbolTable)) / PAGE_SIZE + 1);
            // do_mem_test();

            for (size_t i = 0, g = TotalEntries; i < g; i++)
            {
                SymTable[i].Address = ElfSymbols[i].st_value;
                SymTable[i].FunctionName = &strtab[ElfSymbols[i].st_name];
            }
        }
    }

    Symbols::~Symbols()
    {
        // delete SymTable;
        KernelAllocator.FreePages(SymTable, (TotalEntries * sizeof(SymbolTable)) / PAGE_SIZE + 1);
    }

    const char *Symbols::GetSymbolFromAddress(uint64_t Address)
    {
        Symbols::SymbolTable Result{0, (char *)"<unknown>"};
        for (size_t i = 0; i < TotalEntries; i++)
            if (SymTable[i].Address <= Address && SymTable[i].Address > Result.Address)
                Result = SymTable[i];
        return Result.FunctionName;
    }
}
