#include <memory.hpp>

namespace Memory
{
	SmartHeap::SmartHeap(size_t Size, VirtualMemoryArea *vma)
	{
		if (vma)
		{
			this->vma = vma;
			this->Object = vma->RequestPages(TO_PAGES(Size));
		}
		else
			this->Object = kmalloc(Size);
		this->ObjectSize = Size;
		memset(this->Object, 0, Size);
	}

	SmartHeap::~SmartHeap()
	{
		if (this->vma)
			this->vma->FreePages(this->Object, TO_PAGES(this->ObjectSize));
		else
			kfree(this->Object);
	}
}
