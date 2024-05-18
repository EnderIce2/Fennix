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

#ifndef __FENNIX_KERNEL_MEMORY_PROGRAM_BREAK_H__
#define __FENNIX_KERNEL_MEMORY_PROGRAM_BREAK_H__

#include <types.h>
#include <debug.h>

#include <memory/table.hpp>
#include <memory/vma.hpp>

namespace Memory
{
	class ProgramBreak
	{
	private:
		PageTable *Table = nullptr;
		VirtualMemoryArea *vma = nullptr;

		uintptr_t HeapStart = 0x0;
		uintptr_t Break = 0x0;

	public:
		/* fork() */
		void SetTable(PageTable *Table) { this->Table = Table; }

		/* Directly to syscall */
		void *brk(void *Address);

		void InitBrk(uintptr_t Address)
		{
			func("%#lx", Address);
			HeapStart = Address;
			Break = Address;
		}

		ProgramBreak(PageTable *Table, VirtualMemoryArea *mm);
		~ProgramBreak();
	};
}

#endif // !__FENNIX_KERNEL_MEMORY_PROGRAM_BREAK_H__
