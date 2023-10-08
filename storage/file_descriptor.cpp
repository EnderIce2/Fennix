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
#include <stropts.h>
#include <task.hpp>
#include <printf.h>
#include <lock.hpp>
#include <cwalk.h>

#include "../kernel.h"

namespace vfs
{
	// ReadFSFunction(fd_Read)
	// {
	// 	if (Size <= 0)
	// 		Size = node->Length;

	// 	if (RefOffset > node->Length)
	// 		return 0;

	// 	if ((node->Length - RefOffset) == 0)
	// 		return 0; /* EOF */

	// 	if (RefOffset + (off_t)Size > node->Length)
	// 		Size = node->Length;

	// 	memcpy(Buffer, (uint8_t *)(node->Address + RefOffset), Size);
	// 	return Size;
	// }

	// WriteFSFunction(fd_Write)
	// {
	// 	if (Size <= 0)
	// 		Size = node->Length;

	// 	if (RefOffset > node->Length)
	// 		return 0;

	// 	if (RefOffset + (off_t)Size > node->Length)
	// 		Size = node->Length;

	// 	memcpy((uint8_t *)(node->Address + RefOffset), Buffer, Size);
	// 	return Size;
	// }

	// vfs::FileSystemOperations fd_op = {
	// 	.Name = "fd",
	// 	// .Read = fd_Read,
	// 	// .Write = fd_Write,
	// };

	FileDescriptorTable::Fildes
	FileDescriptorTable::GetFileDescriptor(int FileDescriptor)
	{
		foreach (auto fd in FileDescriptors)
		{
			if (fd.Descriptor == FileDescriptor)
			{
				// debug("Found file descriptor %d", FileDescriptor);
				return fd;
			}
		}
		return {};
	}

	FileDescriptorTable::DupFildes
	FileDescriptorTable::GetDupFildes(int FileDescriptor)
	{
		foreach (auto fd in FildesDuplicates)
		{
			if (fd.Descriptor == FileDescriptor)
			{
				debug("Found duplicated file descriptor %d", FileDescriptor);
				return fd;
			}
		}
		return {};
	}

	int FileDescriptorTable::ProbeMode(mode_t Mode, int Flags)
	{
		if (!(Flags & O_CREAT))
			return 0;

		if (Flags & O_RDONLY)
		{
			if (!(Mode & S_IRUSR))
			{
				debug("No read permission (%d)", Mode);
				return -EACCES;
			}
		}

		if (Flags & O_WRONLY)
		{
			if (!(Mode & S_IWUSR))
			{
				debug("No write permission (%d)", Mode);
				return -EACCES;
			}
		}

		if (Flags & O_RDWR)
		{
			if (!(Mode & S_IRUSR) ||
				!(Mode & S_IWUSR))
			{
				debug("No read/write permission (%d)", Mode);
				return -EACCES;
			}
		}

		return 0;
	}

	int FileDescriptorTable::AddFileDescriptor(const char *AbsolutePath,
											   mode_t Mode, int Flags)
	{
		Tasking::PCB *pcb = thisProcess;

		if (ProbeMode(Mode, Flags) < 0)
			return -EACCES;

		if (Flags & O_CREAT)
		{
			int ret;
			Node *n = new Node(pcb->CurrentWorkingDirectory,
							   AbsolutePath,
							   NodeType::FILE,
							   cwk_path_is_absolute(AbsolutePath),
							   fs,
							   &ret);

			if (ret == -EEXIST)
			{
				debug("%s: File already exists, continuing...",
					  AbsolutePath);
			}
			else if (ret < 0)
			{
				error("Failed to create file %s: %d",
					  AbsolutePath, ret);
				assert(false);
			}
		}

		if (Flags & O_EXCL)
		{
			RefNode *File = fs->Open(AbsolutePath,
									 pcb->CurrentWorkingDirectory);

			if (!File)
			{
				error("Failed to open file %s",
					  AbsolutePath);
				return -EIO;
			}
		}

		if (Flags & O_TRUNC)
		{
			fixme("O_TRUNC");
		}

		if (Flags & O_APPEND)
		{
			fixme("O_APPEND");
		}

		if (Flags & O_CLOEXEC)
		{
			fixme("O_CLOEXEC");
		}

		RefNode *File = fs->Open(AbsolutePath,
								 pcb->CurrentWorkingDirectory);

		if (!File)
		{
			error("Failed to open file %s",
				  AbsolutePath);
			return -EIO;
		}

		Fildes fd = {.Descriptor = GetFreeFileDescriptor()};

		if (fd.Descriptor < 0)
			return -EMFILE;

		fd.Mode = Mode;
		fd.Flags = Flags;
		fd.Handle = File;

		FileDescriptors.push_back(fd);

		char FileName[64];
		sprintf(FileName, "%d", fd.Descriptor);
		vfs::Node *n = fs->Create(FileName, vfs::NodeType::FILE, this->fdDir);
		if (n)
		{
			/* FIXME: Implement proper file descriptors */
			n->Size = File->FileSize;
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
				fs->Delete(FileName, false, this->fdDir);
				return 0;
			}
		}

		forItr(itr, FildesDuplicates)
		{
			if (itr->Descriptor == FileDescriptor)
			{
				FildesDuplicates.erase(itr);

				char FileName[64];
				sprintf(FileName, "%d", FileDescriptor);
				fs->Delete(FileName, false, this->fdDir);
				return 0;
			}
		}

		return -EBADF;
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
			{
				foreach (auto fd in FildesDuplicates)
				{
					if (fd.Descriptor == i)
					{
						Found = true;
						break;
					}
				}
			}

			if (!Found)
				return i;
			i++;
		}

		return -EMFILE;
	}

	const char *FileDescriptorTable::GetAbsolutePath(int FileDescriptor)
	{
		Fildes fd = this->GetFileDescriptor(FileDescriptor);
		DupFildes dfd = this->GetDupFildes(FileDescriptor);

		if (fd.Descriptor == -1 &&
			dfd.Descriptor == -1)
			return "";

		RefNode *hnd = nullptr;
		if (fd.Descriptor != -1)
			hnd = fd.Handle;
		else
			hnd = dfd.Handle;

		Node *node = hnd->node;
		const char *path = new char[strlen(node->FullPath) + 1];
		strcpy((char *)path, node->FullPath);
		return path;
	}

	int FileDescriptorTable::_open(const char *pathname, int flags,
								   mode_t mode)
	{
		if (pathname == nullptr)
			return -EFAULT;

		return AddFileDescriptor(pathname, mode, flags);
	}

	int FileDescriptorTable::_creat(const char *pathname, mode_t mode)
	{
		return _open(pathname, O_WRONLY | O_CREAT | O_TRUNC, mode);
	}

	ssize_t FileDescriptorTable::_read(int fd, void *buf, size_t count)
	{
		Fildes fdesc = this->GetFileDescriptor(fd);
		DupFildes dfdesc = this->GetDupFildes(fd);
		if (fdesc.Descriptor < 0 &&
			dfdesc.Descriptor < 0)
		{
			return -EBADF;
		}

		RefNode *hnd = nullptr;
		if (fdesc.Descriptor != -1)
			hnd = fdesc.Handle;
		else
			hnd = dfdesc.Handle;

		return hnd->read((uint8_t *)buf, count);
	}

	ssize_t FileDescriptorTable::_write(int fd, const void *buf,
										size_t count)
	{
		Fildes fdesc = this->GetFileDescriptor(fd);
		DupFildes dfdesc = this->GetDupFildes(fd);
		if (fdesc.Descriptor < 0 &&
			dfdesc.Descriptor < 0)
		{
			return -EBADF;
		}

		RefNode *hnd = nullptr;
		if (fdesc.Descriptor != -1)
			hnd = fdesc.Handle;
		else
			hnd = dfdesc.Handle;

		return hnd->write((uint8_t *)buf, count);
	}

	int FileDescriptorTable::_close(int fd)
	{
		Fildes fdesc = this->GetFileDescriptor(fd);
		DupFildes dfdesc = this->GetDupFildes(fd);

		if (fdesc.Descriptor < 0 &&
			dfdesc.Descriptor < 0)
		{
			return -EBADF;
		}

		if (RemoveFileDescriptor(fd) < 0)
			return -EBADF;

		/* If the file descriptor is a duplicate,
			we don't need to close the handle,
			because it's a duplicate of another
			file descriptor. */

		bool Found = false;
		RefNode *hnd = nullptr;

		if (fdesc.Descriptor != -1)
			hnd = fdesc.Handle;
		else
			hnd = dfdesc.Handle;

		foreach (auto dfd in FileDescriptors)
		{
			if (dfd.Handle == hnd)
			{
				Found = true;
				break;
			}
		}

		foreach (auto dfd in FildesDuplicates)
		{
			if (dfd.Handle == hnd)
			{
				Found = true;
				break;
			}
		}

		if (!Found)
			delete hnd;
		return 0;
	}

	off_t FileDescriptorTable::_lseek(int fd, off_t offset, int whence)
	{
		Fildes fdesc = this->GetFileDescriptor(fd);
		DupFildes dfdesc = this->GetDupFildes(fd);

		if (fdesc.Descriptor < 0 &&
			dfdesc.Descriptor < 0)
		{
			return -EBADF;
		}

		RefNode *hnd = nullptr;
		if (fdesc.Descriptor != -1)
			hnd = fdesc.Handle;
		else
			hnd = dfdesc.Handle;

		return hnd->seek(offset, whence);
	}

	int FileDescriptorTable::_stat(const char *pathname,
								   struct stat *statbuf)
	{
		if (pathname == nullptr)
			return -EINVAL;

		RefNode *file = fs->Open(pathname,
								 thisProcess->CurrentWorkingDirectory);

		if (!file)
		{
			error("Failed to open file %s",
				  pathname);
			return -EIO;
		}

		Node *node = file->node;
		statbuf->st_dev = 0; /* FIXME: stub */
		statbuf->st_ino = node->IndexNode;
		statbuf->st_mode = node->Type | node->Mode;
		statbuf->st_nlink = 0; /* FIXME: stub */
		statbuf->st_uid = node->UserIdentifier;
		statbuf->st_gid = node->GroupIdentifier;
		statbuf->st_rdev = 0; /* FIXME: stub */
		statbuf->st_size = node->Size;
		statbuf->st_blksize = 0; /* FIXME: stub */
		statbuf->st_blocks = 0;	 /* FIXME: stub */
		statbuf->st_attr = 0;	 /* FIXME: stub */
		return 0;
	}

	int FileDescriptorTable::_fstat(int fd, struct stat *statbuf)
	{
		Fildes fdesc = this->GetFileDescriptor(fd);
		DupFildes dfdesc = this->GetDupFildes(fd);

		if (fdesc.Descriptor < 0 &&
			dfdesc.Descriptor < 0)
		{
			return -EBADF;
		}

		RefNode *hnd = nullptr;
		if (fdesc.Descriptor != -1)
			hnd = fdesc.Handle;
		else
			hnd = dfdesc.Handle;

		Node *node = hnd->node;
		statbuf->st_dev = 0; /* FIXME: stub */
		statbuf->st_ino = node->IndexNode;
		statbuf->st_mode = node->Type | node->Mode;
		statbuf->st_nlink = 0; /* FIXME: stub */
		statbuf->st_uid = node->UserIdentifier;
		statbuf->st_gid = node->GroupIdentifier;
		statbuf->st_rdev = 0; /* FIXME: stub */
		statbuf->st_size = node->Size;
		statbuf->st_blksize = 0; /* FIXME: stub */
		statbuf->st_blocks = 0;	 /* FIXME: stub */
		statbuf->st_attr = 0;	 /* FIXME: stub */
		return 0;
	}

	int FileDescriptorTable::_lstat(const char *pathname,
									struct stat *statbuf)
	{
		if (pathname == nullptr)
			return -EINVAL;

		RefNode *file = fs->Open(pathname,
								 thisProcess->CurrentWorkingDirectory);

		if (!file)
		{
			error("Failed to open file %s",
				  pathname);
			return -EIO;
		}

		Node *node = file->node;
		statbuf->st_dev = 0; /* FIXME: stub */
		statbuf->st_ino = node->IndexNode;
		statbuf->st_mode = node->Type | node->Mode;
		statbuf->st_nlink = 0; /* FIXME: stub */
		statbuf->st_uid = node->UserIdentifier;
		statbuf->st_gid = node->GroupIdentifier;
		statbuf->st_rdev = 0; /* FIXME: stub */
		statbuf->st_size = node->Size;
		statbuf->st_blksize = 0; /* FIXME: stub */
		statbuf->st_blocks = 0;	 /* FIXME: stub */
		statbuf->st_attr = 0;	 /* FIXME: stub */
		return 0;
	}

	int FileDescriptorTable::_dup(int oldfd)
	{
		Fildes fdesc = this->GetFileDescriptor(oldfd);
		DupFildes dfdesc = this->GetDupFildes(oldfd);

		if (fdesc.Descriptor < 0 &&
			dfdesc.Descriptor < 0)
		{
			return -EBADF;
		}

		int newfd = this->GetFreeFileDescriptor();
		if (newfd < 0)
			return -EMFILE;

		DupFildes new_dfd{};
		if (fdesc.Descriptor != -1)
		{
			new_dfd.Handle = fdesc.Handle;
			new_dfd.Mode = fdesc.Mode;
		}
		else
		{
			new_dfd.Handle = dfdesc.Handle;
			new_dfd.Mode = dfdesc.Mode;
		}

		new_dfd.Descriptor = newfd;
		this->FildesDuplicates.push_back(new_dfd);
		debug("Duplicated file descriptor %d to %d",
			  oldfd, newfd);
		return newfd;
	}

	int FileDescriptorTable::_dup2(int oldfd, int newfd)
	{
		Fildes fdesc = this->GetFileDescriptor(oldfd);
		DupFildes dfdesc = this->GetDupFildes(oldfd);

		if (fdesc.Descriptor < 0 &&
			dfdesc.Descriptor < 0)
		{
			return -EBADF;
		}

		if (newfd < 0)
			return -EBADF;

		if (newfd == oldfd)
			return newfd;

		/* Even if it's not valid
			we ignore it. */
		this->_close(newfd);

		DupFildes new_dfd{};
		if (fdesc.Descriptor != -1)
		{
			new_dfd.Handle = fdesc.Handle;
			new_dfd.Mode = fdesc.Mode;
		}
		else
		{
			new_dfd.Handle = dfdesc.Handle;
			new_dfd.Mode = dfdesc.Mode;
		}

		new_dfd.Descriptor = newfd;
		this->FildesDuplicates.push_back(new_dfd);
		debug("Duplicated file descriptor %d to %d",
			  oldfd, newfd);
		return newfd;
	}

	int FileDescriptorTable::_ioctl(int fd, unsigned long request, void *argp)
	{
		Fildes fdesc = this->GetFileDescriptor(fd);
		DupFildes dfdesc = this->GetDupFildes(fd);
		if (fdesc.Descriptor < 0 &&
			dfdesc.Descriptor < 0)
		{
			return -EBADF;
		}

		RefNode *hnd = nullptr;
		if (fdesc.Descriptor != -1)
			hnd = fdesc.Handle;
		else
			hnd = dfdesc.Handle;

		return hnd->ioctl(request, argp);
	}

	FileDescriptorTable::FileDescriptorTable(void *Owner)
	{
		debug("+ %#lx", this);
		this->fdDir = fs->Create("fd", vfs::NodeType::DIRECTORY,
								 ((Tasking::PCB *)Owner)->ProcessDirectory);
	}

	FileDescriptorTable::~FileDescriptorTable()
	{
		debug("- %#lx", this);
		foreach (auto fd in FileDescriptors)
		{
			debug("Removing fd: %d(%#lx)",
				  fd.Descriptor, fd);
			this->RemoveFileDescriptor(fd.Descriptor);
			delete fd.Handle;
		}
	}
}
