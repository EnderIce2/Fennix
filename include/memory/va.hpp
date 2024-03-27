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

#ifndef __FENNIX_KERNEL_MEMORY_VA_H__
#define __FENNIX_KERNEL_MEMORY_VA_H__

#include <types.h>
#include <lock.hpp>
#include <list>

#include <memory/table.hpp>

namespace Memory
{
	class VirtualAllocation
	{
	public:
		struct AllocatedPages
		{
			void *PhysicalAddress;
			void *VirtualAddress;
			size_t PageCount;
			bool Free;
		};

	private:
		NewLock(MgrLock);

		std::list<AllocatedPages> AllocatedPagesList;
		void *BaseAddress;
		void *CurrentBase;

	public:
		PageTable *Table;

		AllocatedPages RequestPages(size_t Count);
		void FreePages(void *Address, size_t Count);

		void MapTo(AllocatedPages ap, PageTable *Table);

		VirtualAllocation(void *Base);
		~VirtualAllocation();
	};
}

#endif // !__FENNIX_KERNEL_MEMORY_VA_H__
