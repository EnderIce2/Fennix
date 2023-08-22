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

#include "Xalloc.hpp"

XallocV2_def;

#define XALLOC_CONCAT(x, y) x##y
#define XStoP(d) (((d) + PAGE_SIZE - 1) / PAGE_SIZE)
#define XPtoS(d) ((d)*PAGE_SIZE)

extern "C" void *Xalloc_REQUEST_PAGES(Xsize_t Pages);
extern "C" void Xalloc_FREE_PAGES(void *Address, Xsize_t Pages);
extern "C" void Xalloc_MAP_MEMORY(void *VirtualAddress,
								  void *PhysicalAddress,
								  Xsize_t Flags);
extern "C" void Xalloc_UNMAP_MEMORY(void *VirtualAddress);

#define Xalloc_BlockSanityKey 0xA110C

/*
	[ IN DEVELOPMENT ]
*/

namespace Xalloc
{
	void V2::Block::Check()
	{
		if (unlikely(this->Sanity != Xalloc_BlockSanityKey))
		{
			Xalloc_err("Block %#lx has an invalid sanity key! (%#x != %#x)",
					   this, this->Sanity, Xalloc_BlockSanityKey);

			while (Xalloc_StopOnFail)
				;
		}
	}

	V2::Block::Block(Xsize_t Size, V2 *ctx)
	{
		this->ctx = ctx;
		this->Data = ctx->AllocateHeap(Size);
		this->DataSize = Size;
	}

	V2::Block::~Block()
	{
	}

	void *V2::Block::operator new(Xsize_t)
	{
		constexpr Xsize_t bPgs = XStoP(sizeof(Block));
		void *ptr = Xalloc_REQUEST_PAGES(bPgs);
		/* TODO: Do something with the rest of
			the allocated memory */
		return ptr;
	}

	void V2::Block::operator delete(void *Address)
	{
		constexpr Xsize_t bPgs = XStoP(sizeof(Block));
		Xalloc_FREE_PAGES(Address, bPgs);
	}

	/* ========================================= */

	Xuint8_t *V2::AllocateHeap(Xsize_t Size)
	{
		Size = this->Align(Size);
		Xsize_t Pages = XStoP(Size);

		Xuint8_t *FinalAddress = 0x0;
		if (this->HeapUsed + Size >= this->HeapSize)
		{
			void *Address = Xalloc_REQUEST_PAGES(Pages);
			void *VirtualAddress = (void *)(this->BaseVirtualAddress + this->HeapSize);
			if (Xalloc_MapPages)
			{
				for (Xsize_t i = 0; i < Pages; i++)
				{
					Xuintptr_t Page = i * Xalloc_PAGE_SIZE;
					void *vAddress = (void *)((Xuintptr_t)VirtualAddress + Page);
					Xalloc_MAP_MEMORY(vAddress, (void *)((Xuintptr_t)Address + Page), 0x3);
				}
			}

			this->HeapSize += XPtoS(Pages);
			FinalAddress = (Xuint8_t *)VirtualAddress;
		}
		else
			FinalAddress = (Xuint8_t *)(this->BaseVirtualAddress + this->HeapUsed);

		this->HeapUsed += Size;
		return (uint8_t *)FinalAddress;
	}

	void V2::FreeHeap(Xuint8_t *At, Xsize_t Size)
	{
		Xsize_t Pages = XStoP(Size);

		if (Xalloc_MapPages)
		{
			for (Xsize_t i = 0; i < Pages; i++)
			{
				Xuintptr_t Page = i * Xalloc_PAGE_SIZE;
				void *VirtualAddress = (void *)((Xuintptr_t)At + Page);
				Xalloc_UNMAP_MEMORY(VirtualAddress);
			}
		}

		Xalloc_FREE_PAGES(At, Pages);
		this->HeapUsed -= Size;
	}

	Xsize_t V2::Align(Xsize_t Size)
	{
		return (Size + 0xF) & ~0xF;
	}

	void *V2::FindFreeBlock(Xsize_t Size, Block *&CurrentBlock)
	{
		if (this->FirstBlock == nullptr)
		{
			this->FirstBlock = new Block(Size, this);
			this->FirstBlock->IsFree = false;
			return this->FirstBlock->Data;
		}

		while (true)
		{
			CurrentBlock->Check();

			/* FIXME: This will waste a lot of space
				need better algorithm */
			if (CurrentBlock->IsFree &&
				CurrentBlock->DataSize >= Size)
			{
				CurrentBlock->IsFree = false;
				return CurrentBlock->Data;
			}

			if (CurrentBlock->Next == nullptr)
				break;

			CurrentBlock = CurrentBlock->Next;
		}

		return nullptr;
	}

	void V2::Arrange()
	{
		Xalloc_err("Arrange() is not implemented yet!");
	}

	void *V2::malloc(Xsize_t Size)
	{
		if (Size == 0)
		{
			Xalloc_warn("Attempted to allocate 0 bytes!");
			return nullptr;
		}

		XallocV2_lock;
		Block *CurrentBlock = this->FirstBlock;
		void *ret = this->FindFreeBlock(Size, CurrentBlock);
		if (ret)
		{
			XallocV2_unlock;
			return ret;
		}

		CurrentBlock->Next = new Block(Size, this);
		CurrentBlock->Next->IsFree = false;
		XallocV2_unlock;
		return CurrentBlock->Next->Data;
	}

	void V2::free(void *Address)
	{
		if (Address == nullptr)
		{
			Xalloc_warn("Attempted to free a null pointer!");
			return;
		}

		XallocV2_lock;

		Block *CurrentBlock = ((Block *)this->FirstBlock);
		while (CurrentBlock != nullptr)
		{
			CurrentBlock->Check();

			if (CurrentBlock->Data == Address)
			{
				if (CurrentBlock->IsFree)
					Xalloc_warn("Attempted to free an already freed block! %#lx", Address);

				CurrentBlock->IsFree = true;
				XallocV2_unlock;
				return;
			}
			CurrentBlock = CurrentBlock->Next;
		}

		Xalloc_err("Invalid address %#lx.", Address);
		XallocV2_unlock;
	}

	void *V2::calloc(Xsize_t NumberOfBlocks, Xsize_t Size)
	{
		if (NumberOfBlocks == 0 || Size == 0)
		{
			Xalloc_warn("The %s%s%s is 0!",
						NumberOfBlocks == 0 ? "NumberOfBlocks" : "",
						NumberOfBlocks == 0 && Size == 0 ? " and " : "",
						Size == 0 ? "Size" : "");
			return nullptr;
		}

		return this->malloc(NumberOfBlocks * Size);
	}

	void *V2::realloc(void *Address, Xsize_t Size)
	{
		if (Address == nullptr && Size != 0)
			return this->malloc(Size);

		if (Size == 0)
		{
			this->free(Address);
			return nullptr;
		}

		// XallocV2_lock;
		// ...
		// XallocV2_unlock;

		// TODO: Implement realloc
		static int once = 0;
		if (!once++)
			Xalloc_trace("realloc is stub!");
		this->free(Address);
		return this->malloc(Size);
	}

	V2::V2(void *VirtualBase)
	{
		if (VirtualBase == 0x0 && Xalloc_MapPages)
		{
			Xalloc_err("VirtualBase is 0x0 and Xalloc_MapPages is true!");
			while (true)
				;
		}

		XallocV2_lock;
		this->BaseVirtualAddress = Xuintptr_t(VirtualBase);
		XallocV2_unlock;
	}

	V2::~V2()
	{
		XallocV2_lock;
		Xalloc_trace("Destructor not implemented yet.");
		XallocV2_unlock;
	}
}
