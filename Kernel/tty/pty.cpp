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

#include <tty.hpp>
#include <filesystem/ioctl.hpp>
#include <string.h>
#include <errno.h>

#include "../kernel.h"

namespace TTY
{
	PTYDevice::PTYDevice()
		: Master(), Slave()
	{
	}

	PTYDevice::~PTYDevice()
	{
	}

	int PTYDevice::Open()
	{
		stub;
		return -ENOSYS;
	}

	int PTYDevice::Close()
	{
		stub;
		return -ENOSYS;
	}

	ssize_t PTYDevice::Read(void *Buffer, size_t Size)
	{
		return Slave.Read(Buffer, Size);
	}

	ssize_t PTYDevice::Write(const void *Buffer, size_t Size)
	{
		return Master.Write(Buffer, Size);
	}

	PTYDevice::PTYMaster::PTYMaster()
		: TermBuf(1024)
	{
	}

	PTYDevice::PTYMaster::~PTYMaster()
	{
	}

	ssize_t PTYDevice::PTYMaster::Read(void *Buffer, size_t Size)
	{
		return TermBuf.Read((char *)Buffer, Size);
	}

	ssize_t PTYDevice::PTYMaster::Write(const void *Buffer, size_t Size)
	{
		return TermBuf.Write((const char *)Buffer, Size);
	}

	PTYDevice::PTYSlave::PTYSlave()
		: TermBuf(1024)
	{
	}

	PTYDevice::PTYSlave::~PTYSlave()
	{
	}

	ssize_t PTYDevice::PTYSlave::Read(void *Buffer, size_t Size)
	{
		return TermBuf.Read((char *)Buffer, Size);
	}

	ssize_t PTYDevice::PTYSlave::Write(const void *Buffer, size_t Size)
	{
		return TermBuf.Write((const char *)Buffer, Size);
	}
}
