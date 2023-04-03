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

typedef long unsigned Xuint64_t;
typedef long unsigned Xsize_t;

#define Xalloc_StopOnFail true
#define Xalloc_PAGE_SIZE PAGE_SIZE
#define Xalloc_trace(m, ...) trace(m, ##__VA_ARGS__)
#define Xalloc_warn(m, ...) warn(m, ##__VA_ARGS__)
#define Xalloc_err(m, ...) error(m, ##__VA_ARGS__)
#define Xalloc_def NewLock(XallocLock)
#define Xalloc_lock XallocLock.Lock(__FUNCTION__)
#define Xalloc_unlock XallocLock.Unlock()

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
}

#endif // !__FENNIX_KERNEL_Xalloc_H__
