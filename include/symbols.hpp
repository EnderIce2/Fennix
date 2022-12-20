#pragma once
#include <types.h>

namespace SymbolResolver
{
    class Symbols
    {
    private:
        struct SymbolTable
        {
            uintptr_t Address;
            char *FunctionName;
        };

        SymbolTable SymTable[0x10000];
        uintptr_t TotalEntries = 0;

    public:
        Symbols(uintptr_t ImageAddress);
        ~Symbols();
        const char *GetSymbolFromAddress(uintptr_t Address);
    };
}

extern SymbolResolver::Symbols *SymTbl;
