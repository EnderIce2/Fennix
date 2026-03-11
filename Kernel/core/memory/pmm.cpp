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
	uint64_t Physical::GetTotalMemory() { return FROM_PAGES(this->TotalMemory.load()); }
	uint64_t Physical::GetFreeMemory() { return FROM_PAGES(this->FreeMemory.load()); }
	uint64_t Physical::GetReservedMemory() { return FROM_PAGES(this->ReservedMemory.load()); }
	uint64_t Physical::GetUsedMemory() { return FROM_PAGES(this->UsedMemory.load()); }

	void Physical::LockPage(fnx::void_t Address)
	{
		if (unlikely(Address == nullptr))
			warn("Trying to lock null address.");

		uintptr_t index = Address / PAGE_SIZE;
		if (unlikely(PageBitmap[index] == true))
			return;

		if (PageBitmap.Set(index, true))
		{
			FreeMemory.fetch_sub(1);
			UsedMemory.fetch_add(1);
		}
	}

	void Physical::LockPages(fnx::void_t Address, size_t PageCount)
	{
		if (unlikely(Address == nullptr || PageCount == 0))
			warn("Trying to lock %s%s.", Address ? "null" : "", PageCount ? "0 pages" : "");

		size_t changed = 0;
		uintptr_t startIndex = Address / PAGE_SIZE;

		for (size_t i = 0; i < PageCount; i++)
		{
			if (PageBitmap.Set(startIndex + i, true))
				changed++;
		}

		if (changed > 0)
		{
			FreeMemory.fetch_sub(changed);
			UsedMemory.fetch_add(changed);
		}
	}

	void Physical::ReservePage(fnx::void_t Address)
	{
		if (unlikely(Address == nullptr))
			warn("Trying to reserve null address.");

		uintptr_t index = Address / PAGE_SIZE;
		if (unlikely(PageBitmap[index] == true))
			return;

		if (PageBitmap.Set(index, true))
		{
			FreeMemory.fetch_sub(1);
			ReservedMemory.fetch_add(1);
		}
	}

	void Physical::ReservePages(fnx::void_t Address, size_t PageCount)
	{
		if (unlikely(Address == nullptr || PageCount == 0))
			warn("Trying to reserve %s%s.", Address ? "null" : "", PageCount ? "0 pages" : "");

		size_t changed = 0;
		uintptr_t startIndex = Address / PAGE_SIZE;

		for (size_t i = 0; i < PageCount; i++)
		{
			if (PageBitmap.Set(startIndex + i, true))
				changed++;
		}

		if (changed > 0)
		{
			FreeMemory.fetch_sub(changed);
			ReservedMemory.fetch_add(changed);
		}
	}

	void Physical::UnreservePage(fnx::void_t Address)
	{
		if (unlikely(Address == nullptr))
			warn("Trying to unreserve null address.");

		uintptr_t Index = Address / PAGE_SIZE;

		if (unlikely(PageBitmap[Index] == false))
			return;

		if (PageBitmap.Set(Index, false))
		{
			FreeMemory.fetch_add(1);
			ReservedMemory.fetch_sub(1);
			if (PageBitmapIndex > Index)
				PageBitmapIndex = Index;
		}
	}

	void Physical::UnreservePages(fnx::void_t Address, size_t PageCount)
	{
		if (unlikely(Address == nullptr || PageCount == 0))
			warn("Trying to unreserve %s%s.", Address ? "null" : "", PageCount ? "0 pages" : "");

		size_t changed = 0;
		uintptr_t startIndex = Address / PAGE_SIZE;

		for (size_t i = 0; i < PageCount; i++)
		{
			if (PageBitmap.Set(startIndex + i, false))
				changed++;
		}

		if (changed > 0)
		{
			FreeMemory.fetch_add(changed);
			ReservedMemory.fetch_sub(changed);
			if (PageBitmapIndex > startIndex)
				PageBitmapIndex = startIndex;
		}
	}

	fnx::void_t Physical::RequestPage(bool FirstFit)
	{
		SmartLock(this->MemoryLock);
	retryWithFirstFit:
		size_t startIndex = FirstFit ? 0 : PageBitmapIndex;

		for (; startIndex < PageBitmap.Size * 8; startIndex++)
		{
			if (PageBitmap[startIndex] == true)
				continue;

			this->LockPage(startIndex * PAGE_SIZE);

			if (FirstFit == false)
				PageBitmapIndex = startIndex + 1;

			return startIndex * PAGE_SIZE;
		}

		if (FirstFit == false)
		{
			warn("Failed to allocate page using Next-Fit algorithm. Retrying with First-Fit...");
			FirstFit = true;
			goto retryWithFirstFit;
		}

		if (OutOfMemoryHandler)
			OutOfMemoryHandler();

		klog("Out of memory! (Free: %ld MiB; Used: %ld MiB; Reserved: %ld MiB)", TO_MiB(FROM_PAGES(FreeMemory.load())), TO_MiB(FROM_PAGES(UsedMemory.load())), TO_MiB(FROM_PAGES(ReservedMemory.load())));
		debug("Raw values: free %#lx used %#lx reserved %#lx", FreeMemory.load(), UsedMemory.load(), ReservedMemory.load());
		CPU::Stop();
		__builtin_unreachable();
	}

	fnx::void_t Physical::RequestPages(size_t Count, bool FirstFit)
	{
		SmartLock(this->MemoryLock);

	retryWithFirstFit:
		size_t startIndex = FirstFit ? 0 : PageBitmapIndex;

		for (; startIndex < PageBitmap.Size * 8; startIndex++)
		{
			if (PageBitmap[startIndex] == true)
				continue;

			if (startIndex + Count > PageBitmap.Size * 8)
				break;

			size_t i = 0;
			for (; i < Count; i++)
			{
				if (PageBitmap[startIndex + i] == true)
					break;
			}

			if (i == Count)
			{
				uintptr_t foundAddress = startIndex * PAGE_SIZE;
				this->LockPages(foundAddress, Count);

				startIndex += Count;

				if (FirstFit == false)
					PageBitmapIndex = startIndex;

				return foundAddress;
			}

			startIndex += i;
		}

		if (FirstFit == false)
		{
			warn("Failed to allocate pages using Next-Fit algorithm. Retrying with First-Fit...");
			FirstFit = true;
			goto retryWithFirstFit;
		}

		if (OutOfMemoryHandler)
			OutOfMemoryHandler();

		klog("Out of memory! (Free: %ld MiB; Used: %ld MiB; Reserved: %ld MiB)", TO_MiB(FROM_PAGES(FreeMemory.load())), TO_MiB(FROM_PAGES(UsedMemory.load())), TO_MiB(FROM_PAGES(ReservedMemory.load())));
		debug("Raw values: free %#lx used %#lx reserved %#lx", FreeMemory.load(), UsedMemory.load(), ReservedMemory.load());
		CPU::Halt(true);
		__builtin_unreachable();
	}

	void Physical::FreePage(fnx::void_t Address)
	{
		SmartLock(this->MemoryLock);

		if (unlikely(Address == nullptr))
		{
			warn("Null pointer passed to FreePage.");
			return;
		}

		size_t Index = Address / PAGE_SIZE;

		if (unlikely(PageBitmap[Index] == false))
		{
			warn("Tried to free an already free page at %#lx.", Address.get());
			return;
		}

		if (PageBitmap.Set(Index, false))
		{
			FreeMemory.fetch_add(1);
			UsedMemory.fetch_sub(1);
			if (PageBitmapIndex > Index)
				PageBitmapIndex = Index;
		}
	}

	void Physical::FreePages(fnx::void_t Address, size_t Count)
	{
		if (unlikely(Address == nullptr || Count == 0))
		{
			warn("%s%s%s passed to FreePages.", Address == nullptr ? "Null pointer " : "", Address == nullptr && Count == 0 ? "and " : "", Count == 0 ? "Zero count" : "");
			return;
		}

		SmartLock(this->MemoryLock);
		size_t changed = 0;
		uintptr_t startIndex = Address / PAGE_SIZE;
		uintptr_t minFreedIndex = -1;

		for (size_t i = 0; i < Count; i++)
		{
			if (PageBitmap.Set(startIndex + i, false))
			{
				changed++;
				if ((startIndex + i) < minFreedIndex)
					minFreedIndex = startIndex + i;
			}
		}

		if (changed > 0)
		{
			FreeMemory.fetch_add(changed);
			UsedMemory.fetch_sub(changed);
			if (PageBitmapIndex > minFreedIndex)
				PageBitmapIndex = minFreedIndex;
		}
	}

	void Physical::Init()
	{
		SmartLock(this->MemoryLock);

		size_t memSize = bInfo.Memory.Size;
		debug("Memory size: %lld bytes (%ld pages)", memSize, TO_PAGES(memSize));
		TotalMemory.store(TO_PAGES(memSize));
		FreeMemory.store(TO_PAGES(memSize));

		size_t bmSize = (memSize / PAGE_SIZE) / 8 + 1;
		uintptr_t bmAddress = 0x0;
		size_t bmAddressSize = 0;

		FindBitmapRegion(bmAddress, bmAddressSize);
		if (bmAddress == 0x0)
		{
			error("No free memory found!");
			CPU::Stop();
		}

		debug("Initializing Bitmap at %#lx-%#lx (%zu Bytes)", bmAddress, bmAddress + bmSize, bmSize);
		PageBitmap.Size = bmSize;
		PageBitmap.Buffer = (fnx::void_t)bmAddress;
		memset((fnx::void_t)bmAddress, 0, bmSize);
		ReserveEssentials();
	}
}
