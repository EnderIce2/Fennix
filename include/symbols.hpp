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
#include <vector>

namespace SymbolResolver
{
	class Symbols
	{
	private:
		struct SymbolTable
		{
			uintptr_t Address = 0;
			char *FunctionName = (char *)"<unknown>";
		};

		std::vector<SymbolTable> SymTable;
		void *Image;
		bool SymbolTableExists = false;

	public:
		decltype(SymbolTableExists) &SymTableExists = this->SymbolTableExists;
		void *GetImage() { return this->Image; }
		const char *GetSymbolFromAddress(uintptr_t Address);
		void AddSymbol(uintptr_t Address, const char *Name);
		void AddSymbolInfoFromGRUB(uint64_t Num, uint64_t EntSize, uint64_t Shndx, uintptr_t Sections);
		void AppendSymbols(uintptr_t ImageAddress, uintptr_t BaseAddress = 0);
		Symbols(uintptr_t ImageAddress);
		~Symbols();
	};
}
