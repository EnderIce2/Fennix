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

namespace Memory
{
	Virtual::PageMapIndexer::PageMapIndexer(uintptr_t VirtualAddress)
	{
		uintptr_t Address = VirtualAddress;
#if defined(__amd64__)
		Address >>= 12;
		this->PTEIndex = Address & 0x1FF;
		Address >>= 9;
		this->PDEIndex = Address & 0x1FF;
		Address >>= 9;
		this->PDPTEIndex = Address & 0x1FF;
		Address >>= 9;
		this->PMLIndex = Address & 0x1FF;
#elif defined(__i386__)
		Address >>= 12;
		this->PTEIndex = Address & 0x3FF;
		Address >>= 10;
		this->PDEIndex = Address & 0x3FF;
#else
#warning "not implemented"
UNUSED(Address);
#endif

		if (VirtualAddress > PAGE_SIZE)
		{
			assert(
				this->PTEIndex != 0 ||
				this->PDEIndex != 0
#if defined(__amd64__)
				|| this->PDPTEIndex != 0 ||
				this->PMLIndex != 0
#endif
			);
		}
	}
}
