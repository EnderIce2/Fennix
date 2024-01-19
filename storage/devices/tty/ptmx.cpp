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
#include <filesystem/ioctl.hpp>
#include <smp.hpp>
#include <errno.h>

#include "../../../kernel.h"

namespace vfs
{
	int PTMXDevice::open(int Flags, mode_t Mode)
	{
		SmartLock(PTMXLock);
		int id = -1;
		for (size_t i = 0; i < ptysId.Size; i++)
		{
			if (unlikely(ptysId.Buffer[i] == false))
			{
				id = int(i);
				ptysId.Buffer[i] = true;
				break;
			}
		}

		if (id == -1)
			return -ENFILE;

		PTYDevice *pty = new PTYDevice(pts, id);
		ptysList.push_back(pty);
		return pty->OpenMaster(Flags, Mode);
	}

	void PTMXDevice::RemovePTY(int fd, Tasking::PCB *pcb)
	{
		SmartLock(PTMXLock);
		if (!pcb)
			pcb = thisProcess;
		assert(pcb != nullptr);

		FileDescriptorTable *fdt = pcb->FileDescriptors;
		RefNode *node = fdt->GetRefNode(fd);

		assert(node->SpecialData != nullptr);
		PTYDevice *pty = (PTYDevice *)node->SpecialData;
		int id = pty->ptyId;

		forItr(itr, ptysList)
		{
			if (*itr != pty)
				continue;

			ptysList.erase(itr);
			delete *itr;
			break;
		}
		ptysId.Buffer[id] = false;
	}

	PTMXDevice::PTMXDevice() : Node(DevFS, "ptmx", CHARDEVICE)
	{
		this->Mode = 0644;
		this->UserIdentifier = 0;
		this->GroupIdentifier = 0;
		pts = new Node(DevFS, "pts", DIRECTORY);

		ptysId.Buffer = new uint8_t[0x1000];
		ptysId.Size = 0x1000;
	}

	PTMXDevice::~PTMXDevice()
	{
		SmartLock(PTMXLock);
		delete pts;
		delete[] ptysId.Buffer;
	}
}
