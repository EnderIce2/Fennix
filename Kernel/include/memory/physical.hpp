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

#ifndef __FENNIX_KERNEL_MEMORY_PHYSICAL_H__
#define __FENNIX_KERNEL_MEMORY_PHYSICAL_H__

#include <types.h>

#include <bitmap.hpp>
#include <lock.hpp>

namespace Memory
{
	class Physical
	{
	private:
		NewLock(MemoryLock);

		std::atomic_uint64_t TotalMemory = 0;
		std::atomic_uint64_t FreeMemory = 0;
		std::atomic_uint64_t ReservedMemory = 0;
		std::atomic_uint64_t UsedMemory = 0;
		size_t PageBitmapIndex = 0;
		Bitmap PageBitmap;

		void ReserveEssentials();
		void FindBitmapRegion(uintptr_t &BitmapAddress, size_t &BitmapAddressSize);

		void LockPage(fnx::void_t Address);
		void LockPages(fnx::void_t Address, size_t PageCount);

		void (*OutOfMemoryHandler)() = nullptr;

		friend class DMA;

	public:
		Bitmap GetPageBitmap() { return PageBitmap; }

		void SetOutOfMemoryHandler(void (*Handler)()) { OutOfMemoryHandler = Handler; }

		/**
		 * @brief Get Total Memory
		 *
		 * @return uint64_t
		 */
		uint64_t GetTotalMemory();

		/**
		 * @brief Get Free Memory
		 *
		 * @return uint64_t
		 */
		uint64_t GetFreeMemory();

		/**
		 * @brief Get Reserved Memory
		 *
		 * @return uint64_t
		 */
		uint64_t GetReservedMemory();

		/**
		 * @brief Get Used Memory
		 *
		 * @return uint64_t
		 */
		uint64_t GetUsedMemory();

		void ReservePage(fnx::void_t Address);
		void ReservePages(fnx::void_t Address, size_t PageCount);
		void UnreservePage(fnx::void_t Address);
		void UnreservePages(fnx::void_t Address, size_t PageCount);

		/**
		 * @brief Request page using Next-Fit algorithm
		 *
		 * @param FirstFit If true, use First-Fit algorithm instead of Next-Fit
		 * @return void* Allocated page address
		 */
		fnx::void_t RequestPage(bool FirstFit = false);

		/**
		 * @brief Request pages using Next-Fit algorithm
		 *
		 * @param PageCount Number of pages
		 * @param FirstFit If true, use First-Fit algorithm instead of Next-Fit
		 * @return void* Allocated pages address
		 */
		fnx::void_t RequestPages(std::size_t Count, bool FirstFit = false);

		/**
		 * @brief Free page
		 *
		 * @param Address Address of the page
		 */
		void FreePage(fnx::void_t Address);

		/**
		 * @brief Free pages
		 *
		 * @param Address Address of the pages
		 * @param PageCount Number of pages
		 */
		void FreePages(fnx::void_t Address, size_t Count);

		void Init();
		Physical() = default;
		~Physical() = default;
	};
}

#endif // !__FENNIX_KERNEL_MEMORY_PHYSICAL_H__
