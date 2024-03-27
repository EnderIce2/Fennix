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
	int MasterDeviceFile::open(int Flags, mode_t Mode)
	{
		switch (this->DeviceType)
		{
		default:
			if (this->SlavesMap.empty())
				return -ENOSYS;
			Slaves slave = this->SlavesMap.begin()->second;
			return slave->begin()->second->open(Flags, Mode);
		}
	}

	int MasterDeviceFile::close()
	{
		switch (this->DeviceType)
		{
		default:
			if (this->SlavesMap.empty())
				return -ENOSYS;
			Slaves slave = this->SlavesMap.begin()->second;
			return slave->begin()->second->close();
		}
	}

	size_t MasterDeviceFile::read(uint8_t *Buffer,
								  size_t Size,
								  off_t Offset)
	{
		switch (this->DeviceType)
		{
		case ddt_Keyboard:
		{
			while (KeyQueue.empty())
				TaskManager->Yield();

			/* Request scancode */
			if (Size == 2 && Buffer[1] == 0x00)
			{
				if (RawKeyQueue.empty())
					return 0;

				Buffer[0] = RawKeyQueue.front();
				RawKeyQueue.pop_front();
				return 1;
			}

			Buffer[0] = KeyQueue.front();
			KeyQueue.pop_front();
			return 1;
		}
		default:
			if (this->SlavesMap.empty())
				return 0;
			Slaves slave = this->SlavesMap.begin()->second;
			return slave->begin()->second->read(Buffer, Size, Offset);
		}
	}

	size_t MasterDeviceFile::write(uint8_t *Buffer,
								   size_t Size,
								   off_t Offset)
	{
		switch (this->DeviceType)
		{
		default:
			if (this->SlavesMap.empty())
				return 0;
			Slaves slave = this->SlavesMap.begin()->second;
			return slave->begin()->second->write(Buffer, Size, Offset);
		}
	}

	int MasterDeviceFile::ioctl(unsigned long Request,
								void *Argp)
	{
		switch (this->DeviceType)
		{
		default:
			if (this->SlavesMap.empty())
				return -ENOSYS;
			Slaves slave = this->SlavesMap.begin()->second;
			return slave->begin()->second->ioctl(Request, Argp);
		}
	}

	void MasterDeviceFile::ClearBuffers()
	{
		this->RawKeyQueue.clear();
		this->KeyQueue.clear();
		/* ... */

		foreach (auto &sm in this->SlavesMap)
		{
			Slaves slave = sm.second;
			foreach (auto &sdf in *slave)
				sdf.second->ClearBuffers();
		}
	}

	int MasterDeviceFile::ReportKeyEvent(maj_t ID, min_t MinorID, uint8_t ScanCode)
	{
		debug("New key event: %02x", ScanCode);
		if (this->SlavesMap.find(ID) == this->SlavesMap.end())
			return -EINVAL;

		std::unordered_map<min_t, SlaveDeviceFile *> *slave = this->SlavesMap[ID];
		if ((*slave).find(MinorID) == (*slave).end())
			return -EINVAL;

		/* We are master, keep a copy of the scancode and
			converted key */

		if (RawKeyQueue.size() > 16)
			RawKeyQueue.pop_front();
		RawKeyQueue.push_back(ScanCode);

		if (KeyQueue.size() > 16)
			KeyQueue.pop_front();

		switch (ScanCode & ~KEY_PRESSED)
		{
		case KEY_LEFT_SHIFT:
		case KEY_RIGHT_SHIFT:
		{
			if (ScanCode & KEY_PRESSED)
				UpperCase = true;
			else
				UpperCase = false;
			break;
		}
		case KEY_CAPS_LOCK:
		{
			if (ScanCode & KEY_PRESSED)
				CapsLock = !CapsLock;
			break;
		}
		default:
			break;
		}

		if (ScanCode & KEY_PRESSED)
			KeyQueue.push_back(GetScanCode(ScanCode, UpperCase || CapsLock));

		SlaveDeviceFile *sdf = (*slave)[MinorID];
		return sdf->ReportKeyEvent(ScanCode);
	}

	int MasterDeviceFile::ReportMouseEvent(maj_t ID, min_t MinorID,
										   bool LeftButton, bool RightButton, bool MiddleButton,
										   bool Button4, bool Button5, bool Button6, bool Button7, bool Button8,
										   uintptr_t X, uintptr_t Y, int8_t Z, bool Relative)
	{
		return -ENOSYS;
	}

	int MasterDeviceFile::ReportNetworkPacket(maj_t ID, min_t MinorID, void *Buffer, size_t Size)
	{
		/* TODO: Buffer must be allocated by the kernel */
		return -ENOSYS;
	}

	int MasterDeviceFile::NewBlock(maj_t ID, min_t MinorID, drvOpen_t Open, drvClose_t Close,
								   drvRead_t Read, drvWrite_t Write, drvIoctl_t Ioctl)
	{
		assert(this->SlavesMap.find(ID) != this->SlavesMap.end());
		Slaves slave = this->SlavesMap[ID];
		assert((*slave).find(MinorID) != (*slave).end());
		SlaveDeviceFile *sdf = (*slave)[MinorID];
		sdf->Open = Open;
		sdf->Close = Close;
		sdf->Read = Read;
		sdf->Write = Write;
		sdf->Ioctl = Ioctl;
		return 0;
	}

	int MasterDeviceFile::NewAudio(maj_t ID, min_t MinorID, drvOpen_t Open, drvClose_t Close,
								   drvRead_t Read, drvWrite_t Write, drvIoctl_t Ioctl)
	{
		assert(this->SlavesMap.find(ID) != this->SlavesMap.end());
		Slaves slave = this->SlavesMap[ID];
		assert((*slave).find(MinorID) != (*slave).end());
		SlaveDeviceFile *sdf = (*slave)[MinorID];
		sdf->Open = Open;
		sdf->Close = Close;
		sdf->Read = Read;
		sdf->Write = Write;
		sdf->Ioctl = Ioctl;
		return 0;
	}

	int MasterDeviceFile::NewNet(maj_t ID, min_t MinorID, drvOpen_t Open, drvClose_t Close,
								 drvRead_t Read, drvWrite_t Write, drvIoctl_t Ioctl)
	{
		assert(this->SlavesMap.find(ID) != this->SlavesMap.end());
		Slaves slave = this->SlavesMap[ID];
		assert((*slave).find(MinorID) != (*slave).end());
		SlaveDeviceFile *sdf = (*slave)[MinorID];
		sdf->Open = Open;
		sdf->Close = Close;
		sdf->Read = Read;
		sdf->Write = Write;
		sdf->Ioctl = Ioctl;
		return 0;
	}

	dev_t MasterDeviceFile::Register(maj_t ID)
	{
		debug("Registering slave device %d", ID);
		Slaves slave;
		if (this->SlavesMap.find(ID) != this->SlavesMap.end())
			slave = this->SlavesMap[ID];
		else
			slave = new std::unordered_map<min_t, SlaveDeviceFile *>();

		char name[24];
		sprintf(name, "%s%ld", this->SlaveName, this->SlaveIDCounter);
		SlaveDeviceFile *sdf = new SlaveDeviceFile(name,
												   this->SlaveParent,
												   this->DeviceType,
												   this->Type);

		sdf->DeviceMajor = ID;
		sdf->DeviceMinor = this->SlaveIDCounter;

		(*slave)[this->SlaveIDCounter] = sdf;
		this->SlavesMap[ID] = slave;
		return this->SlaveIDCounter++;
	}

	int MasterDeviceFile::Unregister(maj_t ID, min_t MinorID)
	{
		debug("Unregistering slave device %d:%d", ID, MinorID);
		if (this->SlavesMap.find(ID) == this->SlavesMap.end())
			return -EINVAL;

		std::unordered_map<min_t, SlaveDeviceFile *> *slave = this->SlavesMap[ID];
		if ((*slave).find(MinorID) == (*slave).end())
			return -EINVAL;

		SlaveDeviceFile *sdf = (*slave)[MinorID];
		delete sdf;
		slave->erase(MinorID);
		if (slave->empty())
		{
			delete slave;
			this->SlavesMap.erase(ID);
		}
		return 0;
	}

	MasterDeviceFile::MasterDeviceFile(const char *MasterName,
									   const char *_SlaveName,
									   Node *Parent,
									   int Type)
		: Node(Parent, MasterName, NodeType::FILE)
	{
		strncpy(this->SlaveName, _SlaveName, sizeof(this->Name));
		this->DeviceType = Type;
		this->SlaveParent = Parent;

		switch (Type)
		{
		case ddt_Keyboard:
		case ddt_Mouse:
		case ddt_Joystick:
		case ddt_Gamepad:
		case ddt_Touchpad:
		case ddt_Touchscreen:
			this->Type = NodeType::CHARDEVICE;
			break;
		case ddt_SATA:
		case ddt_ATA:
		case ddt_NVMe:
			this->Type = NodeType::BLOCKDEVICE;
			break;
		default:
			break;
		}
	}

	MasterDeviceFile::~MasterDeviceFile()
	{
		foreach (auto &sm in this->SlavesMap)
		{
			Slaves slave = sm.second;
			foreach (auto &sdf in *slave)
				delete sdf.second;
			delete slave;
		}
		this->SlavesMap.clear();
	}
}
