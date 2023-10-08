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

#ifndef __FENNIX_KERNEL_MEMORY_SMART_HEAP_H__
#define __FENNIX_KERNEL_MEMORY_SMART_HEAP_H__

#include <types.h>
#include <memory/vma.hpp>
#include <convert.h>
#include <cstddef>

namespace Memory
{
	class SmartHeap
	{
	private:
		VirtualMemoryArea *vma = nullptr;
		void *Object = nullptr;
		std::size_t ObjectSize = 0;

	public:
		auto Get() { return Object; }
		SmartHeap(std::size_t Size,
				  VirtualMemoryArea *vma = nullptr);
		~SmartHeap();

		void *operator->() { return Object; }
		void *operator*() { return Object; }
		operator void *() { return Object; }

		operator const char *()
		{
			return r_cst(const char *, Object);
		}

		operator uintptr_t()
		{
			return r_cst(uintptr_t, Object);
		}

		operator uint8_t *()
		{
			return r_cst(uint8_t *, Object);
		}

		void operator=(void *Address)
		{
			memcpy(Object, Address, ObjectSize);
		}
	};
}

#endif // !__FENNIX_KERNEL_MEMORY_SMART_HEAP_H__
