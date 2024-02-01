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
#if defined(a64)
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
		 * @param Type Type of the page. Check MapType enum.
		 * @return true if page has the specified flag, false otherwise.
		 */
		bool Check(void *VirtualAddress,
				   PTFlag Flag = PTFlag::P,
				   MapType Type = MapType::FourKiB);

		/**
		 * @brief Check if the region has the specified flag.
		 *
		 * @param VirtualAddress Virtual address of the region.
		 * @param Length Length of the region.
		 * @param Flag Flag to check.
		 * @param Type Type of the page. Check MapType enum.
		 * @return true if the region has the specified flag, false otherwise.
		 */
		bool CheckRegion(void *VirtualAddress,
						 size_t Length,
						 PTFlag Flag = PTFlag::P,
						 MapType Type = MapType::FourKiB)
		{
			for (size_t i = 0; i < Length; i += PAGE_SIZE_4K)
			{
				if (!this->Check((void *)((uintptr_t)VirtualAddress + i), Flag, Type))
				{
					debug("address %#lx for pt %#lx has flag(s) %#lx",
						  (uintptr_t)VirtualAddress + i, this->pTable, Flag);
					return false;
				}
			}
			return true;
		}

		/**
		 * @brief Get physical address of the page.
		 * @param VirtualAddress Virtual address of the page.
		 * @return Physical address of the page.
		 */
		void *GetPhysical(void *VirtualAddress);

		/**
		 * @brief Get map type of the page.
		 * @param VirtualAddress Virtual address of the page.
		 * @return Map type of the page.
		 */
		MapType GetMapType(void *VirtualAddress);

#ifdef a64
		PageMapLevel5 *GetPML5(void *VirtualAddress, MapType Type = MapType::FourKiB);
		PageMapLevel4 *GetPML4(void *VirtualAddress, MapType Type = MapType::FourKiB);
		PageDirectoryPointerTableEntry *GetPDPTE(void *VirtualAddress, MapType Type = MapType::FourKiB);
#endif /* a64 */
		PageDirectoryEntry *GetPDE(void *VirtualAddress, MapType Type = MapType::FourKiB);
		PageTableEntry *GetPTE(void *VirtualAddress, MapType Type = MapType::FourKiB);

		/**
		 * @brief Map page.
		 *
		 * @param VirtualAddress Virtual address of the page.
		 * @param PhysicalAddress Physical address of the page.
		 * @param Flags Flags of the page. Check PTFlag enum.
		 * @param Type Type of the page. Check MapType enum.
		 */
		void Map(void *VirtualAddress,
				 void *PhysicalAddress,
				 uint64_t Flag = PTFlag::P,
				 MapType Type = MapType::FourKiB);

		/**
		 * @brief Map multiple pages.
		 *
		 * @param VirtualAddress First virtual address of the page.
		 * @param PhysicalAddress First physical address of the page.
		 * @param Length Length to map.
		 * @param Flags Flags of the page. Check PTFlag enum.
		 * @param Type Type of the page. Check MapType enum.
		 */
		__always_inline inline void Map(void *VirtualAddress,
										void *PhysicalAddress,
										size_t Length,
										uint64_t Flags,
										MapType Type = MapType::FourKiB)
		{
			int PageSize = PAGE_SIZE_4K;

			if (Type == MapType::TwoMiB)
				PageSize = PAGE_SIZE_2M;
			else if (Type == MapType::FourMiB)
				PageSize = PAGE_SIZE_4M;
			else if (Type == MapType::OneGiB)
				PageSize = PAGE_SIZE_1G;

			for (uintptr_t i = 0; i < Length; i += PageSize)
			{
				this->Map((void *)((uintptr_t)VirtualAddress + i),
						  (void *)((uintptr_t)PhysicalAddress + i),
						  Flags, Type);
			}
		}

		/**
		 * @brief Map multiple pages efficiently.
		 *
		 * This function will detect the best page size to map the pages.
		 *
		 * @note This function will not check if PSE or 1GB pages are enabled or supported.
		 *
		 * @param VirtualAddress First virtual address of the page.
		 * @param PhysicalAddress First physical address of the page.
		 * @param Length Length of the pages.
		 * @param Flags Flags of the page. Check PTFlag enum.
		 * @param Fit If true, the function will try to fit the pages in the smallest page size.
		 * @param FailOnModulo If true, the function will return NoMapType if the length is not a multiple of the page size.
		 * @return The best page size to map the pages.
		 */
		__always_inline inline MapType OptimizedMap(void *VirtualAddress,
													void *PhysicalAddress,
													size_t Length,
													uint64_t Flags,
													bool Fit = false,
													bool FailOnModulo = false)
		{
			if (unlikely(Fit))
			{
				while (Length >= PAGE_SIZE_1G)
				{
					this->Map(VirtualAddress, PhysicalAddress, Length, Flags, Virtual::MapType::OneGiB);
					VirtualAddress = (void *)((uintptr_t)VirtualAddress + PAGE_SIZE_1G);
					PhysicalAddress = (void *)((uintptr_t)PhysicalAddress + PAGE_SIZE_1G);
					Length -= PAGE_SIZE_1G;
				}

				while (Length >= PAGE_SIZE_4M)
				{
					this->Map(VirtualAddress, PhysicalAddress, Length, Flags, Virtual::MapType::FourMiB);
					VirtualAddress = (void *)((uintptr_t)VirtualAddress + PAGE_SIZE_4M);
					PhysicalAddress = (void *)((uintptr_t)PhysicalAddress + PAGE_SIZE_4M);
					Length -= PAGE_SIZE_4M;
				}

				while (Length >= PAGE_SIZE_2M)
				{
					this->Map(VirtualAddress, PhysicalAddress, Length, Flags, Virtual::MapType::TwoMiB);
					VirtualAddress = (void *)((uintptr_t)VirtualAddress + PAGE_SIZE_2M);
					PhysicalAddress = (void *)((uintptr_t)PhysicalAddress + PAGE_SIZE_2M);
					Length -= PAGE_SIZE_2M;
				}

				while (Length >= PAGE_SIZE_4K)
				{
					this->Map(VirtualAddress, PhysicalAddress, Length, Flags, Virtual::MapType::FourKiB);
					VirtualAddress = (void *)((uintptr_t)VirtualAddress + PAGE_SIZE_4K);
					PhysicalAddress = (void *)((uintptr_t)PhysicalAddress + PAGE_SIZE_4K);
					Length -= PAGE_SIZE_4K;
				}

				return Virtual::MapType::FourKiB;
			}

			Virtual::MapType Type = Virtual::MapType::FourKiB;

			if (Length >= PAGE_SIZE_1G)
			{
				Type = Virtual::MapType::OneGiB;
				if (Length % PAGE_SIZE_1G != 0)
				{
					warn("Length is not a multiple of 1GB.");
					if (FailOnModulo)
						return Virtual::MapType::NoMapType;
				}
			}
			else if (Length >= PAGE_SIZE_4M)
			{
				Type = Virtual::MapType::FourMiB;
				if (Length % PAGE_SIZE_4M != 0)
				{
					warn("Length is not a multiple of 4MB.");
					if (FailOnModulo)
						return Virtual::MapType::NoMapType;
				}
			}
			else if (Length >= PAGE_SIZE_2M)
			{
				Type = Virtual::MapType::TwoMiB;
				if (Length % PAGE_SIZE_2M != 0)
				{
					warn("Length is not a multiple of 2MB.");
					if (FailOnModulo)
						return Virtual::MapType::NoMapType;
				}
			}

			this->Map(VirtualAddress, PhysicalAddress, Length, Flags, Type);
			return Type;
		}

		/**
		 * @brief Unmap page.
		 *
		 * @param VirtualAddress Virtual address of the page.
		 * @param Type Type of the page. Check MapType enum.
		 */
		void Unmap(void *VirtualAddress, MapType Type = MapType::FourKiB);

		/**
		 * @brief Unmap multiple pages.
		 *
		 * @param VirtualAddress First virtual address of the page.
		 * @param Length Length to map.
		 * @param Type Type of the page. Check MapType enum.
		 */
		__always_inline inline void Unmap(void *VirtualAddress, size_t Length, MapType Type = MapType::FourKiB)
		{
			int PageSize = PAGE_SIZE_4K;

			if (Type == MapType::TwoMiB)
				PageSize = PAGE_SIZE_2M;
			else if (Type == MapType::FourMiB)
				PageSize = PAGE_SIZE_4M;
			else if (Type == MapType::OneGiB)
				PageSize = PAGE_SIZE_1G;

			for (uintptr_t i = 0; i < Length; i += PageSize)
				this->Unmap((void *)((uintptr_t)VirtualAddress + i), Type);
		}

		/**
		 * @brief Remap page.
		 *
		 * @param VirtualAddress Virtual address of the page.
		 * @param PhysicalAddress Physical address of the page.
		 * @param Flags Flags of the page. Check PTFlag enum.
		 * @param Type Type of the page. Check MapType enum.
		 */
		__always_inline inline void Remap(void *VirtualAddress, void *PhysicalAddress, uint64_t Flags, MapType Type = MapType::FourKiB)
		{
			this->Unmap(VirtualAddress, Type);
			this->Map(VirtualAddress, PhysicalAddress, Flags, Type);
		}

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
