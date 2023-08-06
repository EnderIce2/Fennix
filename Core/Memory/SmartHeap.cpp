#include <memory.hpp>

namespace Memory
{
	SmartHeap::SmartHeap(size_t Size)
	{
		this->Object = kmalloc(Size);
		this->ObjectSize = Size;
	}

	SmartHeap::~SmartHeap()
	{
		kfree(this->Object);
	}
}
