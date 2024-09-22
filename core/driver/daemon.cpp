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

#include <interface/driver.h>
#include <interface/input.h>
#include <memory.hpp>
#include <ints.hpp>
#include <task.hpp>
#include <printf.h>
#include <exec.hpp>
#include <rand.hpp>
#include <cwalk.h>
#include <md5.h>

#include "../../kernel.h"

using namespace vfs;

namespace Driver
{
	/**
	 * maj = 0
	 *	min:
	 *		0 - <ROOT>
	 *		1 - /proc/self
	 *		2 - /dev/null
	 *		3 - /dev/zero
	 *		4 - /dev/random
	 *		5 - /dev/mem
	 *		6 - /dev/kcon
	 *
	 * maj = 1
	 * 	min:
	 * 		0 - /dev/input/keyboard
	 * 		1 - /dev/input/mouse
	 * 		..- /dev/input/eventX
	 */

	int __fs_Lookup(struct Inode *_Parent, const char *Name, struct Inode **Result)
	{
		auto Parent = (Manager::DeviceInode *)_Parent;

		const char *basename;
		size_t length;
		cwk_path_get_basename(Name, &basename, &length);
		if (basename == NULL)
		{
			error("Invalid name %s", Name);
			return -EINVAL;
		}

		for (const auto &child : Parent->Children)
		{
			debug("Comparing %s with %s", child->Name.c_str(), basename);
			if (strcmp(child->Name.c_str(), basename) != 0)
				continue;

			*Result = &child->Node;
			return 0;
		}

		debug("Not found %s", Name);
		return -ENOENT;
	}

	int __fs_Create(struct Inode *_Parent, const char *Name, mode_t Mode, struct Inode **Result)
	{
		assert(_Parent != nullptr);

		/* We expect to be /dev or children of it */
		auto Parent = (Manager::DeviceInode *)_Parent;
		auto _dev = new Manager::DeviceInode;
		_dev->Parent = nullptr;
		_dev->ParentInode = _Parent;
		_dev->Name = Name;
		_dev->Node.Mode = Mode;
		_dev->Node.Index = Parent->Node.Index + Parent->Children.size();
		Parent->Children.push_back(_dev);

		*Result = &_dev->Node;
		return 0;
	}

	ssize_t __fs_Read(struct Inode *Node, void *Buffer, size_t Size, off_t Offset)
	{
		switch (Node->GetMajor())
		{
		case 0:
		{
			switch (Node->GetMinor())
			{
			case 2: /* /dev/null */
			{
				return 0;
			}
			case 3: /* /dev/zero */
			{
				if (Size <= 0)
					return 0;

				memset(Buffer, 0, Size);
				return Size;
			}
			case 4: /* /dev/random */
			{
				if (Size <= 0)
					return 0;

				if (Size < sizeof(uint64_t))
				{
					uint8_t *buf = (uint8_t *)Buffer;
					for (size_t i = 0; i < Size; i++)
						buf[i] = (uint8_t)(Random::rand16() & 0xFF);
					return Size;
				}

				uint64_t *buf = (uint64_t *)Buffer;
				for (size_t i = 0; i < Size / sizeof(uint64_t); i++)
					buf[i] = Random::rand64();
				return Size;
			}
			case 5: /* /dev/mem */
			{
				stub;
				return 0;
			}
			case 6: /* /dev/kcon */
			{
				stub;
				return 0;
			}
			default:
				return -ENOENT;
			};
			break;
		}
		case 1:
		{
			switch (Node->GetMinor())
			{
			case 0: /* /dev/input/keyboard */
			{
				if (Size < sizeof(KeyboardReport))
					return -EINVAL;

				size_t nReads = Size / sizeof(KeyboardReport);

				KeyboardReport *report = (KeyboardReport *)Buffer;

				while (DriverManager->GlobalKeyboardInputReports.Count() == 0)
					TaskManager->Yield();

				DriverManager->GlobalKeyboardInputReports.Read(report, nReads);
				return sizeof(KeyboardReport) * nReads;
			}
			case 1: /* /dev/input/mouse */
			{
				if (Size < sizeof(MouseReport))
					return -EINVAL;

				size_t nReads = Size / sizeof(MouseReport);

				MouseReport *report = (MouseReport *)Buffer;

				while (DriverManager->GlobalMouseInputReports.Count() == 0)
					TaskManager->Yield();

				DriverManager->GlobalMouseInputReports.Read(report, nReads);
				return sizeof(MouseReport) * nReads;
			}
			default:
				return -ENOENT;
			};
		}
		default:
		{
			std::unordered_map<dev_t, Driver::DriverObject> &drivers =
				DriverManager->GetDrivers();
			const auto it = drivers.find(Node->GetMajor());
			if (it == drivers.end())
				ReturnLogError(-EINVAL, "Driver %d not found", Node->GetMajor());
			const Driver::DriverObject *drv = &it->second;

			auto dop = drv->DeviceOperations;
			auto dOps = dop->find(Node->GetMinor());
			if (dOps == dop->end())
				ReturnLogError(-EINVAL, "Device %d not found", Node->GetMinor());
			AssertReturnError(dOps->second.Ops, -ENOTSUP);
			AssertReturnError(dOps->second.Ops->Read, -ENOTSUP);
			return dOps->second.Ops->Read(Node, Buffer, Size, Offset);
		}
		}
	}

	ssize_t __fs_Write(struct Inode *Node, const void *Buffer, size_t Size, off_t Offset)
	{
		switch (Node->GetMajor())
		{
		case 0:
		{
			switch (Node->GetMinor())
			{
			case 2: /* /dev/null */
			{
				return Size;
			}
			case 3: /* /dev/zero */
			{
				return Size;
			}
			case 4: /* /dev/random */
			{
				return Size;
			}
			case 5: /* /dev/mem */
			{
				stub;
				return 0;
			}
			case 6: /* /dev/kcon */
			{
				char *buf = (char *)Buffer;
				KernelConsole::VirtualTerminal *vt = KernelConsole::CurrentTerminal.load();
				for (size_t i = 0; i < Size; i++)
					vt->Process(buf[i]);
				return Size;
			}
			default:
				return -ENOENT;
			};
		}
		case 1:
		{
			switch (Node->GetMinor())
			{
			case 0: /* /dev/input/keyboard */
			{
				return -ENOTSUP;
			}
			case 1: /* /dev/input/mouse */
			{
				return -ENOTSUP;
			}
			default:
				return -ENOENT;
			};
		}
		default:
		{
			std::unordered_map<dev_t, Driver::DriverObject> &drivers =
				DriverManager->GetDrivers();
			const auto it = drivers.find(Node->GetMajor());
			if (it == drivers.end())
				ReturnLogError(-EINVAL, "Driver %d not found", Node->GetMajor());
			const Driver::DriverObject *drv = &it->second;

			auto dop = drv->DeviceOperations;
			auto dOps = dop->find(Node->GetMinor());
			if (dOps == dop->end())
				ReturnLogError(-EINVAL, "Device %d not found", Node->GetMinor());
			AssertReturnError(dOps->second.Ops, -ENOTSUP);
			AssertReturnError(dOps->second.Ops->Write, -ENOTSUP);
			return dOps->second.Ops->Write(Node, Buffer, Size, Offset);
		}
		}
	}

	__no_sanitize("alignment")
		ssize_t __fs_Readdir(struct Inode *_Node, struct kdirent *Buffer, size_t Size, off_t Offset, off_t Entries)
	{
		auto Node = (Manager::DeviceInode *)_Node;

		off_t realOffset = Offset;

		size_t totalSize = 0;
		uint16_t reclen = 0;
		struct kdirent *ent = nullptr;

		if (Offset == 0)
		{
			reclen = (uint16_t)(offsetof(struct kdirent, d_name) + strlen(".") + 1);
			if (totalSize + reclen >= Size)
				return -EINVAL;

			ent = (struct kdirent *)((uintptr_t)Buffer + totalSize);
			ent->d_ino = Node->Node.Index;
			ent->d_off = Offset++;
			ent->d_reclen = reclen;
			ent->d_type = DT_DIR;
			strcpy(ent->d_name, ".");
			totalSize += reclen;
		}

		if (Offset <= 1)
		{
			reclen = (uint16_t)(offsetof(struct kdirent, d_name) + strlen("..") + 1);
			if (totalSize + reclen >= Size)
			{
				if (realOffset == 1)
					return -EINVAL;
				return totalSize;
			}

			ent = (struct kdirent *)((uintptr_t)Buffer + totalSize);

			if (Node->Parent)
				ent->d_ino = Node->Parent->Node->Index;
			else if (Node->ParentInode)
				ent->d_ino = Node->ParentInode->Index;
			else
			{
				warn("Parent is null for %s", Node->Name.c_str());
				ent->d_ino = Node->Node.Index;
			}
			ent->d_off = Offset++;
			ent->d_reclen = reclen;
			ent->d_type = DT_DIR;
			strcpy(ent->d_name, "..");
			totalSize += reclen;
		}

		if (!S_ISDIR(Node->Node.Mode))
			return -ENOTDIR;

		if ((Offset >= 2 ? (Offset - 2) : Offset) > (off_t)Node->Children.size())
			return -EINVAL;

		off_t entries = 0;
		for (const auto &var : Node->Children)
		{
			if (var->Node.Offset < Offset)
				continue;

			if (entries >= Entries)
				break;

			reclen = (uint16_t)(offsetof(struct kdirent, d_name) + strlen(var->Name.c_str()) + 1);

			if (totalSize + reclen >= Size)
				break;

			ent = (struct kdirent *)((uintptr_t)Buffer + totalSize);
			ent->d_ino = var->Node.Index;
			ent->d_off = var->Node.Offset;
			ent->d_reclen = reclen;
			ent->d_type = IFTODT(var->Node.Mode);
			strncpy(ent->d_name, var->Name.c_str(), strlen(var->Name.c_str()));

			totalSize += reclen;
			entries++;
		}

		if (totalSize + sizeof(struct kdirent) >= Size)
			return totalSize;

		ent = (struct kdirent *)((uintptr_t)Buffer + totalSize);
		ent->d_ino = 0;
		ent->d_off = 0;
		ent->d_reclen = 0;
		ent->d_type = DT_UNKNOWN;
		ent->d_name[0] = '\0';
		return totalSize;
	}

	void ManagerDaemonWrapper() { DriverManager->Daemon(); }

	void Manager::Daemon()
	{
		while (true)
		{
			TaskManager->Sleep(1000);
		}
	}

	dev_t Manager::RegisterInputDevice(std::unordered_map<dev_t, DriverHandlers> *dop,
									   dev_t DriverID, size_t i, const InodeOperations *Operations)
	{
		std::string prefix = "event";
		for (size_t j = 0; j < 128; j++)
		{
			std::string deviceName = prefix + std::to_string(j);
			FileNode *node = fs->GetByPath(deviceName.c_str(), devInputNode);
			if (node)
				continue;

			/* c rwx r-- r-- */
			mode_t mode = S_IRWXU |
						  S_IRGRP |
						  S_IROTH |
						  S_IFCHR;

			node = fs->ForceCreate(devInputNode, deviceName.c_str(), mode);
			node->Node->SetDevice(DriverID, i);

			DriverHandlers dh{};
			dh.Ops = Operations;
			dh.Node = node->Node;
			dh.InputReports = new RingBuffer<InputReport>(16);
			dop->insert({i, std::move(dh)});
			return i;
		}

		ReturnLogError(-1, "No available slots for device %d", DriverID);
		return -1; /* -Werror=return-type */
	}

	dev_t Manager::RegisterBlockDevice(std::unordered_map<dev_t, DriverHandlers> *dop,
									   dev_t DriverID, size_t i, const InodeOperations *Operations)
	{
		std::string prefix = "event";
		for (size_t j = 0; j < 128; j++)
		{
			std::string deviceName = prefix + std::to_string(j);
			FileNode *node = fs->GetByPath(deviceName.c_str(), devInputNode);
			if (node)
				continue;

			/* c rwx r-- r-- */
			mode_t mode = S_IRWXU |
						  S_IRGRP |
						  S_IROTH |
						  S_IFCHR;

			node = fs->ForceCreate(devInputNode, deviceName.c_str(), mode);
			node->Node->SetDevice(DriverID, i);

			DriverHandlers dh{};
			dh.Ops = Operations;
			dh.Node = node->Node;
			dh.InputReports = new RingBuffer<InputReport>(16);
			dop->insert({i, std::move(dh)});
			return i;
		}

		ReturnLogError(-1, "No available slots for device %d", DriverID);
		return -1; /* -Werror=return-type */
	}

	dev_t Manager::RegisterDevice(dev_t DriverID, DeviceType Type, const InodeOperations *Operations)
	{
		std::unordered_map<dev_t, Driver::DriverObject> &drivers =
			DriverManager->GetDrivers();
		const auto it = drivers.find(DriverID);
		if (it == drivers.end())
			ReturnLogError(-EINVAL, "Driver %d not found", DriverID);
		const Driver::DriverObject *drv = &it->second;

		auto dop = drv->DeviceOperations;
		for (size_t i = 0; i < 128; i++)
		{
			const auto dOps = dop->find(i);
			const auto dOpsEnd = dop->end();
			if (dOps != dOpsEnd)
				continue;

			DeviceType devType = (DeviceType)(Type & DEVICE_TYPE_MASK);
			switch (devType)
			{
			case DEVICE_TYPE_INPUT:
				return RegisterInputDevice(dop, DriverID, i, Operations);
			case DEVICE_TYPE_BLOCK:
				return RegisterBlockDevice(dop, DriverID, i, Operations);
			default:
				ReturnLogError(-1, "Invalid device type %d", Type);
			}
		}

		ReturnLogError(-1, "No available slots for device %d", DriverID);
	}

	int Manager::UnregisterDevice(dev_t DriverID, dev_t Device)
	{
		std::unordered_map<dev_t, Driver::DriverObject> &drivers =
			DriverManager->GetDrivers();
		const auto it = drivers.find(DriverID);
		if (it == drivers.end())
			ReturnLogError(-EINVAL, "Driver %d not found", DriverID);
		const Driver::DriverObject *drv = &it->second;

		auto dop = drv->DeviceOperations;
		const auto dOps = dop->find(Device);
		if (dOps == dop->end())
			ReturnLogError(-EINVAL, "Device %d not found", Device);
		dop->erase(dOps);
		fixme("remove eventX from /dev/input");
		fixme("delete InputReports");
		return 0;
	}

	int Manager::ReportInputEvent(dev_t DriverID, InputReport *Report)
	{
		std::unordered_map<dev_t, Driver::DriverObject> &drivers =
			DriverManager->GetDrivers();
		const auto it = drivers.find(DriverID);
		if (it == drivers.end())
			ReturnLogError(-EINVAL, "Driver %d not found", DriverID);
		const Driver::DriverObject *drv = &it->second;

		auto dop = drv->DeviceOperations;
		auto dOps = dop->find(Report->Device);
		if (dOps == dop->end())
			ReturnLogError(-EINVAL, "Device %d not found", Report->Device);

		dOps->second.InputReports->Write(Report, 1);

		switch (Report->Type)
		{
		case INPUT_TYPE_KEYBOARD:
		{
			KeyboardReport *kReport = &Report->Keyboard;
			GlobalKeyboardInputReports.Write(kReport, 1);
			break;
		}
		case INPUT_TYPE_MOUSE:
		{
			MouseReport *mReport = &Report->Mouse;
			GlobalMouseInputReports.Write(mReport, 1);
			break;
		}
		default:
			assert(!"Invalid input type");
		}
		return 0;
	}

	void Manager::InitializeDaemonFS()
	{
		dev_t MinorID = 0;
		DeviceInode *_dev = new DeviceInode;
		_dev->Name = "dev";

		/* d rwx r-- r-- */
		mode_t mode = S_IRWXU |
					  S_IRGRP |
					  S_IROTH |
					  S_IFDIR;
		Inode *dev = (Inode *)_dev;
		dev->Mode = mode;
		dev->Flags = I_FLAG_MOUNTPOINT | I_FLAG_CACHE_KEEP;

		FileSystemInfo *fsi = new FileSystemInfo;
		fsi->Name = "Driver Manager";
		fsi->RootName = "dev";
		fsi->Flags = I_FLAG_ROOT | I_FLAG_MOUNTPOINT | I_FLAG_CACHE_KEEP;
		fsi->SuperOps = {};
		fsi->Ops.Lookup = __fs_Lookup;
		fsi->Ops.Create = __fs_Create;
		fsi->Ops.Read = __fs_Read;
		fsi->Ops.Write = __fs_Write;
		fsi->Ops.ReadDir = __fs_Readdir;

		dev->Device = fs->RegisterFileSystem(fsi, dev);
		dev->SetDevice(0, MinorID++);
		MinorID++; /* 1 = /proc/self */

		devNode = fs->Mount(fs->GetRoot(0), dev, "/dev");
		_dev->Parent = devNode->Parent;
		_dev->ParentInode = devNode->Parent->Node;

		/* d rwx r-- r-- */
		mode = S_IRWXU |
			   S_IRGRP |
			   S_IROTH |
			   S_IFDIR;
		DeviceInode *input = new DeviceInode;
		input->Parent = devNode;
		input->ParentInode = devNode->Node;
		input->Name = "input";
		input->Node.Device = dev->Device;
		input->Node.Mode = mode;
		input->Node.Flags = I_FLAG_CACHE_KEEP;
		_dev->Children.push_back(input);
		devInputNode = fs->GetByPath("input", devNode);

		auto createDevice = [](DeviceInode *p1, FileNode *p2, const std::string &name, dev_t maj, dev_t min, mode_t mode)
		{
			DeviceInode *device = new DeviceInode;
			device->Parent = p2;
			device->ParentInode = p2->Node;
			device->Name = name;
			device->Node.Device = p2->Node->Device;
			device->Node.Mode = mode;
			device->Node.SetDevice(maj, min);
			device->Node.Flags = I_FLAG_CACHE_KEEP;
			p1->Children.push_back(device);
		};

		/* c rw- rw- rw- */
		mode = S_IRUSR | S_IWUSR |
			   S_IRGRP | S_IWGRP |
			   S_IROTH | S_IWOTH |
			   S_IFCHR;
		createDevice(_dev, devNode, "null", 0, MinorID++, mode);

		/* c rw- rw- rw- */
		mode = S_IRUSR | S_IWUSR |
			   S_IRGRP | S_IWGRP |
			   S_IROTH | S_IWOTH |
			   S_IFCHR;
		createDevice(_dev, devNode, "zero", 0, MinorID++, mode);

		/* c rw- rw- rw- */
		mode = S_IRUSR | S_IWUSR |
			   S_IRGRP | S_IWGRP |
			   S_IROTH | S_IWOTH |
			   S_IFCHR;
		createDevice(_dev, devNode, "random", 0, MinorID++, mode);

		/* c rw- r-- --- */
		mode = S_IRUSR | S_IWUSR |
			   S_IRGRP |

			   S_IFCHR;
		createDevice(_dev, devNode, "mem", 0, MinorID++, mode);

		/* c rw- r-- --- */
		mode = S_IRUSR | S_IWUSR |
			   S_IRGRP |

			   S_IFCHR;
		createDevice(_dev, devNode, "kcon", 0, MinorID++, mode);

		/* ------------------------------------------------------ */

		MinorID = 0;

		/* c rw- r-- --- */
		mode = S_IRUSR | S_IWUSR |
			   S_IRGRP |

			   S_IFCHR;
		createDevice(input, devInputNode, "keyboard", 1, MinorID++, mode);

		/* c rw- r-- --- */
		mode = S_IRUSR | S_IWUSR |
			   S_IRGRP |

			   S_IFCHR;
		createDevice(input, devInputNode, "mouse", 1, MinorID++, mode);
	}
}
