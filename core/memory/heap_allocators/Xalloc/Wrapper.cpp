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

#include <memory.hpp>

extern "C" void *Xalloc_REQUEST_PAGES(Xsize_t Pages)
{
	return KernelAllocator.RequestPages(Pages);
}

extern "C" void Xalloc_FREE_PAGES(void *Address, Xsize_t Pages)
{
	KernelAllocator.FreePages(Address, Pages);
}

extern "C" void Xalloc_MAP_MEMORY(void *VirtualAddress, void *PhysicalAddress, Xsize_t Flags)
{
	Memory::Virtual(KernelPageTable).Map(VirtualAddress, PhysicalAddress, Flags);
}

extern "C" void Xalloc_UNMAP_MEMORY(void *VirtualAddress)
{
	Memory::Virtual(KernelPageTable).Unmap(VirtualAddress);
}
