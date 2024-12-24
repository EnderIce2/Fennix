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

#ifndef __FENNIX_KERNEL_MEMORY_STACK_GUARD_H__
#define __FENNIX_KERNEL_MEMORY_STACK_GUARD_H__

#include <types.h>
#include <list>

#include <memory/table.hpp>
#include <memory/vma.hpp>

namespace Memory
{
	class StackGuard
	{
	private:
		struct AllocatedPages
		{
			void *PhysicalAddress;
			void *VirtualAddress;
		};

		void *StackBottom = nullptr;
		void *StackTop = nullptr;
		void *StackPhysicalBottom = nullptr;
		void *StackPhysicalTop = nullptr;
		uint64_t CurrentSize = 0;
		bool UserMode = false;
		bool Expanded = false;
		VirtualMemoryArea *vma = nullptr;
		std::list<AllocatedPages> AllocatedPagesList;

	public:
		std::list<AllocatedPages> GetAllocatedPages()
		{
			return AllocatedPagesList;
		}

		/** Fork stack guard */
		void Fork(StackGuard *Parent);

		/** For general info */
		uint64_t GetSize() { return CurrentSize; }

		/** For general info */
		bool GetUserMode() { return UserMode; }

		/** For general info */
		bool IsExpanded() { return Expanded; }

		/** For general info */
		void *GetStackBottom() { return StackBottom; }

		/** For RSP */
		void *GetStackTop() { return StackTop; }

		/**
		 * For general info (avoid if possible)
		 *
		 * @note This can be used only if the
		 * stack was NOT expanded.
		 */
		void *GetStackPhysicalBottom()
		{
			if (Expanded)
				return nullptr;
			return StackPhysicalBottom;
		}

		/**
		 * For general info (avoid if possible)
		 *
		 * @note This can be used only if the
		 * stack was NOT expanded.
		 */
		void *GetStackPhysicalTop()
		{
			if (Expanded)
				return nullptr;
			return StackPhysicalTop;
		}

		/**
		 * Called by exception handler */
		bool Expand(uintptr_t FaultAddress);

		/**
		 * Construct a new Stack Guard object
		 * @param User Stack for user mode?
		 */
		StackGuard(bool User, VirtualMemoryArea *vma);

		/**
		 * Destroy the Stack Guard object
		 */
		~StackGuard();
	};
}

#endif // !__FENNIX_KERNEL_MEMORY_STACK_GUARD_H__
