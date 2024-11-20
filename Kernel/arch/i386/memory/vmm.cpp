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

#include <convert.h>
#include <debug.h>

namespace Memory
{
	bool Virtual::Check(void *VirtualAddress, PTFlag Flag, MapType Type)
	{
		// 0x1000 aligned
		uintptr_t Address = (uintptr_t)VirtualAddress;
		Address &= 0xFFFFF000;

		PageMapIndexer Index = PageMapIndexer(Address);
		PageDirectoryEntry *PDE = &this->Table->Entries[Index.PDEIndex];
		PageTableEntryPtr *PTE = nullptr;

		if ((PDE->raw & Flag) > 0)
		{
			if (Type == MapType::FourMiB && PDE->PageSize)
				return true;

			PTE = (PageTableEntryPtr *)((uintptr_t)PDE->GetAddress() << 12);
			if (PTE)
			{
				if ((PTE->Entries[Index.PTEIndex].Present))
					return true;
			}
		}
		return false;
	}

	void *Virtual::GetPhysical(void *VirtualAddress)
	{
		// 0x1000 aligned
		uintptr_t Address = (uintptr_t)VirtualAddress;
		Address &= 0xFFFFF000;

		PageMapIndexer Index = PageMapIndexer(Address);
		PageDirectoryEntry *PDE = &this->Table->Entries[Index.PDEIndex];
		PageTableEntryPtr *PTE = nullptr;

		if (PDE->Present)
		{
			if (PDE->PageSize)
				return (void *)((uintptr_t)PDE->GetAddress() << 12);

			PTE = (PageTableEntryPtr *)((uintptr_t)PDE->GetAddress() << 12);
			if (PTE)
			{
				if (PTE->Entries[Index.PTEIndex].Present)
					return (void *)((uintptr_t)PTE->Entries[Index.PTEIndex].GetAddress() << 12);
			}
		}
		return nullptr;
	}

	Virtual::MapType Virtual::GetMapType(void *VirtualAddress)
	{
		// 0x1000 aligned
		uintptr_t Address = (uintptr_t)VirtualAddress;
		Address &= 0xFFFFF000;

		PageMapIndexer Index = PageMapIndexer(Address);

		PageDirectoryEntry *PDE = &this->Table->Entries[Index.PDEIndex];
		PageTableEntryPtr *PTE = nullptr;

		if (PDE->Present)
		{
			if (PDE->PageSize)
				return MapType::FourMiB;

			PTE = (PageTableEntryPtr *)((uintptr_t)PDE->GetAddress() << 12);
			if (PTE)
			{
				if (PTE->Entries[Index.PTEIndex].Present)
					return MapType::FourKiB;
			}
		}
		return MapType::NoMapType;
	}

	PageDirectoryEntry *Virtual::GetPDE(void *VirtualAddress, MapType Type)
	{
		uintptr_t Address = (uintptr_t)VirtualAddress;
		Address &= 0xFFFFF000;

		PageMapIndexer Index = PageMapIndexer(Address);
		PageDirectoryEntry *PDE = &this->Table->Entries[Index.PDEIndex];
		if (PDE->Present)
			return PDE;
		return nullptr;
	}

	PageTableEntry *Virtual::GetPTE(void *VirtualAddress, MapType Type)
	{
		uintptr_t Address = (uintptr_t)VirtualAddress;
		Address &= 0xFFFFF000;

		PageMapIndexer Index = PageMapIndexer(Address);
		PageDirectoryEntry *PDE = &this->Table->Entries[Index.PDEIndex];
		if (!PDE->Present)
			return nullptr;

		PageTableEntryPtr *PTEPtr = (PageTableEntryPtr *)(PDE->GetAddress() << 12);
		PageTableEntry *PTE = &PTEPtr->Entries[Index.PTEIndex];
		if (PTE->Present)
			return PTE;
		return nullptr;
	}

	void Virtual::Map(void *VirtualAddress, void *PhysicalAddress, uint64_t Flags, MapType Type)
	{
		SmartLock(this->MemoryLock);
		if (unlikely(!this->Table))
		{
			error("No page table");
			return;
		}

		Flags |= PTFlag::P;

		PageMapIndexer Index = PageMapIndexer((uintptr_t)VirtualAddress);
		// Clear any flags that are not 1 << 0 (Present) - 1 << 5 (Accessed) because rest are for page table entries only
		uint64_t DirectoryFlags = Flags & 0x3F;

		PageDirectoryEntry *PDE = &this->Table->Entries[Index.PDEIndex];
		if (Type == MapType::FourMiB)
		{
			PDE->raw |= (uintptr_t)Flags;
			PDE->PageSize = true;
			PDE->SetAddress((uintptr_t)PhysicalAddress >> 12);
			debug("Mapped 4MB page at %p to %p", VirtualAddress, PhysicalAddress);
			return;
		}

		PageTableEntryPtr *PTEPtr = nullptr;
		if (!PDE->Present)
		{
			PTEPtr = (PageTableEntryPtr *)KernelAllocator.RequestPages(TO_PAGES(sizeof(PageTableEntryPtr) + 1));
			memset(PTEPtr, 0, sizeof(PageTableEntryPtr));
			PDE->Present = true;
			PDE->SetAddress((uintptr_t)PTEPtr >> 12);
		}
		else
			PTEPtr = (PageTableEntryPtr *)(PDE->GetAddress() << 12);
		PDE->raw |= (uintptr_t)DirectoryFlags;

		PageTableEntry *PTE = &PTEPtr->Entries[Index.PTEIndex];
		PTE->Present = true;
		PTE->raw |= (uintptr_t)Flags;
		PTE->SetAddress((uintptr_t)PhysicalAddress >> 12);
		CPU::x32::invlpg(VirtualAddress);

#ifdef DEBUG
/* https://stackoverflow.com/a/3208376/9352057 */
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)       \
	(byte & 0x80 ? '1' : '0'),     \
		(byte & 0x40 ? '1' : '0'), \
		(byte & 0x20 ? '1' : '0'), \
		(byte & 0x10 ? '1' : '0'), \
		(byte & 0x08 ? '1' : '0'), \
		(byte & 0x04 ? '1' : '0'), \
		(byte & 0x02 ? '1' : '0'), \
		(byte & 0x01 ? '1' : '0')

		if (!this->Check(VirtualAddress, (PTFlag)Flags, Type)) // quick workaround just to see where it fails
			warn("Failed to map v:%#lx p:%#lx with flags: " BYTE_TO_BINARY_PATTERN, VirtualAddress, PhysicalAddress, BYTE_TO_BINARY(Flags));
#endif
	}

	void Virtual::Unmap(void *VirtualAddress, MapType Type)
	{
		SmartLock(this->MemoryLock);
		if (!this->Table)
		{
			error("No page table");
			return;
		}

		PageMapIndexer Index = PageMapIndexer((uintptr_t)VirtualAddress);
		PageDirectoryEntry *PDE = &this->Table->Entries[Index.PDEIndex];
		if (!PDE->Present)
		{
			warn("Page %#lx not present", PDE->GetAddress());
			return;
		}

		if (Type == MapType::FourMiB && PDE->PageSize)
		{
			PDE->Present = false;
			return;
		}

		PageTableEntryPtr *PTEPtr = (PageTableEntryPtr *)((uintptr_t)PDE->Address << 12);
		PageTableEntry PTE = PTEPtr->Entries[Index.PTEIndex];
		if (!PTE.Present)
		{
			warn("Page %#lx not present", PTE.GetAddress());
			return;
		}

		PTE.Present = false;
		PTEPtr->Entries[Index.PTEIndex] = PTE;
		CPU::x32::invlpg(VirtualAddress);
	}
}
