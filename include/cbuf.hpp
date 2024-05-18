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

#ifndef __FENNIX_KERNEL_CIRCULAR_BUFFER_H__
#define __FENNIX_KERNEL_CIRCULAR_BUFFER_H__

#include <types.h>
#include <lock.hpp>

class CircularBuffer
{
private:
	spin_lock Lock;
	uint8_t *Buffer;
	size_t BufferSize;
	size_t BufferCount;

	size_t Head;
	size_t Tail;

public:
	CircularBuffer(size_t Size);
	~CircularBuffer();
	size_t Write(const uint8_t *Data, size_t Size);
	size_t Read(uint8_t *Data, size_t Size);
	size_t Peek(uint8_t *Data, size_t Size);
	size_t Count();
	size_t Free();
};

#endif // !__FENNIX_KERNEL_CIRCULAR_BUFFER_H__
