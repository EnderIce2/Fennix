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
	uint64_t Physical::GetTotalMemory()
	{
		return this->TotalMemory.load();
	}

	uint64_t Physical::GetFreeMemory()
	{
		return this->FreeMemory.load();
	}

	uint64_t Physical::GetReservedMemory()
	{
		return this->ReservedMemory.load();
	}

	uint64_t Physical::GetUsedMemory()
	{
		return this->UsedMemory.load();
	}

	bool Physical::SwapPage(void *Address)
	{
		fixme("%p", Address);
		return false;
	}

	bool Physical::SwapPages(void *Address, size_t PageCount)
	{
		for (size_t i = 0; i < PageCount; i++)
		{
			if (!this->SwapPage((void *)((uintptr_t)Address + (i * PAGE_SIZE))))
				return false;
		}
		return false;
	}

	bool Physical::UnswapPage(void *Address)
	{
		fixme("%p", Address);
		return false;
	}

	bool Physical::UnswapPages(void *Address, size_t PageCount)
	{
		for (size_t i = 0; i < PageCount; i++)
		{
			if (!this->UnswapPage((void *)((uintptr_t)Address + (i * PAGE_SIZE))))
				return false;
		}
		return false;
	}

	void *Physical::RequestPage()
	{
		SmartLock(this->MemoryLock);

		for (; PageBitmapIndex < PageBitmap.Size * 8; PageBitmapIndex++)
		{
			if (PageBitmap[PageBitmapIndex] == true)
				continue;

			this->LockPage((void *)(PageBitmapIndex * PAGE_SIZE));
			return (void *)(PageBitmapIndex * PAGE_SIZE);
		}

		if (this->SwapPage((void *)(PageBitmapIndex * PAGE_SIZE)))
		{
			this->LockPage((void *)(PageBitmapIndex * PAGE_SIZE));
			return (void *)(PageBitmapIndex * PAGE_SIZE);
		}

		if (TaskManager && !TaskManager->IsPanic())
		{
			error("Out of memory! Killing current process...");
			TaskManager->KillProcess(thisProcess, Tasking::KILL_OOM);
			TaskManager->Yield();
		}

		error("Out of memory! (Free: %ld MiB; Used: %ld MiB; Reserved: %ld MiB)",
			  TO_MiB(FreeMemory.load()), TO_MiB(UsedMemory.load()), TO_MiB(ReservedMemory.load()));
		KPrint("Out of memory! (Free: %ld MiB; Used: %ld MiB; Reserved: %ld MiB)",
			   TO_MiB(FreeMemory.load()), TO_MiB(UsedMemory.load()), TO_MiB(ReservedMemory.load()));
		debug("Raw values: free %#lx used %#lx reserved %#lx",
			  FreeMemory.load(), UsedMemory.load(), ReservedMemory.load());
		CPU::Stop();
		__builtin_unreachable();
	}

	void *Physical::RequestPages(size_t Count)
	{
		SmartLock(this->MemoryLock);

		for (; PageBitmapIndex < PageBitmap.Size * 8; PageBitmapIndex++)
		{
			if (PageBitmap[PageBitmapIndex] == true)
				continue;

			for (uint64_t Index = PageBitmapIndex; Index < PageBitmap.Size * 8; Index++)
			{
				if (PageBitmap[Index] == true)
					continue;

				for (size_t i = 0; i < Count; i++)
				{
					if (PageBitmap[Index + i] == true)
						goto NextPage;
				}

				this->LockPages((void *)(Index * PAGE_SIZE), Count);
				return (void *)(Index * PAGE_SIZE);

			NextPage:
				Index += Count;
				continue;
			}
		}

		if (this->SwapPages((void *)(PageBitmapIndex * PAGE_SIZE), Count))
		{
			this->LockPages((void *)(PageBitmapIndex * PAGE_SIZE), Count);
			return (void *)(PageBitmapIndex * PAGE_SIZE);
		}

		if (TaskManager && !TaskManager->IsPanic())
		{
			error("Out of memory! Killing current process...");
			TaskManager->KillProcess(thisProcess, Tasking::KILL_OOM);
			TaskManager->Yield();
		}

		error("Out of memory! (Free: %ld MiB; Used: %ld MiB; Reserved: %ld MiB)",
			  TO_MiB(FreeMemory.load()), TO_MiB(UsedMemory.load()), TO_MiB(ReservedMemory.load()));
		KPrint("Out of memory! (Free: %ld MiB; Used: %ld MiB; Reserved: %ld MiB)",
			   TO_MiB(FreeMemory.load()), TO_MiB(UsedMemory.load()), TO_MiB(ReservedMemory.load()));
		debug("Raw values: free %#lx used %#lx reserved %#lx",
			  FreeMemory.load(), UsedMemory.load(), ReservedMemory.load());
		CPU::Halt(true);
		__builtin_unreachable();
	}

	void Physical::FreePage(void *Address)
	{
		SmartLock(this->MemoryLock);

		if (unlikely(Address == nullptr))
		{
			warn("Null pointer passed to FreePage.");
			return;
		}

		size_t Index = (size_t)Address / PAGE_SIZE;

		if (unlikely(PageBitmap[Index] == false))
		{
			warn("Tried to free an already free page. (%p)",
				 Address);
			return;
		}

		if (PageBitmap.Set(Index, false))
		{
			FreeMemory.fetch_add(PAGE_SIZE);
			UsedMemory.fetch_sub(PAGE_SIZE);
			if (PageBitmapIndex > Index)
				PageBitmapIndex = Index;
		}
	}

	void Physical::FreePages(void *Address, size_t Count)
	{
		if (unlikely(Address == nullptr || Count == 0))
		{
			warn("%s%s%s passed to FreePages.", Address == nullptr ? "Null pointer " : "", Address == nullptr && Count == 0 ? "and " : "", Count == 0 ? "Zero count" : "");
			return;
		}
		for (size_t t = 0; t < Count; t++)
			this->FreePage((void *)((uintptr_t)Address + (t * PAGE_SIZE)));
	}

	void Physical::LockPage(void *Address)
	{
		if (unlikely(Address == nullptr))
			warn("Trying to lock null address.");

		uintptr_t Index = (uintptr_t)Address / PAGE_SIZE;

		if (unlikely(PageBitmap[Index] == true))
			return;

		if (PageBitmap.Set(Index, true))
		{
			FreeMemory.fetch_sub(PAGE_SIZE);
			UsedMemory.fetch_add(PAGE_SIZE);
		}
	}

	void Physical::LockPages(void *Address, size_t PageCount)
	{
		if (unlikely(Address == nullptr || PageCount == 0))
			warn("Trying to lock %s%s.",
				 Address ? "null address" : "",
				 PageCount ? "0 pages" : "");

		for (size_t i = 0; i < PageCount; i++)
			this->LockPage((void *)((uintptr_t)Address + (i * PAGE_SIZE)));
	}

	void Physical::ReservePage(void *Address)
	{
		if (unlikely(Address == nullptr))
			warn("Trying to reserve null address.");

		uintptr_t Index = (Address == NULL) ? 0 : (uintptr_t)Address / PAGE_SIZE;

		if (unlikely(PageBitmap[Index] == true))
			return;

		if (PageBitmap.Set(Index, true))
		{
			FreeMemory.fetch_sub(PAGE_SIZE);
			ReservedMemory.fetch_add(PAGE_SIZE);
		}
	}

	void Physical::ReservePages(void *Address, size_t PageCount)
	{
		if (unlikely(Address == nullptr || PageCount == 0))
			warn("Trying to reserve %s%s.",
				 Address ? "null address" : "",
				 PageCount ? "0 pages" : "");

		for (size_t t = 0; t < PageCount; t++)
		{
			uintptr_t Index = ((uintptr_t)Address + (t * PAGE_SIZE)) / PAGE_SIZE;

			if (unlikely(PageBitmap[Index] == true))
				return;

			if (PageBitmap.Set(Index, true))
			{
				FreeMemory.fetch_sub(PAGE_SIZE);
				ReservedMemory.fetch_add(PAGE_SIZE);
			}
		}
	}

	void Physical::UnreservePage(void *Address)
	{
		if (unlikely(Address == nullptr))
			warn("Trying to unreserve null address.");

		uintptr_t Index = (Address == NULL) ? 0 : (uintptr_t)Address / PAGE_SIZE;

		if (unlikely(PageBitmap[Index] == false))
			return;

		if (PageBitmap.Set(Index, false))
		{
			FreeMemory.fetch_add(PAGE_SIZE);
			ReservedMemory.fetch_sub(PAGE_SIZE);
			if (PageBitmapIndex > Index)
				PageBitmapIndex = Index;
		}
	}

	void Physical::UnreservePages(void *Address, size_t PageCount)
	{
		if (unlikely(Address == nullptr || PageCount == 0))
			warn("Trying to unreserve %s%s.",
				 Address ? "null address" : "",
				 PageCount ? "0 pages" : "");

		for (size_t t = 0; t < PageCount; t++)
		{
			uintptr_t Index = ((uintptr_t)Address + (t * PAGE_SIZE)) / PAGE_SIZE;

			if (unlikely(PageBitmap[Index] == false))
				return;

			if (PageBitmap.Set(Index, false))
			{
				FreeMemory.fetch_add(PAGE_SIZE);
				ReservedMemory.fetch_sub(PAGE_SIZE);
				if (PageBitmapIndex > Index)
					PageBitmapIndex = Index;
			}
		}
	}

	void Physical::Init()
	{
		SmartLock(this->MemoryLock);

		uint64_t MemorySize = bInfo.Memory.Size;
		debug("Memory size: %lld bytes (%ld pages)",
			  MemorySize, TO_PAGES(MemorySize));
		TotalMemory.store(MemorySize);
		FreeMemory.store(MemorySize);

		size_t BitmapSize = (size_t)(MemorySize / PAGE_SIZE) / 8 + 1;
		uintptr_t BitmapAddress = 0x0;
		size_t BitmapAddressSize = 0;

		FindBitmapRegion(BitmapAddress, BitmapAddressSize);
		if (BitmapAddress == 0x0)
		{
			error("No free memory found!");
			CPU::Stop();
		}

		debug("Initializing Bitmap at %p-%p (%d Bytes)",
			  BitmapAddress,
			  (void *)(BitmapAddress + BitmapSize),
			  BitmapSize);

		PageBitmap.Size = BitmapSize;
		PageBitmap.Buffer = (uint8_t *)BitmapAddress;
		for (size_t i = 0; i < BitmapSize; i++)
			*(uint8_t *)(PageBitmap.Buffer + i) = 0;

		ReserveEssentials();
	}

	Physical::Physical() {}
	Physical::~Physical() {}
}
