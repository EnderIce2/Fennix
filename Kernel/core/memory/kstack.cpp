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
		size_t stackSize = FROM_PAGES(pagesNeeded);

		assert((CurrentStackTop - stackSize) > KERNEL_STACK_BASE);

		fnx::void_t physicalAddress = KernelAllocator.RequestPages(pagesNeeded);
		fnx::void_t virtualAddress = CurrentStackTop - stackSize;

		Memory::Virtual vmm(KernelPageTable);
		vmm.Map(virtualAddress, physicalAddress, stackSize, Memory::RW | Memory::G);

		AllocatedStacks.push_back({physicalAddress, virtualAddress, stackSize});
		CurrentStackTop -= stackSize;
		TotalSize += stackSize;

		debug("new stack: p:%#lx v:%#lx size:%#lx(req:%#lx) total: %d KiB", physicalAddress.get(), virtualAddress.get(), stackSize, Size, TO_KiB(TotalSize));
		return {physicalAddress, virtualAddress, stackSize};
	}

	void KernelStackManager::Free(fnx::void_t Address)
	{
		SmartLock(StackLock);

		auto it = std::find_if(AllocatedStacks.begin(), AllocatedStacks.end(),
							   [Address](const StackAllocation &stack)
							   {
								   return stack.VirtualAddress == Address;
							   });

		if (it == AllocatedStacks.end())
			return;

		Memory::Virtual(KernelPageTable).Unmap(Address, it->Size);
		KernelAllocator.FreePages(it->PhysicalAddress, TO_PAGES(it->Size));

		/* oh uhhhh... CurrentStackTop? */
		TotalSize -= it->Size;
		AllocatedStacks.erase(it);

		debug("freed stack at v:%#lx, p:%#lx, size:%#lx total size: %d KiB",
			  it->VirtualAddress.get(), it->PhysicalAddress.get(), it->Size, TO_KiB(TotalSize));
	}
}
