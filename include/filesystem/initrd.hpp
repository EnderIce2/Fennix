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

#ifndef __FENNIX_KERNEL_FILESYSTEM_INITRD_H__
#define __FENNIX_KERNEL_FILESYSTEM_INITRD_H__

#include <types.h>

#include <filesystem.hpp>

namespace vfs
{
    class Initrd
    {
    public:
        struct InitrdHeader
        {
            uint32_t nfiles;
        };

        struct InitrdFileHeader
        {
            uint8_t magic;
            char name[64];
            uint32_t offset;
            uint32_t length;
        };

        Initrd(uintptr_t Address);
        ~Initrd();
    };
}

#endif // !__FENNIX_KERNEL_FILESYSTEM_INITRD_H__
