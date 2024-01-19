/*
	This file is part of Fennix Drivers.

	Fennix Drivers is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Drivers is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Drivers. If not, see <https://www.gnu.org/licenses/>.
*/

#include <base.h>
#include <driver.h>

extern __driverAPI *API;

/**
 * TODO: memory allocator
 * we can't really call kernel's malloc because the
 * kernel keep track of memory usage for each driver
 *
 * maybe implement the allocator in vma?
 */

void *__new_alloc_page(std::size_t Size)
{
	/* Do not allow allocations larger than 4 KiB */
	if (Size > PAGE_SIZE)
		asmv("ud2");

	return API->RequestPages(API->MajorID, 1);
}

void __delete_alloc_page(void *Pointer)
{
	API->FreePages(API->MajorID, Pointer, 1);
}

void *operator new(std::size_t Size)
{
	return __new_alloc_page(Size);
}

void *operator new[](std::size_t Size)
{
	return __new_alloc_page(Size);
}

void operator delete(void *Pointer)
{
	__delete_alloc_page(Pointer);
}

void operator delete[](void *Pointer)
{
	__delete_alloc_page(Pointer);
}

void operator delete(void *Pointer, std::size_t)
{
	__delete_alloc_page(Pointer);
}

void operator delete[](void *Pointer, std::size_t)
{
	__delete_alloc_page(Pointer);
}
