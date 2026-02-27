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

#include <acpi.hpp>
#include <debug.h>
#include <elf.h>
#ifdef DEBUG
#include <uart.hpp>
#endif

#include "../../kernel.h"

namespace Memory
{
	fnx::void_t DMA::Allocate(size_t Size, size_t Alignment, size_t Boundary, size_t MaxAddress)
	{
		if (Size == 0)
			return nullptr;

		size_t PageCount = TO_PAGES(Size);
		Bitmap bitmap = KernelAllocator.GetPageBitmap();
		size_t TotalPages = bitmap.Size * 8;

		size_t SearchLimitPages = TotalPages;
		if (MaxAddress > 0)
		{
			size_t MaxPage = MaxAddress / PAGE_SIZE;
			if (SearchLimitPages > MaxPage)
				SearchLimitPages = MaxPage;
		}

		/* Search for a contiguous block of free pages that meets the requirements */
		for (size_t i = 0; i < SearchLimitPages; i++)
		{
			uintptr_t Address = i * PAGE_SIZE;

			/* 1. Check Alignment */
			if (Alignment > 0 && (Address % Alignment != 0))
				continue;

			/* 2. Check Boundary crossing */
			if (Boundary > 0)
			{
				uintptr_t EndAddress = Address + Size - 1;
				if ((Address / Boundary) != (EndAddress / Boundary))
				{
					/* Optimization: jump to the start of the next boundary block */
					size_t NextBoundaryPage = (((Address / Boundary) + 1) * Boundary) / PAGE_SIZE;

					if (NextBoundaryPage > i)
						i = NextBoundaryPage - 1;

					continue;
				}
			}

			/* 3. Check if the pages are actually free */
			if (i + PageCount > SearchLimitPages)
				break; /* Not enough memory left before the boundary/end */

			bool IsFree = true;
			for (size_t j = 0; j < PageCount; j++)
			{
				if (bitmap[i + j])
				{
					IsFree = false;
					i += j; /* Optimization: Skip the checked pages */
					break;
				}
			}

			if (IsFree)
			{
				void *Ptr = (void *)Address;
				KernelAllocator.LockPages(Ptr, PageCount);
				return Ptr;
			}
		}

		return nullptr;
	}
}
