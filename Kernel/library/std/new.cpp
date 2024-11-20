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

#include <new>

#include <memory.hpp>
#include <stdexcept>

void *operator new(std::size_t count)
{
	if (count == 0)
		++count;

	if (void *ptr = kmalloc(count))
		return ptr;

	throw std::bad_alloc{};
}

void *operator new[](std::size_t count)
{
	if (count == 0)
		++count;

	if (void *ptr = kmalloc(count))
		return ptr;

	throw std::bad_alloc{};
}

// void *operator new(std::size_t count, std::align_val_t al)
// void *operator new[](std::size_t count, std::align_val_t al)

// void *operator new(std::size_t count, const std::nothrow_t &tag)
// void *operator new[](std::size_t count, const std::nothrow_t &tag)
// void *operator new(std::size_t count, std::align_val_t al, const std::nothrow_t &)
// void *operator new[](std::size_t count, std::align_val_t al, const std::nothrow_t &)

void *operator new(std::size_t, void *ptr) noexcept { return ptr; }
// void *operator new[](std::size_t count, void *ptr) noexcept

// void *operator new(std::size_t count, ...)
// void *operator new[](std::size_t count, ...)
// void *operator new(std::size_t count, std::align_val_t al, ...)
// void *operator new[](std::size_t count, std::align_val_t al, ...)

void operator delete(void *ptr) noexcept { kfree(ptr); }

void operator delete[](void *ptr) noexcept { kfree(ptr); }

// void operator delete(void *ptr, std::align_val_t al) noexcept
// void operator delete[](void *ptr, std::align_val_t al) noexcept
void operator delete(void *ptr, std::size_t) noexcept { kfree(ptr); }

void operator delete[](void *ptr, std::size_t sz) noexcept { kfree(ptr); }
void operator delete(void *ptr, std::size_t sz, std::align_val_t al) noexcept { kfree(ptr); }
// void operator delete[](void *ptr, std::size_t sz, std::align_val_t al) noexcept

// void operator delete(void *ptr, const std::nothrow_t &tag) noexcept
// void operator delete[](void *ptr, const std::nothrow_t &tag) noexcept
// void operator delete(void *ptr, std::align_val_t al, const std::nothrow_t &tag) noexcept
// void operator delete[](void *ptr, std::align_val_t al, const std::nothrow_t &tag) noexcept

// void operator delete(void *ptr, void *place) noexcept
// void operator delete[](void *ptr, void *place) noexcept

// void operator delete(void *ptr, ...)
// void operator delete[](void *ptr, ...)
