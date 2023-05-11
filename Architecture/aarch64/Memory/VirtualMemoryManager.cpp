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
    }

    void *Virtual::GetPhysical(void *VirtualAddress)
    {
    }

    void Virtual::Map(void *VirtualAddress, void *PhysicalAddress, uint64_t Flags, MapType Type)
    {
    }

    void Virtual::Unmap(void *VirtualAddress, MapType Type)
    {
    }
}
