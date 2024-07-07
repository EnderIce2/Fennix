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

#ifndef __FENNIX_KERNEL_RING_BUFFER_H__
#define __FENNIX_KERNEL_RING_BUFFER_H__

#include <types.h>
#include <lock.hpp>

template <typename T>
class RingBuffer
{
private:
	spin_lock *Lock;
	T *Buffer;
	size_t BufferSize;
	size_t BufferCount;

	size_t Head;
	size_t Tail;

public:
	RingBuffer(size_t Size = 16)
		: Lock(new spin_lock()),
		  Buffer(new T[Size]),
		  BufferSize(Size),
		  BufferCount(0),
		  Head(0),
		  Tail(0) {}

	~RingBuffer()
	{
		delete Lock;
		delete[] Buffer;
	}

	size_t Write(const T *Data, size_t Size)
	{
		sl_guard(*Lock);

		size_t written = 0;
		while (Size > 0)
		{
			if (BufferCount == BufferSize)
				break;

			Buffer[Head] = *Data++;
			Head = (Head + 1) % BufferSize;
			BufferCount++;
			written++;
			Size--;
		}

		return written;
	}

	size_t Read(T *Data, size_t Size)
	{
		sl_guard(*Lock);

		size_t read = 0;
		while (Size > 0)
		{
			if (BufferCount == 0)
				break;

			*Data++ = Buffer[Tail];
			Tail = (Tail + 1) % BufferSize;
			BufferCount--;
			read++;
			Size--;
		}

		return read;
	}

	size_t Peek(T *Data, size_t Size)
	{
		sl_guard(*Lock);

		size_t read = 0;
		size_t tail = Tail;
		while (Size > 0)
		{
			if (BufferCount == 0)
				break;

			*Data++ = Buffer[tail];
			tail = (tail + 1) % BufferSize;
			read++;
			Size--;
		}

		return read;
	}

	size_t Count()
	{
		sl_guard(*Lock);
		return BufferCount;
	}

	size_t Free()
	{
		sl_guard(*Lock);
		return BufferSize - BufferCount;
	}
};

#endif // !__FENNIX_KERNEL_RING_BUFFER_H__
