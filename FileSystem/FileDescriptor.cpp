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

#include <smart_ptr.hpp>
#include <convert.h>
#include <task.hpp>
#include <printf.h>
#include <lock.hpp>
#include <cwalk.h>

#include "../kernel.h"

namespace VirtualFileSystem
{
	ReadFSFunction(fd_Read)
	{
		if (!Size)
			Size = node->Length;

		if (RefOffset > node->Length)
			return 0;

		if (RefOffset + (off_t)Size > node->Length)
			Size = node->Length - RefOffset;

		memcpy(Buffer, (uint8_t *)(node->Address + RefOffset), Size);
		return Size;
	}

	WriteFSFunction(fd_Write)
	{
		if (!Size)
			Size = node->Length;

		if (RefOffset > node->Length)
			return 0;

		if (RefOffset + (off_t)Size > node->Length)
			Size = node->Length - RefOffset;

		memcpy((uint8_t *)(node->Address + RefOffset), Buffer, Size);
		return Size;
	}

	VirtualFileSystem::FileSystemOperations fd_op = {
		.Name = "fd",
		// .Read = fd_Read,
		// .Write = fd_Write,
	};

	FileDescriptorTable::FileDescriptor
	FileDescriptorTable::GetFileDescriptor(int FileDescriptor)
	{
		foreach (auto fd in FileDescriptors)
		{
			if (fd.Descriptor == FileDescriptor)
				return fd;
		}
		return {.Descriptor = -1};
	}

	int FileDescriptorTable::ProbeMode(mode_t Mode, int Flags)
	{
		if (Flags & O_RDONLY)
		{
			if (!(Mode & S_IRUSR))
				return -EACCES;
		}

		if (Flags & O_WRONLY)
		{
			if (!(Mode & S_IWUSR))
				return -EACCES;
		}

		if (Flags & O_RDWR)
		{
			if (!(Mode & S_IRUSR) ||
				!(Mode & S_IWUSR))
				return -EACCES;
		}

		return 0;
	}

	int FileDescriptorTable::AddFileDescriptor(const char *AbsolutePath,
											   mode_t Mode, int Flags)
	{
		Tasking::PCB *pcb = thisProcess;

		if (ProbeMode(Mode, Flags) < 0)
		{
			errno = EACCES;
			return -1;
		}

		if (Flags & O_CREAT)
		{
			Node *n = vfs->Create(AbsolutePath,
								  NodeFlags::FILE,
								  pcb->CurrentWorkingDirectory);

			if (!n)
			{
				error("Failed to create file %s: %d",
					  AbsolutePath, errno);
				return -1;
			}
		}

		if (Flags & O_EXCL)
		{
			RefNode *File = vfs->Open(AbsolutePath,
									  pcb->CurrentWorkingDirectory);

			if (!File)
			{
				errno = EEXIST;
				return -1;
			}
		}

		if (Flags & O_TRUNC)
		{
			fixme("Implement O_TRUNC");
		}

		if (Flags & O_APPEND)
		{
			fixme("Implement O_APPEND");
		}

		if (Flags & O_CLOEXEC)
		{
			fixme("Implement O_CLOEXEC");
		}

		RefNode *File = vfs->Open(AbsolutePath,
								  pcb->CurrentWorkingDirectory);

		if (!File)
		{
			error("Failed to open file %s: %d",
				  AbsolutePath, errno);
			return -1;
		}

		FileDescriptorTable::FileDescriptor fd;
		fd.Descriptor = GetFreeFileDescriptor();

		if (fd.Descriptor < 0)
		{
			errno = EMFILE;
			return -1;
		}

		fd.Mode = Mode;
		fd.Flags = Flags;
		fd.Handle = File;

		FileDescriptors.push_back(fd);

		char FileName[64];
		sprintf(FileName, "%d", fd.Descriptor);
		VirtualFileSystem::Node *n = vfs->Create(FileName, VirtualFileSystem::NodeFlags::FILE, this->fdDir);
		if (n)
		{
			/* FIXME: Implement proper file descriptors */
			n->Address = (uintptr_t)0xdeadbeef;
			n->Length = File->FileSize;
			n->Operator = &fd_op;
		}

		return fd.Descriptor;
	}

	int FileDescriptorTable::RemoveFileDescriptor(int FileDescriptor)
	{
		forItr(itr, FileDescriptors)
		{
			if (itr->Descriptor == FileDescriptor)
			{
				FileDescriptors.erase(itr);

				char FileName[64];
				sprintf(FileName, "%d", FileDescriptor);
				vfs->Delete(FileName, false, this->fdDir);
				return 0;
			}
		}

		errno = EBADF;
		return -1;
	}

	int FileDescriptorTable::GetFreeFileDescriptor()
	{
		int i = 0;
		while (true)
		{
			bool Found = false;
			foreach (auto fd in FileDescriptors)
			{
				if (fd.Descriptor == i)
				{
					Found = true;
					break;
				}
			}
			if (!Found)
				return i;
			i++;
		}

		errno = EMFILE;
		return -1;
	}

	std::string FileDescriptorTable::GetAbsolutePath(int FileDescriptor)
	{
		FileDescriptorTable::FileDescriptor fd =
			this->GetFileDescriptor(FileDescriptor);
		if (fd.Descriptor == -1)
			return "";

		Node *node = fd.Handle->node;
		std::string absolutePath = vfs->GetPathFromNode(node);
		std::string path = absolutePath.c_str();
		return path;
	}

	int FileDescriptorTable::_open(const char *pathname, int flags,
								   mode_t mode)
	{
		if (pathname == nullptr)
		{
			errno = EFAULT;
			return -1;
		}

		return AddFileDescriptor(pathname, mode, flags);
	}

	int FileDescriptorTable::_creat(const char *pathname, mode_t mode)
	{
		return _open(pathname, O_WRONLY | O_CREAT | O_TRUNC, mode);
	}

	ssize_t FileDescriptorTable::_read(int fd, void *buf, size_t count)
	{
		FileDescriptor fdesc;
		fdesc = this->GetFileDescriptor(fd);

		if (fdesc.Descriptor < 0)
		{
			errno = EBADF;
			return -1;
		}

		return fdesc.Handle->Read((uint8_t *)buf, count);
	}

	ssize_t FileDescriptorTable::_write(int fd, const void *buf,
										size_t count)
	{
		FileDescriptor fdesc;
		fdesc = this->GetFileDescriptor(fd);

		if (fdesc.Descriptor < 0)
			return -1;

		return fdesc.Handle->Write((uint8_t *)buf, count);
	}

	int FileDescriptorTable::_close(int fd)
	{
		FileDescriptor fdesc;
		fdesc = this->GetFileDescriptor(fd);

		if (fdesc.Descriptor < 0)
		{
			errno = EBADF;
			return -1;
		}

		if (RemoveFileDescriptor(fd) < 0)
		{
			errno = EBADF;
			return -1;
		}

		delete fdesc.Handle;
		return 0;
	}

	off_t FileDescriptorTable::_lseek(int fd, off_t offset, int whence)
	{
		FileDescriptor fdesc;
		fdesc = this->GetFileDescriptor(fd);

		if (fdesc.Descriptor < 0)
		{
			errno = EBADF;
			return -1;
		}

		return fdesc.Handle->Seek(offset, whence);
	}

	int FileDescriptorTable::_stat(const char *pathname,
								   struct stat *statbuf)
	{
		if (pathname == nullptr)
		{
			errno = EINVAL;
			return -1;
		}

		RefNode *file = vfs->Open(pathname,
								  thisProcess->CurrentWorkingDirectory);

		if (!file)
		{
			error("Failed to open file %s: %d",
				  pathname, errno);
			return -1;
		}

		Node *node = file->node;
		statbuf->st_dev = 0; /* FIXME: stub */
		statbuf->st_ino = node->IndexNode;
		statbuf->st_mode = node->Flags | node->Mode;
		statbuf->st_nlink = 0; /* FIXME: stub */
		statbuf->st_uid = node->UserIdentifier;
		statbuf->st_gid = node->GroupIdentifier;
		statbuf->st_rdev = 0; /* FIXME: stub */
		statbuf->st_size = node->Length;
		statbuf->st_blksize = 0; /* FIXME: stub */
		statbuf->st_blocks = 0;	 /* FIXME: stub */
		statbuf->st_attr = 0; /* FIXME: stub */
		return 0;
	}

	int FileDescriptorTable::_fstat(int fd, struct stat *statbuf)
	{
		FileDescriptor fdesc;
		fdesc = this->GetFileDescriptor(fd);

		if (fdesc.Descriptor < 0)
		{
			errno = EBADF;
			return -1;
		}

		Node *node = fdesc.Handle->node;
		statbuf->st_dev = 0; /* FIXME: stub */
		statbuf->st_ino = node->IndexNode;
		statbuf->st_mode = node->Flags | node->Mode;
		statbuf->st_nlink = 0; /* FIXME: stub */
		statbuf->st_uid = node->UserIdentifier;
		statbuf->st_gid = node->GroupIdentifier;
		statbuf->st_rdev = 0; /* FIXME: stub */
		statbuf->st_size = node->Length;
		statbuf->st_blksize = 0; /* FIXME: stub */
		statbuf->st_blocks = 0;	 /* FIXME: stub */
		statbuf->st_attr = 0; /* FIXME: stub */
		return 0;
	}

	int FileDescriptorTable::_lstat(const char *pathname,
									struct stat *statbuf)
	{
		if (pathname == nullptr)
		{
			errno = EINVAL;
			return -1;
		}

		RefNode *file = vfs->Open(pathname,
								  thisProcess->CurrentWorkingDirectory);

		if (!file)
		{
			error("Failed to open file %s: %d",
				  pathname, errno);
			return -1;
		}

		Node *node = file->node;
		statbuf->st_dev = 0; /* FIXME: stub */
		statbuf->st_ino = node->IndexNode;
		statbuf->st_mode = node->Flags | node->Mode;
		statbuf->st_nlink = 0; /* FIXME: stub */
		statbuf->st_uid = node->UserIdentifier;
		statbuf->st_gid = node->GroupIdentifier;
		statbuf->st_rdev = 0; /* FIXME: stub */
		statbuf->st_size = node->Length;
		statbuf->st_blksize = 0; /* FIXME: stub */
		statbuf->st_blocks = 0;	 /* FIXME: stub */
		statbuf->st_attr = 0; /* FIXME: stub */
		return 0;
	}

	FileDescriptorTable::FileDescriptorTable(void *Owner)
	{
		this->fdDir = vfs->Create("fd", VirtualFileSystem::NodeFlags::DIRECTORY,
								  ((Tasking::PCB *)Owner)->ProcessDirectory);
	}

	FileDescriptorTable::~FileDescriptorTable()
	{
		foreach (auto var in FileDescriptors)
		{
			this->RemoveFileDescriptor(var.Descriptor);
			delete var.Handle;
		}
	}
}
