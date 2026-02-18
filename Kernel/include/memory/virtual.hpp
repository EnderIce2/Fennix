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

#ifndef __FENNIX_KERNEL_MEMORY_VIRTUAL_H__
#define __FENNIX_KERNEL_MEMORY_VIRTUAL_H__

#include <types.h>
#include <lock.hpp>

#include <memory/table.hpp>
#include <memory/macro.hpp>

namespace Memory
{
	class Virtual
	{
	private:
		NewLock(MemoryLock);
		PageTable *pTable = nullptr;

	public:
		enum MapType
		{
			NoMapType,
			FourKiB,
			TwoMiB,
			FourMiB,
			OneGiB
		};

		class PageMapIndexer
		{
		public:
#if defined(__amd64__)
			uintptr_t PMLIndex = 0;
			uintptr_t PDPTEIndex = 0;
#endif
			uintptr_t PDEIndex = 0;
			uintptr_t PTEIndex = 0;
			PageMapIndexer(uintptr_t VirtualAddress);
		};

		/**
		 * @brief Check if page has the specified flag.
		 *
		 * @param VirtualAddress Virtual address of the page
		 * @param Flag Flag to check
		 * @return true if page has the specified flag, false otherwise.
		 */
		bool Check(fnx::void_t VirtualAddress, PTFlag Flag = PTFlag::P, MapType Type = MapType::FourKiB);

		/**
		 * @brief Check if the region has the specified flag.
		 *
		 * @param VirtualAddress Virtual address of the region.
		 * @param Length Length of the region.
		 * @param Flag Flag to check.
		 * @return true if the region has the specified flag, false otherwise.
		 */
		bool CheckRegion(fnx::void_t VirtualAddress, size_t Length, PTFlag Flag = PTFlag::P)
		{
			for (size_t i = 0; i < Length; i += PAGE_SIZE_4K)
			{
				if (!this->Check(VirtualAddress + i, Flag))
					return false;
			}
			return true;
		}

		/**
		 * @brief Get physical address of the page.
		 * @param VirtualAddress Virtual address of the page.
		 * @return Physical address of the page.
		 */
		fnx::void_t GetPhysical(fnx::void_t VirtualAddress);

		/**
		 * @brief Get map type of the page.
		 * @param VirtualAddress Virtual address of the page.
		 * @return Map type of the page.
		 */
		MapType GetMapType(fnx::void_t VirtualAddress);

#ifdef __amd64__
		PageMapLevel5 *GetPML5(fnx::void_t VirtualAddress, MapType Type = MapType::FourKiB);
		PageMapLevel4 *GetPML4(fnx::void_t VirtualAddress, MapType Type = MapType::FourKiB);
		PageDirectoryPointerTableEntry *GetPDPTE(fnx::void_t VirtualAddress, MapType Type = MapType::FourKiB);
#endif /* __amd64__ */
		PageDirectoryEntry *GetPDE(fnx::void_t VirtualAddress, MapType Type = MapType::FourKiB);
		PageTableEntry *GetPTE(fnx::void_t VirtualAddress, MapType Type = MapType::FourKiB);

		/**
		 * @brief Map a single page.
		 *
		 * @param VirtualAddress Virtual address of the page
		 * @param PhysicalAddress Physical address of the page
		 * @param Flags Flags of the page. Check PTFlag enum
		 * @param Type Type of the page. Check MapType enum
		 */
		void SingleMap(fnx::void_t VirtualAddress, fnx::void_t PhysicalAddress,
					   uint64_t Flag = PTFlag::P, MapType Type = MapType::FourKiB);

		/**
		 * @brief Map memory region.
		 *
		 * @param VirtualAddress First virtual address of the page
		 * @param PhysicalAddress First physical address of the page
		 * @param Length Length in bytes to map
		 * @param Flags PTFlag enum
		 */
		void Map(fnx::void_t VirtualAddress, fnx::void_t PhysicalAddress, size_t Length, uint64_t Flags)
		{
			for (uintptr_t i = 0; i < Length; i += PAGE_SIZE_4K)
			{
				this->SingleMap(VirtualAddress + i, PhysicalAddress + i, Flags);
			}
		}

		/**
		 * @brief Map multiple pages efficiently.
		 *
		 * This function will detect the best page size to map the pages.
		 *
		 * @param VirtualAddress First virtual address of the page.
		 * @param PhysicalAddress First physical address of the page.
		 * @param Length Length in bytes to map.
		 * @param Flags Flags of the page. Check PTFlag enum.
		 * @param FailOnModulo If true, the function will return NoMapType if the length is not a multiple of the page size.
		 */
		void OptimizedMap(fnx::void_t VirtualAddress, fnx::void_t PhysicalAddress, size_t Length, uint64_t Flags);

		/**
		 * @brief Unmap page.
		 *
		 * @param VirtualAddress Virtual address of the page.
		 * @param Type Type of the page. Check MapType enum.
		 */
		void Unmap(fnx::void_t VirtualAddress, MapType Type = MapType::FourKiB);

		/**
		 * @brief Unmap multiple pages.
		 *
		 * @param VirtualAddress First virtual address of the page.
		 * @param Length Length to map.
		 * @param Type Type of the page. Check MapType enum.
		 */
		inline void Unmap(fnx::void_t VirtualAddress, size_t Length, MapType Type = MapType::FourKiB)
		{
			int PageSize = PAGE_SIZE_4K;

			if (Type == MapType::TwoMiB)
				PageSize = PAGE_SIZE_2M;
			else if (Type == MapType::FourMiB)
				PageSize = PAGE_SIZE_4M;
			else if (Type == MapType::OneGiB)
				PageSize = PAGE_SIZE_1G;

			for (uintptr_t i = 0; i < Length; i += PageSize)
				this->Unmap(VirtualAddress + i, Type);
		}

		/**
		 * @brief Remap page.
		 *
		 * @param VirtualAddress Virtual address of the page.
		 * @param PhysicalAddress Physical address of the page.
		 * @param Flags Flags of the page. Check PTFlag enum.
		 * @param Type Type of the page. Check MapType enum.
		 */
		void Remap(fnx::void_t VirtualAddress, fnx::void_t PhysicalAddress, uint64_t Flags, MapType Type = MapType::FourKiB);

		/**
		 * @brief Construct a new Virtual object
		 *
		 * @param Table Page table. If null, it will use the current page table.
		 */
		Virtual(PageTable *Table = nullptr);

		/**
		 * @brief Destroy the Virtual object
		 *
		 */
		~Virtual();
	};
}

#endif // !__FENNIX_KERNEL_MEMORY_VIRTUAL_H__
