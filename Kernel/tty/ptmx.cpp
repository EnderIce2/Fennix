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
#include <smp.hpp>
#include <errno.h>

#include "../kernel.h"

namespace TTY
{
	PTMXDevice::PTMXDevice()
	{
	}

	PTMXDevice::~PTMXDevice()
	{
		for (auto pty : PTYs)
			delete pty;
	}

	int PTMXDevice::Open()
	{
		stub;
		return -ENOSYS;
	}

	int PTMXDevice::Close()
	{
		stub;
		return -ENOSYS;
	}

	PTYDevice *PTMXDevice::CreatePTY()
	{
		std::lock_guard<std::mutex> lock(PTYMutex);
		PTYDevice *pty = new PTYDevice();
		PTYs.push_back(pty);
		return pty;
	}

	int PTMXDevice::RemovePTY(PTYDevice *pty)
	{
		std::lock_guard<std::mutex> lock(PTYMutex);
		auto it = PTYs.erase(std::remove(PTYs.begin(), PTYs.end(), pty), PTYs.end());
		if (it == PTYs.end())
			return -ENOENT;
		delete pty;
		return 0;
	}
}
