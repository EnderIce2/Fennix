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

#include "../../kernel.h"

namespace Driver
{
	dev_t DeviceDirID;

	int __fs_Lookup(struct Inode *_Parent, const char *Name, struct Inode **Result)
	{
		func("%#lx %s %#lx", _Parent, Name, Result);

		assert(_Parent != nullptr);

		const char *basename;
		size_t length;
		cwk_path_get_basename(Name, &basename, &length);
		if (basename == NULL)
		{
			error("Invalid name %s", Name);
			return -EINVAL;
		}

		auto Parent = (Manager::DeviceInode *)_Parent;
		for (const auto &child : Parent->Children)
		{
			debug("Comparing %s with %s", basename, child->Name.c_str());
			if (strcmp(child->Name.c_str(), basename) != 0)
				continue;

			*Result = &child->inode;
			return 0;
		}

		debug("Not found %s", Name);
		return -ENOENT;
	}

	int __fs_Create(struct Inode *_Parent, const char *Name, mode_t Mode, struct Inode **Result)
	{
		func("%#lx %s %d", _Parent, Name, Mode);

		assert(_Parent != nullptr);

		/* We expect to be /dev or children of it */
		auto Parent = (Manager::DeviceInode *)_Parent;
		auto _dev = new Manager::DeviceInode;
		_dev->Parent = nullptr;
		_dev->ParentInode = _Parent;
		_dev->Name = Name;
		_dev->inode.Device = Parent->inode.Device;
		_dev->inode.Mode = Mode;
		_dev->inode.Index = Parent->inode.Index + Parent->Children.size();
		_dev->inode.Offset = Parent->Children.size();
		Parent->Children.push_back(_dev);

		*Result = &_dev->inode;
		return 0;
	}

	ssize_t __fs_Read(struct Inode *Node, void *Buffer, size_t Size, off_t Offset)
	{
		func("%#lx %d %d", Node, Size, Offset);

		if ((dev_t)Node->GetMajor() == DeviceDirID)
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

	ssize_t __fs_Write(struct Inode *Node, const void *Buffer, size_t Size, off_t Offset)
	{
		func("%#lx %p %d %d", Node, Buffer, Size, Offset);

		if ((dev_t)Node->GetMajor() == DeviceDirID)
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

	int __fs_Open(struct Inode *Node, int Flags, mode_t Mode)
	{
		func("%#lx %d %d", Node, Flags, Mode);

		if ((dev_t)Node->GetMajor() == DeviceDirID)
		{
			switch (Node->GetMinor())
			{
			case 0: /* /dev/input/keyboard */
			case 1: /* /dev/input/mouse */
				return -ENOTSUP;
			default:
				return -ENOENT;
			};
		}

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
		AssertReturnError(dOps->second.Ops->Open, -ENOTSUP);
		return dOps->second.Ops->Open(Node, Flags, Mode);
	}

	int __fs_Close(struct Inode *Node)
	{
		func("%#lx", Node);

		if ((dev_t)Node->GetMajor() == DeviceDirID)
		{
			switch (Node->GetMinor())
			{
			case 0: /* /dev/input/keyboard */
			case 1: /* /dev/input/mouse */
				return -ENOTSUP;
			default:
				return -ENOENT;
			};
		}

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
		AssertReturnError(dOps->second.Ops->Close, -ENOTSUP);
		return dOps->second.Ops->Close(Node);
	}

	int __fs_Ioctl(struct Inode *Node, unsigned long Request, void *Argp)
	{
		func("%#lx %lu %#lx", Node, Request, Argp);

		if ((dev_t)Node->GetMajor() == DeviceDirID)
		{
			switch (Node->GetMinor())
			{
			case 0: /* /dev/input/keyboard */
			case 1: /* /dev/input/mouse */
				return -ENOSYS;
			default:
				return -ENOENT;
			};
		}

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
		AssertReturnError(dOps->second.Ops->Ioctl, -ENOTSUP);
		return dOps->second.Ops->Ioctl(Node, Request, Argp);
	}

	__no_sanitize("alignment") ssize_t __fs_Readdir(struct Inode *_Node, struct kdirent *Buffer, size_t Size, off_t Offset, off_t Entries)
	{
		func("%#lx %#lx %d %d %d", _Node, Buffer, Size, Offset, Entries);

		auto node = (Manager::DeviceInode *)_Node;
		off_t realOffset = Offset;
		size_t totalSize = 0;
		uint16_t reclen = 0;
		struct kdirent *ent = nullptr;

		if (!S_ISDIR(node->inode.Mode))
			return -ENOTDIR;

		if (Offset == 0)
		{
			reclen = (uint16_t)(offsetof(struct kdirent, d_name) + strlen(".") + 1);
			if (totalSize + reclen > Size)
				return -EINVAL;
			ent = (struct kdirent *)((uintptr_t)Buffer + totalSize);
			ent->d_ino = node->inode.Index;
			ent->d_off = 0;
			ent->d_reclen = reclen;
			ent->d_type = DT_DIR;
			strcpy(ent->d_name, ".");
			totalSize += reclen;
			Offset++;
		}

		if (Offset == 1)
		{
			reclen = (uint16_t)(offsetof(struct kdirent, d_name) + strlen("..") + 1);
			if (totalSize + reclen > Size)
				return totalSize;
			ent = (struct kdirent *)((uintptr_t)Buffer + totalSize);
			if (node->Parent)
				ent->d_ino = node->Parent->inode->Index;
			else if (node->ParentInode)
				ent->d_ino = node->ParentInode->Index;
			else
				ent->d_ino = node->inode.Index;
			ent->d_off = 1;
			ent->d_reclen = reclen;
			ent->d_type = DT_DIR;
			strcpy(ent->d_name, "..");
			totalSize += reclen;
			Offset++;
		}

		off_t entryIndex = 0;
		for (const auto &var : node->Children)
		{
			if (entryIndex + 2 < realOffset)
			{
				entryIndex++;
				continue;
			}
			if (Entries && entryIndex >= Entries)
				break;

			reclen = (uint16_t)(offsetof(struct kdirent, d_name) + strlen(var->Name.c_str()) + 1);
			if (totalSize + reclen > Size)
				break;
			ent = (struct kdirent *)((uintptr_t)Buffer + totalSize);
			ent->d_ino = var->inode.Index;
			ent->d_off = entryIndex + 2;
			ent->d_reclen = reclen;
			ent->d_type = IFTODT(var->inode.Mode);
			strcpy(ent->d_name, var->Name.c_str());
			totalSize += reclen;
			entryIndex++;
		}

		if (totalSize + offsetof(struct kdirent, d_name) + 1 > Size)
			return totalSize;

		ent = (struct kdirent *)((uintptr_t)Buffer + totalSize);
		ent->d_ino = 0;
		ent->d_off = 0;
		ent->d_reclen = 0;
		ent->d_type = DT_UNKNOWN;
		ent->d_name[0] = '\0';
		return totalSize;
	}

	int __fs_Stat(struct Inode *_Node, struct kstat *Stat)
	{
		func("%#lx %#lx", _Node, Stat);
		auto node = (Manager::DeviceInode *)_Node;

		assert(node != nullptr);
		assert(Stat != nullptr);

		Stat->Device = node->inode.Device;
		Stat->Index = node->inode.Index;
		Stat->HardLinks = 1;
		Stat->UserID = 0;
		Stat->GroupID = 0;
		Stat->RawDevice = node->inode.RawDevice;
		Stat->Size = node->Size;
		Stat->AccessTime = node->AccessTime;
		Stat->ModifyTime = node->ModifyTime;
		Stat->ChangeTime = node->ChangeTime;
		Stat->BlockSize = node->BlockSize;
		Stat->Blocks = node->Blocks;
		Stat->Attribute = 0;

		return 0;
	}

	void Manager::InitializeDeviceDirectory()
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

		FileSystemInfo *fsi = new FileSystemInfo;
		fsi->Name = "Device Virtual File System";
		fsi->SuperOps = {};
		fsi->Ops.Lookup = __fs_Lookup;
		fsi->Ops.Create = __fs_Create;
		fsi->Ops.Remove = nullptr;
		fsi->Ops.Rename = nullptr;
		fsi->Ops.Read = __fs_Read;
		fsi->Ops.Write = __fs_Write;
		fsi->Ops.Truncate = nullptr;
		fsi->Ops.Open = __fs_Open;
		fsi->Ops.Close = __fs_Close;
		fsi->Ops.Ioctl = __fs_Ioctl;
		fsi->Ops.ReadDir = __fs_Readdir;
		fsi->Ops.MkDir = nullptr;
		fsi->Ops.RmDir = nullptr;
		fsi->Ops.SymLink = nullptr;
		fsi->Ops.ReadLink = nullptr;
		fsi->Ops.Seek = nullptr;
		fsi->Ops.Stat = __fs_Stat;

		DeviceDirID = dev->Device = fs->RegisterFileSystem(fsi);
		dev->SetDevice(0, MinorID++);

		Node root = fs->GetRoot(0);
		devNode = fs->Mount(root, dev, "dev", fsi);
		assert(devNode->Parent != nullptr);

		_dev->Parent = devNode->Parent;
		_dev->ParentInode = devNode->Parent->inode;

		/* d rwx r-- r-- */
		mode = S_IRWXU |
			   S_IRGRP |
			   S_IROTH |
			   S_IFDIR;
		DeviceInode *blk = new DeviceInode;
		blk->Parent = devNode;
		blk->ParentInode = devNode->inode;
		blk->Name = "block";
		blk->inode.Device = dev->Device;
		blk->inode.Mode = mode;
		blk->inode.Offset = _dev->Children.size();
		_dev->Children.push_back(blk);
		devBlockNode = fs->Lookup(devNode, "block");

		/* d rwx r-- r-- */
		mode = S_IRWXU |
			   S_IRGRP |
			   S_IROTH |
			   S_IFDIR;
		DeviceInode *input = new DeviceInode;
		input->Parent = devNode;
		input->ParentInode = devNode->inode;
		input->Name = "input";
		input->inode.Device = dev->Device;
		input->inode.Mode = mode;
		input->inode.Offset = _dev->Children.size();
		_dev->Children.push_back(input);
		devInputNode = fs->Lookup(devNode, "input");

		auto createDevice = [](DeviceInode *p1, Node p2, const std::string &name, dev_t maj, dev_t min, mode_t mode)
		{
			DeviceInode *device = new DeviceInode;
			device->Parent = p2;
			device->ParentInode = p2->inode;
			device->Name = name;
			device->inode.Device = p2->inode->Device;
			device->inode.Mode = mode;
			device->inode.SetDevice(maj, min);
			device->inode.Offset = p1->Children.size();
			p1->Children.push_back(device);
		};

		MinorID = 0;

		/* c rw- r-- --- */
		mode = S_IRUSR | S_IWUSR |
			   S_IRGRP |

			   S_IFCHR;
		createDevice(input, devInputNode, "keyboard", DeviceDirID, MinorID++, mode);

		/* c rw- r-- --- */
		mode = S_IRUSR | S_IWUSR |
			   S_IRGRP |

			   S_IFCHR;
		createDevice(input, devInputNode, "mouse", DeviceDirID, MinorID++, mode);
	}
}
