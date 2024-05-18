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

#include <filesystem.hpp>

#include <convert.h>
#include <printf.h>
#include <rand.hpp>
#include <cwalk.h>

#include "../kernel.h"

namespace vfs
{
	/*	maj = 0
		min:
			0 - <ROOT>
			1 - /proc/self
			2 - /dev/null
			3 - /dev/zero
			4 - /dev/random
			5 - /dev/mem
	*/

	int __vfs_Lookup(struct Inode *_Parent, const char *Name, struct Inode **Result)
	{
		vfsInode *Parent = (vfsInode *)_Parent;

		if (!S_ISDIR(Parent->Node.Mode))
			return -ENOTDIR;

		assert(Parent->Node.Flags & I_FLAG_MOUNTPOINT);

		if (Parent->Children.empty())
			return -ENOENT;

		off_t offset = 0;
		foreach (const auto &Root in Parent->Children)
		{
			char rootName[128]{};
			snprintf(rootName, sizeof(rootName), "root-%ld", offset);

			if (strcmp(rootName, Name) == 0)
			{
				*Result = Root;
				return 0;
			}
			offset++;
		}

		return -ENOENT;
	}

	int __vfs_Create(struct Inode *Parent, const char *Name, mode_t Mode, struct Inode **Result)
	{
		assert(Parent != nullptr);
		assert(!"Not implemented");
	}

	ssize_t __vfs_Read(struct Inode *Node, void *Buffer, size_t Size, off_t Offset)
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
		default:
			return -ENOENT;
		};
	}

	ssize_t __vfs_Write(struct Inode *Node, const void *Buffer, size_t Size, off_t Offset)
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
		default:
			return -ENOENT;
		};
	}

	/* This implementation is used internally by the kernel, so no "." & ".." */
	ssize_t __vfs_Readdir(struct Inode *_Node, struct kdirent *Buffer, size_t Size, off_t Offset, off_t Entries)
	{
		if (_Node->GetMinor() != 0)
		{
			debug("_Node->GetMinor() != 0");
			return -ENOENT;
		}

		assert(_Node->Flags & I_FLAG_MOUNTPOINT);

		fixme("maybe wrong implementation of readdir");

		size_t totalSize = 0;
		off_t entriesSkipped = 0;
		struct kdirent *ent = nullptr;
		vfsInode *Node = (vfsInode *)_Node;
		off_t entries = 0;
		foreach (const auto &Root in Node->Children)
		{
			if (entries >= Entries)
				break;

			uint16_t reclen = (uint16_t)(offsetof(struct kdirent, d_name) + strlen("root") + 1);

			if (Offset > entriesSkipped)
			{
				entriesSkipped++;
				continue;
			}

			if (totalSize + reclen >= Size)
				break;

			ent = (struct kdirent *)((uintptr_t)Buffer + totalSize);
			ent->d_ino = Root->Index;
			ent->d_off = Root->Offset;
			ent->d_reclen = reclen;
			ent->d_type = IFTODT(Root->Mode);
			strncpy(ent->d_name, "root", strlen("root"));

			totalSize += reclen;
			entries++;
		}

		if (ent)
			ent->d_off = INT32_MAX;

		return totalSize;
	}

	ssize_t __vfs_ReadLink(struct Inode *Node, char *Buffer, size_t Size)
	{
		switch (Node->GetMinor())
		{
		case 1:
		{
			/* FIXME: https://github.com/torvalds/linux/blob/c942a0cd3603e34dd2d7237e064d9318cb7f9654/fs/proc/self.c#L11
					https://lxr.linux.no/#linux+v3.2.9/fs/proc/base.c#L2482 */

			int ret = snprintf(Buffer, Size, "/proc/%d", thisProcess->ID);
			debug("ReadLink: %s (%d bytes)", Buffer, ret);
			return ret;
		}
		default:
			return -ENOENT;
		}
	}

	void Virtual::Initialize()
	{
		SmartLock(VirtualLock);

		trace("Initializing virtual file system...");
		uint32_t iFlags = I_FLAG_CACHE_KEEP;

		/* d rwx rwx rwx */
		mode_t mode = S_IRWXU |
					  S_IRWXG |
					  S_IRWXO |
					  S_IFDIR;
		FileNode *dev = this->ForceCreate(this->GetRoot(0), "dev", mode);
		FileNode *mnt = this->ForceCreate(this->GetRoot(0), "mnt", mode);
		FileNode *proc = this->ForceCreate(this->GetRoot(0), "proc", mode);
		FileNode *log = this->ForceCreate(this->GetRoot(0), "var", mode);
		log = this->ForceCreate(log, "log", mode);
		dev->Node->Flags = iFlags;
		mnt->Node->Flags = iFlags;
		proc->Node->Flags = iFlags;
		log->Node->Flags = iFlags;

		/* l rwx rwx rwx */
		mode = S_IRWXU |
			   S_IRWXG |
			   S_IRWXO |
			   S_IFLNK;
		FileNode *self = this->ForceCreate(proc, "self", mode);
		self->Node->Device = FileSystemRoots->Node.Device;
		self->Node->SetDevice(0, 1);
		self->Node->Flags = iFlags;

		/* c rw- rw- rw- */
		mode = S_IRUSR | S_IWUSR |
			   S_IRGRP | S_IWGRP |
			   S_IROTH | S_IWOTH |
			   S_IFCHR;
		FileNode *null = this->ForceCreate(dev, "null", mode);
		null->Node->Device = FileSystemRoots->Node.Device;
		null->Node->SetDevice(0, 2);
		null->Node->Flags = iFlags;

		/* c rw- rw- rw- */
		mode = S_IRUSR | S_IWUSR |
			   S_IRGRP | S_IWGRP |
			   S_IROTH | S_IWOTH |
			   S_IFCHR;
		FileNode *zero = this->ForceCreate(dev, "zero", mode);
		zero->Node->Device = FileSystemRoots->Node.Device;
		zero->Node->SetDevice(0, 3);
		zero->Node->Flags = iFlags;

		/* c rw- rw- rw- */
		mode = S_IRUSR | S_IWUSR |
			   S_IRGRP | S_IWGRP |
			   S_IROTH | S_IWOTH |
			   S_IFCHR;
		FileNode *random = this->ForceCreate(dev, "random", mode);
		random->Node->Device = FileSystemRoots->Node.Device;
		random->Node->SetDevice(0, 4);
		random->Node->Flags = iFlags;

		/* c rw- r-- --- */
		mode = S_IRUSR | S_IWUSR |
			   S_IRGRP |

			   S_IFCHR;
		FileNode *mem = this->ForceCreate(dev, "mem", mode);
		mem->Node->Device = FileSystemRoots->Node.Device;
		mem->Node->SetDevice(0, 5);
		mem->Node->Flags = iFlags;

		new vfs::PTMXDevice();
	}

	Virtual::Virtual()
	{
		SmartLock(VirtualLock);

		FileSystemRoots = new vfsInode;
		FileSystemRoots->Node.Index = -1;

		FileSystemRoots->Node.Mode = S_IRWXU |
									 S_IRWXG |
									 S_IROTH | S_IXOTH |
									 S_IFDIR;

		FileSystemRoots->Node.Flags = I_FLAG_MOUNTPOINT | I_FLAG_CACHE_KEEP;

		FileSystemRoots->Node.Offset = INT32_MAX;
		FileSystemRoots->Name = "<ROOT>";

		FileSystemInfo *fsi = new FileSystemInfo;
		fsi->Name = "Virtual Roots";
		fsi->Flags = I_FLAG_MOUNTPOINT | I_FLAG_CACHE_KEEP;
		fsi->SuperOps = {};
		fsi->Ops.Lookup = __vfs_Lookup;
		fsi->Ops.Create = __vfs_Create;
		fsi->Ops.Read = __vfs_Read;
		fsi->Ops.Write = __vfs_Write;
		fsi->Ops.ReadDir = __vfs_Readdir;
		fsi->Ops.ReadLink = __vfs_ReadLink;

		FileSystemRoots->Node.Device = this->RegisterFileSystem(fsi, &FileSystemRoots->Node);
		FileSystemRoots->Node.SetDevice(0, 0);
	}

	Virtual::~Virtual()
	{
	}
}
