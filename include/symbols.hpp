#pragma once
#include <types.h>

namespace SymbolResolver
{
    class Symbols
    {
    private:
        struct SymbolTable
        {
            uint64_t Address;
            char *FunctionName;
        };

        SymbolTable SymTable[0x10000];
        uint64_t TotalEntries = 0;

    public:
        Symbols(uint64_t ImageAddress);
        ~Symbols();
        const char *GetSymbolFromAddress(uint64_t Address);
    };
}

extern SymbolResolver::Symbols *SymTbl;
