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

#ifndef __FENNIX_KERNEL_MEMORY_VMA_H__
#define __FENNIX_KERNEL_MEMORY_VMA_H__

#include <types.h>
#include <fs/vfs.hpp>
#include <bitmap.hpp>
#include <lock.hpp>
#include <list>

#include <memory/table.hpp>

namespace Memory
{

	class VirtualMemoryArea
	{
	public:
		struct AllocatedPages
		{
			void *Address;
			size_t PageCount;
			bool Protected;
		};

		struct SharedRegion
		{
			void *Address = nullptr;
			bool Read = 0, Write = 0, Exec = 0;
			bool Fixed = 0, Shared = 0;
			size_t Length = 0;
			size_t ReferenceCount = 0;
		};

	private:
		NewLock(MgrLock);
		Bitmap PageBitmap;

		std::list<AllocatedPages> AllocatedPagesList;
		std::list<SharedRegion> SharedRegions;

	public:
		PageTable *Table = nullptr;
		uint64_t GetAllocatedMemorySize();

		void *RequestPages(size_t Count, bool User = false, bool Protect = false);
		void FreePages(void *Address, size_t Count);
		void DetachAddress(void *Address);

		/**
		 * Create a Copy-on-Write region
		 *
		 * @param Address Hint address
		 * @param Length Length of the region
		 * @param Read Make the region readable
		 * @param Write Make the region writable
		 * @param Exec Make the region executable
		 * @param Fixed Fixed address
		 * @param Shared Shared region
		 * @return Address of the region
		 */
		void *CreateCoWRegion(void *Address, size_t Length,
							  bool Read, bool Write, bool Exec,
							  bool Fixed, bool Shared);

		bool HandleCoW(uintptr_t PFA);
		void FreeAllPages();
		void Fork(VirtualMemoryArea *Parent);

		void Reserve(void *Address, size_t Length);
		void Unreserve(void *Address, size_t Length);

		int Map(void *VirtualAddress, void *PhysicalAddress,
				size_t Length, uint64_t Flags);
		int Remap(void *VirtualAddress, void *PhysicalAddress, uint64_t Flags);
		int Unmap(void *VirtualAddress, size_t Length);
		void *__UserCheckAndGetAddress(void *Address, size_t Length);
		int __UserCheck(void *Address, size_t Length);

		template <typename T>
		T UserCheckAndGetAddress(T Address, size_t Length = 0)
		{
			if (Length == 0)
				Length = sizeof(T);

			void *PhysAddr = __UserCheckAndGetAddress((void *)Address, Length);
			if (PhysAddr == nullptr)
				return {};
			return T(PhysAddr);
		}

		template <typename T>
		int UserCheck(T Address, size_t Length = 0)
		{
			if (Length == 0)
				Length = sizeof(T);
			return __UserCheck((void *)Address, Length);
		}

		VirtualMemoryArea(PageTable *Table);
		~VirtualMemoryArea();
	};
}

#endif // !__FENNIX_KERNEL_MEMORY_VMA_H__
