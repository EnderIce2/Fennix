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

#include <sys/mman.h>

#include <assert.h>

void *mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off)
{
	assert(!"mmap not implemented yet");
}

int posix_madvise(void *addr, size_t size, int advice)
{
	assert(!"posix_madvise not implemented yet");
}

int munmap(void *addr, size_t length)
{
	assert(!"munmap not implemented yet");
}
