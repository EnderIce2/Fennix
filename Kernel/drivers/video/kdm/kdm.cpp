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

extern Driver::Manager *DriverManager;
namespace Driver::KernelDisplayManager
{
	dev_t DriverID;

	struct
	{
		dev_t kdm;
	} ids;

	int Open(struct Inode *Node, int Flags, mode_t Mode)
	{
		dev_t min = Node->GetMinor();
		if (min == ids.kdm)
			return -ENOSYS;

		return -ENODEV;
	}

	int Close(struct Inode *Node)
	{
		dev_t min = Node->GetMinor();
		if (min == ids.kdm)
			return -ENOSYS;

		return -ENODEV;
	}

	int Ioctl(struct Inode *Node, unsigned long Request, void *Argp)
	{
		dev_t min = Node->GetMinor();
		if (min == ids.kdm)
			return -ENOSYS;

		return -ENODEV;
	}

	ssize_t Read(struct Inode *Node, void *Buffer, size_t Size, off_t Offset)
	{
		dev_t min = Node->GetMinor();
		if (min == ids.kdm)
			return -ENOSYS;

		return -ENODEV;
	}

	ssize_t Write(struct Inode *Node, const void *Buffer, size_t Size, off_t Offset)
	{
		dev_t min = Node->GetMinor();
		if (min == ids.kdm)
			return -ENOSYS;

		return -ENODEV;
	}

	off_t Seek(struct Inode *Node, off_t Offset) { return -ENOSYS; }
	int Stat(struct Inode *Node, struct kstat *Stat) { return -ENOSYS; }

	const struct InodeOperations ops = {
		.Lookup = nullptr,
		.Create = nullptr,
		.Remove = nullptr,
		.Rename = nullptr,
		.Read = Read,
		.Write = Write,
		.Truncate = nullptr,
		.Open = Open,
		.Close = Close,
		.Ioctl = Ioctl,
		.ReadDir = nullptr,
		.MkDir = nullptr,
		.RmDir = nullptr,
		.SymLink = nullptr,
		.ReadLink = nullptr,
		.Seek = Seek,
		.Stat = Stat,
	};

	int Entry()
	{
		mode_t mode = 0;

		/* c rw- rw- rw- */
		mode = S_IRUSR | S_IWUSR |
			   S_IRGRP | S_IWGRP |
			   S_IROTH | S_IWOTH |
			   S_IFCHR;
		ids.kdm = DriverManager->CreateDeviceFile(DriverID, "kdm", mode, &ops);
		return 0;
	}

	int Final()
	{
		DriverManager->UnregisterDevice(DriverID, ids.kdm);
		return 0;
	}

	int Panic() { return 0; }
	int Probe() { return 0; }

	REGISTER_BUILTIN_DRIVER(kdm,
							"Kernel Display Manager Driver",
							"enderice2",
							1, 0, 0,
							Entry,
							Final,
							Panic,
							Probe);
}
