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

#include <filesystem/mounts.hpp>
#include <rand.hpp>
#include <errno.h>

#include "../../kernel.h"

namespace vfs
{
	size_t RandomDevice::read(uint8_t *Buffer, size_t Size, off_t Offset)
	{
		if (Size <= 0)
			return 0;

		uint64_t *buf = (uint64_t *)Buffer;
		for (size_t i = 0; i < Size / sizeof(uint64_t); i++)
			buf[i] = Random::rand64();
		return Size;
	}

	size_t RandomDevice::write(uint8_t *Buffer, size_t Size, off_t Offset)
	{
		return Size;
	}

	RandomDevice::RandomDevice() : Node(DevFS, "random", CHARDEVICE) {}
	RandomDevice::~RandomDevice() {}
}
