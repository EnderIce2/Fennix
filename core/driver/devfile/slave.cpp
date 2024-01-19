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

#include <driver.hpp>

#include <memory.hpp>
#include <ints.hpp>
#include <task.hpp>
#include <printf.h>
#include <exec.hpp>
#include <cwalk.h>
#include <md5.h>

#include "../../../kernel.h"
#include "../../../driver.h"

using namespace vfs;

namespace Driver
{
	int SlaveDeviceFile::open(int Flags, mode_t Mode)
	{
		switch (this->DeviceType)
		{
		default:
			if (this->Open)
				return this->Open(this->DeviceMajor, this->DeviceMinor,
								  Flags, Mode);
			return -ENOSYS;
		}
	}

	int SlaveDeviceFile::close()
	{
		switch (this->DeviceType)
		{
		default:
			if (this->Close)
				return this->Close(this->DeviceMajor, this->DeviceMinor);
			return -ENOSYS;
		}
	}

	size_t SlaveDeviceFile::read(uint8_t *Buffer,
								 size_t Size,
								 off_t Offset)
	{
		switch (this->DeviceType)
		{
		case ddt_Keyboard:
		{
			while (KeyQueue.empty())
				TaskManager->Yield();

			Buffer[0] = KeyQueue.front();
			KeyQueue.pop_front();
			return 1;
		}
		default:
			if (this->Read)
				return this->Read(this->DeviceMajor, this->DeviceMinor,
								  Buffer, Size, Offset);
			return 0;
		}
	}

	size_t SlaveDeviceFile::write(uint8_t *Buffer,
								  size_t Size,
								  off_t Offset)
	{
		switch (this->DeviceType)
		{
		default:
			if (this->Write)
				return this->Write(this->DeviceMajor, this->DeviceMinor,
								   Buffer, Size, Offset);
			return 0;
		}
	}

	int SlaveDeviceFile::ioctl(unsigned long Request,
							   void *Argp)
	{
		switch (this->DeviceType)
		{
		default:
			if (this->Ioctl)
				return this->Ioctl(this->DeviceMajor, this->DeviceMinor,
								   Request, Argp);
			return -ENOSYS;
		}
	}

	void SlaveDeviceFile::ClearBuffers()
	{
		KeyQueue.clear();
		/* ... */
	}

	int SlaveDeviceFile::ReportKeyEvent(uint8_t ScanCode)
	{
		if (KeyQueue.size() > 16)
			KeyQueue.pop_front();
		KeyQueue.push_back(ScanCode);
		return 0;
	}

	SlaveDeviceFile::SlaveDeviceFile(const char *Name, vfs::Node *Parent, int Type, vfs::NodeType NType)
		: Node(Parent, Name, NType)
	{
		this->DeviceType = Type;
	}

	SlaveDeviceFile::~SlaveDeviceFile()
	{
	}
}
