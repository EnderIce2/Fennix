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

#ifndef __FENNIX_KERNEL_Xalloc_H__
#define __FENNIX_KERNEL_Xalloc_H__

#include <memory.hpp>
#include <lock.hpp>
#include <debug.h>

typedef __UINT8_TYPE__ Xuint8_t;
typedef __SIZE_TYPE__ Xsize_t;
typedef __UINTPTR_TYPE__ Xuintptr_t;

#define Xalloc_StopOnFail true
#define Xalloc_MapPages true
#define Xalloc_PAGE_SIZE PAGE_SIZE
#define Xalloc_trace(m, ...) trace(m, ##__VA_ARGS__)
#define Xalloc_warn(m, ...) warn(m, ##__VA_ARGS__)
#define Xalloc_err(m, ...) error(m, ##__VA_ARGS__)

#define XallocV1_def NewLock(XallocV1Lock)
#define XallocV1_lock XallocV1Lock.Lock(__FUNCTION__)
#define XallocV1_unlock XallocV1Lock.Unlock()

#define XallocV2_def NewLock(XallocV2Lock)
#define XallocV2_lock XallocV2Lock.Lock(__FUNCTION__)
#define XallocV2_unlock XallocV2Lock.Unlock()

namespace Xalloc
{
	class V1
	{
	private:
		void *BaseVirtualAddress = nullptr;
		void *FirstBlock = nullptr;
		void *LastBlock = nullptr;

		bool UserMapping = false;
		bool SMAPUsed = false;

	public:
		/** @brief Execute "stac" instruction if the kernel has SMAP enabled */
		void Xstac();

		/** @brief Execute "clac" instruction if the kernel has SMAP enabled */
		void Xclac();

		/**
		 * @brief Arrange the blocks to optimize the memory usage
		 * The allocator is not arranged by default
		 * to avoid performance issues.
		 * This function will defragment the memory
		 * and free the unused blocks.
		 *
		 * You should call this function when the
		 * kernel is idle or when is not using
		 * the allocator.
		 */
		void Arrange();

		/**
		 * @brief Allocate a new memory block
		 *
		 * @param Size Size of the block to allocate.
		 * @return void* Pointer to the allocated block.
		 */
		void *malloc(Xsize_t Size);

		/**
		 * @brief Free a previously allocated block
		 *
		 * @param Address Address of the block to free.
		 */
		void free(void *Address);

		/**
		 * @brief Allocate a new memory block
		 *
		 * @param NumberOfBlocks Number of blocks to allocate.
		 * @param Size Size of the block to allocate.
		 * @return void* Pointer to the allocated block.
		 */
		void *calloc(Xsize_t NumberOfBlocks, Xsize_t Size);

		/**
		 * @brief Reallocate a previously allocated block
		 *
		 * @param Address Address of the block to reallocate.
		 * @param Size New size of the block.
		 * @return void* Pointer to the reallocated block.
		 */
		void *realloc(void *Address, Xsize_t Size);

		/**
		 * @brief Construct a new Allocator object
		 *
		 * @param BaseVirtualAddress Virtual address to map the pages.
		 * @param UserMode Map the new pages with USER flag?
		 * @param SMAPEnabled Does the kernel has Supervisor Mode Access Prevention enabled?
		 */
		V1(void *BaseVirtualAddress, bool UserMode, bool SMAPEnabled);

		/**
		 * @brief Destroy the Allocator object
		 *
		 */
		~V1();
	};

	class V2
	{
	private:
		class Block
		{
		public:
			int Sanity = 0xA110C;
			Block *Next = nullptr;
			bool IsFree = true;
			V2 *ctx = nullptr;

			Xuint8_t *Data = nullptr;
			Xsize_t DataSize = 0;

			void Check();
			Block(Xsize_t Size, V2 *ctx);
			~Block();
			void *operator new(Xsize_t);
			void operator delete(void *Address);
		} __attribute__((packed, aligned((16))));

		/* The base address of the virtual memory */
		Xuintptr_t BaseVirtualAddress = 0x0;

		/* The size of the heap */
		Xsize_t HeapSize = 0x0;

		/* The used size of the heap */
		Xsize_t HeapUsed = 0x0;

		Block *FirstBlock = nullptr;

		Xuint8_t *AllocateHeap(Xsize_t Size);
		void FreeHeap(Xuint8_t *At, Xsize_t Size);

		Xsize_t Align(Xsize_t Size);
		void *FindFreeBlock(Xsize_t Size,
							Block *&CurrentBlock);

	public:
		/**
		 * Arrange the blocks to optimize the memory
		 * usage.
		 * The allocator is not arranged by default
		 * to avoid performance issues.
		 * This function will defragment the memory
		 * and free the unused blocks.
		 *
		 * You should call this function when the
		 * kernel is idle or when is not using the
		 * allocator.
		 */
		void Arrange();

		/**
		 * Allocate a new memory block
		 *
		 * @param Size Size of the block to allocate.
		 * @return void* Pointer to the allocated
		 * block.
		 */
		void *malloc(Xsize_t Size);

		/**
		 * Free a previously allocated block
		 *
		 * @param Address Address of the block to
		 * free.
		 */
		void free(void *Address);

		/**
		 * Allocate a new memory block
		 *
		 * @param NumberOfBlocks Number of blocks
		 * to allocate.
		 * @param Size Size of the block to allocate.
		 * @return void* Pointer to the allocated
		 * block.
		 */
		void *calloc(Xsize_t NumberOfBlocks,
					 Xsize_t Size);

		/**
		 * Reallocate a previously allocated block
		 *
		 * @param Address Address of the block
		 * to reallocate.
		 * @param Size New size of the block.
		 * @return void* Pointer to the reallocated
		 * block.
		 */
		void *realloc(void *Address, Xsize_t Size);

		/**
		 * Construct a new Allocator object
		 *
		 * @param VirtualBase Virtual address
		 * to map the pages.
		 */
		V2(void *VirtualBase);

		/**
		 * Destroy the Allocator object
		 */
		~V2();

		friend class Block;
	};
}

#endif // !__FENNIX_KERNEL_Xalloc_H__
