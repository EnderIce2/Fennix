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

#include <memory.hpp>

#include "../../kernel.h"

namespace Memory
{
	KernelStackManager::StackAllocation KernelStackManager::DetailedAllocate(size_t Size)
	{
		SmartLock(StackLock);
		Size += 0x10;

		size_t pagesNeeded = TO_PAGES(Size);
		size_t stackSize = pagesNeeded * PAGE_SIZE;

		assert((CurrentStackTop - stackSize) > KERNEL_STACK_BASE);

		void *physicalMemory = KernelAllocator.RequestPages(pagesNeeded);
		void *virtualAddress = (void *)(CurrentStackTop - stackSize);

		Memory::Virtual vmm(KernelPageTable);
		vmm.Map(virtualAddress, physicalMemory, stackSize, Memory::RW | Memory::G);

		AllocatedStacks.push_back({physicalMemory, virtualAddress, stackSize});
		CurrentStackTop -= stackSize;
		TotalSize += stackSize;
		return {physicalMemory, virtualAddress, stackSize};
	}

	void *KernelStackManager::Allocate(size_t Size)
	{
		return this->DetailedAllocate(Size).VirtualAddress;
	}

	void KernelStackManager::Free(void *Address)
	{
		SmartLock(StackLock);

		auto it = std::find_if(AllocatedStacks.begin(), AllocatedStacks.end(),
							   [Address](const StackAllocation &stack)
							   {
								   return stack.VirtualAddress == Address;
							   });

		if (it == AllocatedStacks.end())
			return;

		size_t pagesToFree = TO_PAGES(it->Size);
		Memory::Virtual vmm(KernelPageTable);
		vmm.Unmap(Address, it->Size);
		KernelAllocator.FreePages(it->PhysicalAddress, pagesToFree);

		TotalSize -= it->Size;
		AllocatedStacks.erase(it);
	}

	KernelStackManager::KernelStackManager() {}
	KernelStackManager::~KernelStackManager() {}
}
