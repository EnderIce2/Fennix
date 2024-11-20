#pragma once
#include <types.h>

namespace SymbolResolver
{
    class Symbols
    {
    public:
        struct SymbolTable
        {
            uint64_t Address;
            char *FunctionName;
        };

        Symbols(uint64_t Address);
        ~Symbols();
        const char *GetSymbolFromAddress(uint64_t Address);
    };
}

extern SymbolResolver::Symbols *SymTbl;
