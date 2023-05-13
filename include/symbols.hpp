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
        int64_t TotalEntries = 0;
        void *Image;

    public:
        int64_t GetTotalEntries() { return this->TotalEntries; }
        void *GetImage() { return this->Image; }
        const char *GetSymbolFromAddress(uintptr_t Address);
        void AddSymbol(uintptr_t Address, const char *Name);
        void AddBySymbolInfo(uint64_t Num, uint64_t EntSize, uint64_t Shndx, uintptr_t Sections);
        Symbols(uintptr_t ImageAddress);
        ~Symbols();
    };
}
