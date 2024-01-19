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

#include <memory/vma.hpp>
#include <memory/table.hpp>
#include <debug.h>

#include "../../kernel.h"

namespace Memory
{
	// ReadFSFunction(MEM_Read)
	// {
	// 	if (Size <= 0)
	// 		Size = node->Length;

	// 	if (RefOffset > node->Length)
	// 		return 0;

	// 	if ((node->Length - RefOffset) == 0)
	// 		return 0; /* EOF */

	// 	if (RefOffset + (off_t)Size > node->Length)
	// 		Size = node->Length;

	// 	memcpy(Buffer, (uint8_t *)(node->Address + RefOffset), Size);
	// 	return Size;
	// }

	// WriteFSFunction(MEM_Write)
	// {
	// 	if (Size <= 0)
	// 		Size = node->Length;

	// 	if (RefOffset > node->Length)
	// 		return 0;

	// 	if (RefOffset + (off_t)Size > node->Length)
	// 		Size = node->Length;

	// 	memcpy((uint8_t *)(node->Address + RefOffset), Buffer, Size);
	// 	return Size;
	// }

	// vfs::FileSystemOperations mem_op = {
	// 	.Name = "mem",
	// 	.Read = MEM_Read,
	// 	.Write = MEM_Write,
	// };

	uint64_t VirtualMemoryArea::GetAllocatedMemorySize()
	{
		SmartLock(MgrLock);
		uint64_t Size = 0;
		foreach (auto ap in AllocatedPagesList)
			Size += ap.PageCount;
		return FROM_PAGES(Size);
	}

	bool VirtualMemoryArea::Add(void *Address, size_t Count)
	{
		SmartLock(MgrLock);
		function("%#lx, %lld", Address, Count);

		if (Address == nullptr)
		{
			error("Address is null!");
			return false;
		}

		if (Count == 0)
		{
			error("Count is 0!");
			return false;
		}

		for (size_t i = 0; i < AllocatedPagesList.size(); i++)
		{
			if (AllocatedPagesList[i].Address == Address)
			{
				error("Address already exists!");
				return false;
			}
			else if ((uintptr_t)Address < (uintptr_t)AllocatedPagesList[i].Address)
			{
				if ((uintptr_t)Address + (Count * PAGE_SIZE) > (uintptr_t)AllocatedPagesList[i].Address)
				{
					error("Address intersects with an allocated page!");
					return false;
				}
			}
			else
			{
				if ((uintptr_t)AllocatedPagesList[i].Address + (AllocatedPagesList[i].PageCount * PAGE_SIZE) > (uintptr_t)Address)
				{
					error("Address intersects with an allocated page!");
					return false;
				}
			}
		}

		AllocatedPagesList.push_back({Address, Count});
		return true;
	}

	void *VirtualMemoryArea::RequestPages(size_t Count, bool User)
	{
		SmartLock(MgrLock);
		function("%lld, %s", Count, User ? "true" : "false");

		void *Address = KernelAllocator.RequestPages(Count);
		memset(Address, 0, Count * PAGE_SIZE);
		for (size_t i = 0; i < Count; i++)
		{
			int Flags = Memory::PTFlag::RW;
			if (User)
				Flags |= Memory::PTFlag::US;

			void *AddressToMap = (void *)((uintptr_t)Address + (i * PAGE_SIZE));

			Memory::Virtual vmm(this->Table);
			vmm.Map(AddressToMap, AddressToMap, Flags);
		}

		AllocatedPagesList.push_back({Address, Count});
		return Address;
	}

	void VirtualMemoryArea::FreePages(void *Address, size_t Count)
	{
		SmartLock(MgrLock);
		function("%#lx, %lld", Address, Count);

		forItr(itr, AllocatedPagesList)
		{
			if (itr->Address == Address)
			{
				/** TODO: Advanced checks. Allow if the page count is less than the requested one.
				 * This will allow the user to free only a part of the allocated pages.
				 *
				 * But this will be in a separate function because we need to specify if we
				 * want to free from the start or from the end and return the new address.
				 */
				if (itr->PageCount != Count)
				{
					error("Page count mismatch! (Allocated: %lld, Requested: %lld)", itr->PageCount, Count);
					return;
				}

				KernelAllocator.FreePages(Address, Count);

				Memory::Virtual vmm(this->Table);
				for (size_t i = 0; i < Count; i++)
				{
					void *AddressToMap = (void *)((uintptr_t)Address + (i * PAGE_SIZE));
					vmm.Remap(AddressToMap, AddressToMap, Memory::PTFlag::RW);
					// vmm.Unmap((void *)((uintptr_t)Address + (i * PAGE_SIZE)));
				}
				AllocatedPagesList.erase(itr);
				return;
			}
		}
	}

	void VirtualMemoryArea::DetachAddress(void *Address)
	{
		SmartLock(MgrLock);
		function("%#lx", Address);

		forItr(itr, AllocatedPagesList)
		{
			if (itr->Address == Address)
			{
				AllocatedPagesList.erase(itr);
				return;
			}
		}
	}

	void *VirtualMemoryArea::CreateCoWRegion(void *Address,
											 size_t Length,
											 bool Read, bool Write, bool Exec,
											 bool Fixed, bool Shared)
	{
		function("%#lx, %lld, %s, %s, %s, %s, %s", Address, Length,
				 Read ? "true" : "false",
				 Write ? "true" : "false",
				 Exec ? "true" : "false",
				 Fixed ? "true" : "false",
				 Shared ? "true" : "false");

		Memory::Virtual vmm(this->Table);

		// FIXME
		// for (uintptr_t j = uintptr_t(Address);
		// 	 j < uintptr_t(Address) + Length;
		// 	 j += PAGE_SIZE)
		// {
		// 	if (vmm.Check((void *)j, Memory::G))
		// 	{
		// 		if (Fixed)
		// 			return (void *)-EINVAL;
		// 		Address = (void *)(j + PAGE_SIZE);
		// 	}
		// }

		bool AnyAddress = Address == nullptr;
		debug("AnyAddress: %s", AnyAddress ? "true" : "false");

		if (AnyAddress)
		{
			Address = this->RequestPages(TO_PAGES(Length), true);
			debug("Allocated %#lx-%#lx for pt %#lx",
				  Address, (uintptr_t)Address + Length, this->Table);
			return Address;
		}

		SmartLock(MgrLock);
		vmm.Unmap(Address, Length);
		vmm.Map(Address, nullptr, Length, PTFlag::CoW);
		debug("CoW region created at range %#lx-%#lx for pt %#lx",
			  Address, (uintptr_t)Address + Length, this->Table);

		SharedRegion sr{
			.Address = Address,
			.Read = Read,
			.Write = Write,
			.Exec = Exec,
			.Fixed = Fixed,
			.Shared = Shared,
			.Length = Length,
			.ReferenceCount = 0,
		};
		SharedRegions.push_back(sr);
		debug("CoW region created at %#lx for pt %#lx",
			  Address, this->Table);
		return Address;
	}

	bool VirtualMemoryArea::HandleCoW(uintptr_t PFA)
	{
		function("%#lx", PFA);
		Memory::Virtual vmm(this->Table);
		Memory::PageTableEntry *pte = vmm.GetPTE((void *)PFA);

		if (!pte)
		{
			/* Unmapped page */
			debug("PTE is null!");
			return false;
		}

		if (pte->CopyOnWrite == true)
		{
			foreach (auto sr in SharedRegions)
			{
				uintptr_t Start = (uintptr_t)sr.Address;
				uintptr_t End = (uintptr_t)sr.Address + sr.Length;

				if (PFA >= Start && PFA < End)
				{
					debug("Start: %#lx, End: %#lx (PFA: %#lx)",
						  Start, End, PFA);

					if (sr.Shared)
					{
						fixme("Shared CoW");
						return false;
					}
					else
					{
						void *pAddr = this->RequestPages(1);
						if (pAddr == nullptr)
							return false;
						memset(pAddr, 0, PAGE_SIZE);

						assert(pte->Present == true);
						pte->ReadWrite = sr.Write;
						pte->UserSupervisor = sr.Read;
						pte->ExecuteDisable = sr.Exec;

						pte->CopyOnWrite = false;
						debug("PFA %#lx is CoW (pt %#lx, flags %#lx)",
							  PFA, this->Table, pte->raw);
#if defined(a64)
						CPU::x64::invlpg((void *)PFA);
#elif defined(a32)
						CPU::x32::invlpg((void *)PFA);
#endif
						return true;
					}
				}
			}
		}

		debug("PFA %#lx is not CoW (pt %#lx)",
			  PFA, this->Table);
		return false;
	}

	void VirtualMemoryArea::FreeAllPages()
	{
		SmartLock(MgrLock);
		foreach (auto ap in AllocatedPagesList)
		{
			KernelAllocator.FreePages(ap.Address, ap.PageCount);
			Memory::Virtual vmm(this->Table);
			for (size_t i = 0; i < ap.PageCount; i++)
				vmm.Remap((void *)((uintptr_t)ap.Address + (i * PAGE_SIZE)),
						  (void *)((uintptr_t)ap.Address + (i * PAGE_SIZE)),
						  Memory::PTFlag::RW);
		}
		AllocatedPagesList.clear();
	}

	void VirtualMemoryArea::Fork(VirtualMemoryArea *Parent)
	{
		function("%#lx", Parent);

		if (Parent == nullptr)
		{
			error("Parent is null!");
			return;
		}

		if (Parent->Table == nullptr)
		{
			error("Parent's table is null!");
			return;
		}

		Memory::Virtual vmm(this->Table);
		SmartLock(MgrLock);
		foreach (auto ap in Parent->GetAllocatedPagesList())
		{
			MgrLock.Unlock();
			void *Address = this->RequestPages(ap.PageCount);
			MgrLock.Lock(__FUNCTION__);
			if (Address == nullptr)
				return;

			memcpy(Address, ap.Address, ap.PageCount * PAGE_SIZE);

			// map these new allocated pages to be the same as the parent
			for (size_t i = 0; i < ap.PageCount; i++)
			{
				void *AddressToMap = (void *)((uintptr_t)ap.Address + (i * PAGE_SIZE));
				void *RealAddress = (void *)((uintptr_t)Address + (i * PAGE_SIZE));

#if defined(a86)
				Memory::PageTableEntry *pte = vmm.GetPTE(AddressToMap);
				uintptr_t Flags = 0;
				Flags |= pte->Present ? 1UL : 0;
				Flags |= pte->ReadWrite ? 2UL : 0;
				Flags |= pte->UserSupervisor ? 4UL : 0;
				Flags |= pte->CopyOnWrite ? 512UL : 0;

				debug("Mapping %#lx to %#lx (flags %s/%s/%s/%s)",
					  RealAddress, AddressToMap,
					  Flags & PTFlag::P ? "P" : "-",
					  Flags & PTFlag::RW ? "RW" : "-",
					  Flags & PTFlag::US ? "US" : "-",
					  Flags & PTFlag::CoW ? "CoW" : "-");
				vmm.Map(AddressToMap, RealAddress, Flags);
#else
#warning "Not implemented"
#endif
			}
		}

		foreach (auto sr in Parent->GetSharedRegions())
		{
			MgrLock.Unlock();
			void *Address = this->CreateCoWRegion(sr.Address, sr.Length,
												 sr.Read, sr.Write, sr.Exec,
												 sr.Fixed, sr.Shared);
			MgrLock.Lock(__FUNCTION__);
			if (Address == nullptr)
				return;
			memcpy(Address, sr.Address, sr.Length);
		}
	}

	VirtualMemoryArea::VirtualMemoryArea(PageTable *Table)
	{
		debug("+ %#lx %s", this,
			  KernelSymbolTable ? KernelSymbolTable->GetSymbol((uintptr_t)__builtin_return_address(0)) : "");

		SmartLock(MgrLock);
		if (Table)
			this->Table = Table;
		else
		{
			if (TaskManager)
				this->Table = thisProcess->PageTable;
			else
#if defined(a64)
				this->Table = (PageTable *)CPU::x64::readcr3().raw;
#elif defined(a32)
				this->Table = (PageTable *)CPU::x32::readcr3().raw;
#endif
		}
	}

	VirtualMemoryArea::~VirtualMemoryArea()
	{
		debug("- %#lx %s", this,
			  KernelSymbolTable ? KernelSymbolTable->GetSymbol((uintptr_t)__builtin_return_address(0)) : "");

#ifdef DEBUG
		if (this->Table == KernelPageTable)
			debug("Not remapping kernel page table allocated pages.");
#endif

		SmartLock(MgrLock);
		Memory::Virtual vmm(this->Table);
		foreach (auto ap in AllocatedPagesList)
		{
			KernelAllocator.FreePages(ap.Address, ap.PageCount);

			if (this->Table == KernelPageTable)
				continue;

			for (size_t i = 0; i < ap.PageCount; i++)
				vmm.Remap((void *)((uintptr_t)ap.Address + (i * PAGE_SIZE)),
						  (void *)((uintptr_t)ap.Address + (i * PAGE_SIZE)),
						  Memory::PTFlag::RW);
		}
	}
}
