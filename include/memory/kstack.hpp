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

#ifndef __FENNIX_KERNEL_MEMORY_KERNEL_STACK_MANAGER_H__
#define __FENNIX_KERNEL_MEMORY_KERNEL_STACK_MANAGER_H__

#include <memory.hpp>

namespace Memory
{
	class KernelStackManager
	{
	public:
		struct StackAllocation
		{
			void *PhysicalAddress;
			void *VirtualAddress;
			size_t Size;
		};

	private:
		NewLock(StackLock);
		std::list<StackAllocation> AllocatedStacks;
		size_t TotalSize = 0;
		uintptr_t CurrentStackTop = KERNEL_STACK_END;

	public:
		/**
		 * Allocate a new stack with detailed information
		 *
		 * @param Size Size in bytes to allocate
		 * @return {PhysicalAddress, VirtualAddress, Size}
		 */
		StackAllocation DetailedAllocate(size_t Size);

		/**
		 * Allocate a new stack
		 *
		 * @param Size Size in bytes to allocate
		 * @return Pointer to the BASE of the stack
		 */
		void *Allocate(size_t Size);

		/**
		 * Free a previously allocated stack
		 *
		 * @param Address Virtual Address
		 */
		void Free(void *Address);
		KernelStackManager();
		~KernelStackManager();
	};
}

#endif // !__FENNIX_KERNEL_MEMORY_KERNEL_STACK_MANAGER_H__
