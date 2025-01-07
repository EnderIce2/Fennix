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
#include <cpu.hpp>
#include <debug.h>
#include <bitset>

#include "../../kernel.h"

namespace Memory
{
	uint64_t VirtualMemoryArea::GetAllocatedMemorySize()
	{
		SmartLock(MgrLock);
		uint64_t Size = 0;
		foreach (auto ap in AllocatedPagesList)
			Size += ap.PageCount;
		return FROM_PAGES(Size);
	}

	void *VirtualMemoryArea::RequestPages(size_t Count, bool User, bool Protect)
	{
		func("%lld, %s, %s", Count,
			 User ? "true" : "false",
			 Protect ? "true" : "false");

		void *Address = KernelAllocator.RequestPages(Count);
		memset(Address, 0, Count * PAGE_SIZE);

		int Flags = PTFlag::RW;
		if (User)
			Flags |= PTFlag::US;
		if (Protect)
			Flags |= PTFlag::KRsv;

		Virtual vmm(this->Table);

		SmartLock(MgrLock);

		vmm.Map(Address, Address, FROM_PAGES(Count), Flags);
		AllocatedPagesList.push_back({Address, Count, Protect});
		debug("%#lx +{%#lx, %lld}", this, Address, Count);
		return Address;
	}

	void VirtualMemoryArea::FreePages(void *Address, size_t Count)
	{
		func("%#lx, %lld", Address, Count);

		SmartLock(MgrLock);
		forItr(itr, AllocatedPagesList)
		{
			if (itr->Address != Address)
				continue;

			if (itr->Protected)
			{
				error("Address %#lx is protected", Address);
				return;
			}

			/** TODO: Advanced checks. Allow if the page count is less than the requested one.
			 * This will allow the user to free only a part of the allocated pages.
			 *
			 * But this will be in a separate function because we need to specify if we
			 * want to free from the start or from the end and return the new address.
			 */
			if (itr->PageCount != Count)
			{
				error("Page count mismatch! (Allocated: %lld, Requested: %lld)",
					  itr->PageCount, Count);
				return;
			}

			Virtual vmm(this->Table);
			for (size_t i = 0; i < Count; i++)
			{
				void *AddressToMap = (void *)((uintptr_t)Address + (i * PAGE_SIZE));
				vmm.Remap(AddressToMap, AddressToMap, PTFlag::RW);
			}

			KernelAllocator.FreePages(Address, Count);
			AllocatedPagesList.erase(itr);
			debug("%#lx -{%#lx, %lld}", this, Address, Count);
			return;
		}
	}

	void VirtualMemoryArea::DetachAddress(void *Address)
	{
		func("%#lx", Address);

		SmartLock(MgrLock);
		forItr(itr, AllocatedPagesList)
		{
			if (itr->Address == Address)
			{
				if (itr->Protected)
				{
					error("Address %#lx is protected", Address);
					return;
				}

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
		func("%#lx, %lld, %s, %s, %s, %s, %s", Address, Length,
			 Read ? "true" : "false",
			 Write ? "true" : "false",
			 Exec ? "true" : "false",
			 Fixed ? "true" : "false",
			 Shared ? "true" : "false");

		Virtual vmm(this->Table);

		// FIXME
		// for (uintptr_t j = uintptr_t(Address);
		// 	 j < uintptr_t(Address) + Length;
		// 	 j += PAGE_SIZE)
		// {
		// 	if (vmm.Check((void *)j, G))
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
		if (vmm.Check(Address, PTFlag::KRsv))
		{
			error("Cannot create CoW region at %#lx", Address);
			return (void *)-EPERM;
		}

		debug("unmapping %#lx-%#lx", Address, (uintptr_t)Address + Length);
		vmm.Unmap(Address, Length);
		debug("mapping cow at %#lx-%#lx", Address, (uintptr_t)Address + Length);
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
		func("%#lx", PFA);
		Virtual vmm(this->Table);
		PageTableEntry *pte = vmm.GetPTE((void *)PFA);

		debug("ctx: %#lx", this);

		if (!pte)
		{
			/* Unmapped page */
			debug("PTE is null!");
			return false;
		}

		if (!pte->CopyOnWrite)
		{
			debug("PFA %#lx is not CoW (pt %#lx) (flags %#lx)",
				  PFA, this->Table, pte->raw);
			return false;
		}

		foreach (auto sr in SharedRegions)
		{
			uintptr_t Start = (uintptr_t)sr.Address;
			uintptr_t End = (uintptr_t)sr.Address + sr.Length;
			debug("Start: %#lx, End: %#lx (PFA: %#lx)",
				  Start, End, PFA);

			if (PFA >= Start && PFA < End)
			{
				if (sr.Shared)
				{
					fixme("Shared CoW");
					return false;
				}

				void *pAddr = this->RequestPages(1);
				if (pAddr == nullptr)
					return false;
				memset(pAddr, 0, PAGE_SIZE);

				assert(pte->Present == true);
				pte->ReadWrite = sr.Write;
				pte->UserSupervisor = sr.Read;
#if defined(__amd64__)
				pte->ExecuteDisable = sr.Exec;
#endif

				pte->CopyOnWrite = false;
				debug("PFA %#lx is CoW (pt %#lx, flags %#lx)",
					  PFA, this->Table, pte->raw);
#if defined(__amd64__)
				CPU::x64::invlpg((void *)PFA);
#elif defined(__i386__)
				CPU::x32::invlpg((void *)PFA);
#endif
				return true;
			}
		}

		debug("%#lx not found in CoW regions", PFA);
		return false;
	}

	void VirtualMemoryArea::FreeAllPages()
	{
		SmartLock(MgrLock);
		foreach (auto ap in AllocatedPagesList)
		{
			KernelAllocator.FreePages(ap.Address, ap.PageCount);
			Virtual vmm(this->Table);
			for (size_t i = 0; i < ap.PageCount; i++)
				vmm.Remap((void *)((uintptr_t)ap.Address + (i * PAGE_SIZE)),
						  (void *)((uintptr_t)ap.Address + (i * PAGE_SIZE)),
						  PTFlag::RW);
		}
		AllocatedPagesList.clear();
	}

	void VirtualMemoryArea::Fork(VirtualMemoryArea *Parent)
	{
		func("%#lx", Parent);
		assert(Parent);

		debug("parent apl:%d sr:%d [P:%#lx C:%#lx]",
			  Parent->AllocatedPagesList.size(), Parent->SharedRegions.size(),
			  Parent->Table, this->Table);
		debug("ctx: this: %#lx parent: %#lx", this, Parent);

		Virtual vmm(this->Table);
		SmartLock(MgrLock);
		foreach (auto &ap in Parent->AllocatedPagesList)
		{
			if (ap.Protected)
			{
				debug("Protected %#lx-%#lx", ap.Address,
					  (uintptr_t)ap.Address + (ap.PageCount * PAGE_SIZE));
				continue; /* We don't want to modify these pages. */
			}

			MgrLock.Unlock();
			void *Address = this->RequestPages(ap.PageCount);
			MgrLock.Lock(__FUNCTION__);
			if (Address == nullptr)
				return;

			memcpy(Address, ap.Address, FROM_PAGES(ap.PageCount));

			for (size_t i = 0; i < ap.PageCount; i++)
			{
				void *AddressToMap = (void *)((uintptr_t)ap.Address + (i * PAGE_SIZE));
				void *RealAddress = (void *)((uintptr_t)Address + (i * PAGE_SIZE));

#if defined(__amd64__) || defined(__i386__)
				PageTableEntry *pte = vmm.GetPTE(AddressToMap);
				uintptr_t Flags = 0;
				Flags |= pte->Present ? (uintptr_t)PTFlag::P : 0;
				Flags |= pte->ReadWrite ? (uintptr_t)PTFlag::RW : 0;
				Flags |= pte->UserSupervisor ? (uintptr_t)PTFlag::US : 0;
				Flags |= pte->CopyOnWrite ? (uintptr_t)PTFlag::CoW : 0;
				Flags |= pte->KernelReserve ? (uintptr_t)PTFlag::KRsv : 0;

				debug("Mapping %#lx to %#lx (flags %s/%s/%s/%s/%s)",
					  RealAddress, AddressToMap,
					  Flags & PTFlag::P ? "P" : "-",
					  Flags & PTFlag::RW ? "RW" : "-",
					  Flags & PTFlag::US ? "US" : "-",
					  Flags & PTFlag::CoW ? "CoW" : "-",
					  Flags & PTFlag::KRsv ? "KRsv" : "-");
				MgrLock.Unlock();
				this->Map(AddressToMap, RealAddress, PAGE_SIZE, Flags);
				MgrLock.Lock(__FUNCTION__);
#else
#error "Not implemented"
#endif
			}

			debug("Forked %#lx-%#lx", ap.Address,
				  (uintptr_t)ap.Address + (ap.PageCount * PAGE_SIZE));
		}

		foreach (auto &sr in Parent->SharedRegions)
		{
			MgrLock.Unlock();
			void *Address = this->CreateCoWRegion(sr.Address, sr.Length,
												  sr.Read, sr.Write, sr.Exec,
												  sr.Fixed, sr.Shared);
			MgrLock.Lock(__FUNCTION__);
			if (Address == nullptr)
				return;
			memcpy(Address, sr.Address, sr.Length);
			debug("Forked CoW region %#lx-%#lx", sr.Address,
				  (uintptr_t)sr.Address + sr.Length);
		}
	}

	int VirtualMemoryArea::Map(void *VirtualAddress, void *PhysicalAddress,
							   size_t Length, uint64_t Flags)
	{
		Virtual vmm(this->Table);
		SmartLock(MgrLock);

		uintptr_t intVirtualAddress = (uintptr_t)VirtualAddress;
		uintptr_t intPhysicalAddress = (uintptr_t)PhysicalAddress;

		for (uintptr_t va = intVirtualAddress;
			 va < intVirtualAddress + Length; va += PAGE_SIZE)
		{
			if (vmm.Check(VirtualAddress, PTFlag::KRsv))
			{
				error("Virtual address %#lx is reserved", VirtualAddress);
				return -EPERM;
			}
		}

		for (uintptr_t va = intPhysicalAddress;
			 va < intPhysicalAddress + Length; va += PAGE_SIZE)
		{
			if (vmm.Check(PhysicalAddress, PTFlag::KRsv))
			{
				error("Physical address %#lx is reserved", PhysicalAddress);
				return -EPERM;
			}
		}

		vmm.Map(VirtualAddress, PhysicalAddress, Length, Flags);
		debug("Mapped %#lx-%#lx to %#lx-%#lx (flags %#lx)",
			  VirtualAddress, intVirtualAddress + Length,
			  PhysicalAddress, intPhysicalAddress + Length,
			  Flags);
		return 0;
	}

	int VirtualMemoryArea::Remap(void *VirtualAddress, void *PhysicalAddress, uint64_t Flags)
	{
		Virtual vmm(this->Table);
		SmartLock(MgrLock);

		if (vmm.Check(VirtualAddress, PTFlag::KRsv))
		{
			error("Virtual address %#lx is reserved", VirtualAddress);
			return -EPERM;
		}

		if (vmm.Check(PhysicalAddress, PTFlag::KRsv))
		{
			error("Physical address %#lx is reserved", PhysicalAddress);
			return -EPERM;
		}

		vmm.Remap(VirtualAddress, PhysicalAddress, Flags);
		debug("Remapped %#lx to %#lx (flags %#lx)",
			  VirtualAddress, PhysicalAddress, Flags);
		return 0;
	}

	int VirtualMemoryArea::Unmap(void *VirtualAddress, size_t Length)
	{
		Virtual vmm(this->Table);
		SmartLock(MgrLock);

		uintptr_t intVirtualAddress = (uintptr_t)VirtualAddress;

		for (uintptr_t va = intVirtualAddress;
			 va < intVirtualAddress + Length; va += PAGE_SIZE)
		{
			if (vmm.Check(VirtualAddress, PTFlag::KRsv))
			{
				error("Virtual address %#lx is reserved", VirtualAddress);
				return -EPERM;
			}
		}

		vmm.Unmap(VirtualAddress, Length);
		debug("Unmapped %#lx-%#lx", VirtualAddress, intVirtualAddress + Length);
		return 0;
	}

	void *VirtualMemoryArea::__UserCheckAndGetAddress(void *Address, size_t Length)
	{
		Virtual vmm(this->Table);
		SmartLock(MgrLock);

		void *pAddress = this->Table->Get(Address);
		if (pAddress == nullptr)
		{
			debug("Virtual address %#lx returns nullptr", Address);
			return nullptr;
		}

		uintptr_t intAddress = (uintptr_t)Address;
		intAddress = ALIGN_DOWN(intAddress, PAGE_SIZE);
		for (uintptr_t va = intAddress; va < intAddress + Length; va += PAGE_SIZE)
		{
			if (vmm.Check((void *)va, PTFlag::US))
				continue;

			debug("Unable to get address %#lx, page is not user accessible", va);
			return nullptr;
		}

		return pAddress;
	}

	int VirtualMemoryArea::__UserCheck(void *Address, size_t Length)
	{
		Virtual vmm(this->Table);
		SmartLock(MgrLock);

		if (vmm.Check(Address, PTFlag::US))
			return 0;

		debug("Address %#lx is not user accessible", Address);
		return -EFAULT;
	}

	VirtualMemoryArea::VirtualMemoryArea(PageTable *_Table)
		: Table(_Table)
	{
		SmartLock(MgrLock);
		if (_Table == nullptr)
		{
			if (TaskManager)
			{
				Tasking::PCB *pcb = thisProcess;
				assert(pcb);
				this->Table = thisProcess->PageTable;
			}
			else
				this->Table = (PageTable *)CPU::PageTable();
		}
	}

	VirtualMemoryArea::~VirtualMemoryArea()
	{
		/* No need to remap pages, the page table will be destroyed */

		SmartLock(MgrLock);
		foreach (auto ap in AllocatedPagesList)
			KernelAllocator.FreePages(ap.Address, ap.PageCount);
	}
}
