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
		uint64_t PageBitmapIndex = 0;
		Bitmap PageBitmap;

		void ReserveEssentials();
		void FindBitmapRegion(uintptr_t &BitmapAddress,
							  size_t &BitmapAddressSize);

	public:
		Bitmap GetPageBitmap() { return PageBitmap; }

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

		/**
		 * @brief Swap page
		 *
		 * @param Address Address of the page
		 * @return true if swap was successful
		 * @return false if swap was unsuccessful
		 */
		bool SwapPage(void *Address);

		/**
		 * @brief Swap pages
		 *
		 * @param Address Address of the pages
		 * @param PageCount Number of pages
		 * @return true if swap was successful
		 * @return false if swap was unsuccessful
		 */
		bool SwapPages(void *Address, size_t PageCount);

		/**
		 * @brief Unswap page
		 *
		 * @param Address Address of the page
		 * @return true if unswap was successful
		 * @return false if unswap was unsuccessful
		 */
		bool UnswapPage(void *Address);

		/**
		 * @brief Unswap pages
		 *
		 * @param Address Address of the pages
		 * @param PageCount Number of pages
		 * @return true if unswap was successful
		 * @return false if unswap was unsuccessful
		 */
		bool UnswapPages(void *Address, size_t PageCount);

		/**
		 * @brief Lock page
		 *
		 * @param Address Address of the page
		 */
		void LockPage(void *Address);

		/**
		 * @brief Lock pages
		 *
		 * @param Address Address of the pages
		 * @param PageCount Number of pages
		 */
		void LockPages(void *Address, size_t PageCount);

		void ReservePage(void *Address);
		void ReservePages(void *Address, size_t PageCount);
		void UnreservePage(void *Address);
		void UnreservePages(void *Address, size_t PageCount);

		/**
		 * @brief Request page
		 *
		 * @return void* Allocated page address
		 */
		void *RequestPage();

		/**
		 * @brief Request pages
		 *
		 * @param PageCount Number of pages
		 * @return void* Allocated pages address
		 */
		void *RequestPages(std::size_t Count);

		/**
		 * @brief Free page
		 *
		 * @param Address Address of the page
		 */
		void FreePage(void *Address);

		/**
		 * @brief Free pages
		 *
		 * @param Address Address of the pages
		 * @param PageCount Number of pages
		 */
		void FreePages(void *Address, size_t Count);

		/** @brief Do not use. */
		void Init();

		/** @brief Do not use. */
		Physical();

		/** @brief Do not use. */
		~Physical();
	};
}

#endif // !__FENNIX_KERNEL_MEMORY_PHYSICAL_H__
