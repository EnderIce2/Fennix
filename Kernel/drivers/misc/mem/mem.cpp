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
#include <rand.hpp>

extern Driver::Manager *DriverManager;
namespace Driver::MemoryDevices
{
	dev_t DriverID;

	struct
	{
		dev_t null;
		dev_t zero;
		dev_t random;
		dev_t urandom;
		dev_t mem;
	} ids;

	int Open(struct Inode *Node, int Flags, mode_t Mode) { return -ENOENT; }

	int Close(struct Inode *Node) { return -ENOSYS; }

	int Ioctl(struct Inode *Node, unsigned long Request, void *Argp) { return -ENOSYS; }

	ssize_t Read(struct Inode *Node, void *Buffer, size_t Size, off_t Offset)
	{
		dev_t min = Node->GetMinor();
		if (min == ids.null)
			return 0;
		else if (min == ids.zero)
		{
			if (Size <= 0)
				return 0;

			memset(Buffer, 0, Size);
			return Size;
		}
		else if (min == ids.random || min == ids.urandom)
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
		else if (min == ids.mem)
		{
			stub;
			return 0;
		}

		return -ENODEV;
	}

	ssize_t Write(struct Inode *Node, const void *Buffer, size_t Size, off_t Offset)
	{
		dev_t min = Node->GetMinor();
		if (min == ids.null)
			return Size;
		else if (min == ids.zero)
			return Size;
		else if (min == ids.random || min == ids.urandom)
			return Size;
		else if (min == ids.mem)
			return Size;

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
		ids.null = DriverManager->CreateDeviceFile(DriverID, "null", mode, &ops);
		ids.zero = DriverManager->CreateDeviceFile(DriverID, "zero", mode, &ops);
		ids.random = DriverManager->CreateDeviceFile(DriverID, "random", mode, &ops);
		ids.urandom = DriverManager->CreateDeviceFile(DriverID, "urandom", mode, &ops);

		/* c rw- r-- --- */
		mode = S_IRUSR | S_IWUSR |
			   S_IRGRP |

			   S_IFCHR;
		ids.mem = DriverManager->CreateDeviceFile(DriverID, "mem", mode, &ops);
		return 0;
	}

	int Final()
	{
		DriverManager->UnregisterDevice(DriverID, ids.null);
		DriverManager->UnregisterDevice(DriverID, ids.zero);
		DriverManager->UnregisterDevice(DriverID, ids.random);
		DriverManager->UnregisterDevice(DriverID, ids.urandom);
		DriverManager->UnregisterDevice(DriverID, ids.mem);
		return 0;
	}

	int Panic() { return 0; }
	int Probe() { return 0; }

	REGISTER_BUILTIN_DRIVER(mem,
							"Memory Devices Driver",
							"enderice2",
							1, 0, 0,
							Entry,
							Final,
							Panic,
							Probe);
}
