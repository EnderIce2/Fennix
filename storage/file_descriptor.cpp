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

	FileDescriptorTable::Fildes &
	FileDescriptorTable::GetFileDescriptor(int FileDescriptor)
	{
		foreach (auto &fd in FileDescriptors)
		{
			if (fd.Descriptor == FileDescriptor)
			{
				// debug("Found file descriptor %d", FileDescriptor);
				return fd;
			}
		}
		return nullfd;
	}

	FileDescriptorTable::Fildes &
	FileDescriptorTable::GetDupFildes(int FileDescriptor)
	{
		foreach (auto &fd in FildesDuplicates)
		{
			if (fd.Descriptor == FileDescriptor)
			{
				debug("Found duplicated file descriptor %d", FileDescriptor);
				return fd;
			}
		}
		return nullfd;
	}

	FileDescriptorTable::Fildes &
	FileDescriptorTable::GetDescriptor(int FileDescriptor)
	{
		Fildes &fd = this->GetFileDescriptor(FileDescriptor);
		Fildes &dfd = this->GetDupFildes(FileDescriptor);

		if (fd.Descriptor == -1 &&
			dfd.Descriptor == -1)
			return nullfd;

		if (fd.Descriptor != -1)
			return fd;
		else
			return dfd;
	}

	int FileDescriptorTable::GetFlags(int FileDescriptor)
	{
		Fildes &fd = this->GetDescriptor(FileDescriptor);
		if (fd == nullfd)
		{
			debug("invalid fd %d", FileDescriptor);
			return -EBADF;
		}

		return fd.Flags;
	}

	int FileDescriptorTable::SetFlags(int FileDescriptor, int Flags)
	{
		Fildes &fd = this->GetDescriptor(FileDescriptor);
		if (fd == nullfd)
		{
			debug("invalid fd %d", FileDescriptor);
			return -EBADF;
		}

		fd.Flags = Flags;
		return 0;
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
			bool absolute = cwk_path_is_absolute(AbsolutePath);
			new Node(pcb->CurrentWorkingDirectory,
					 AbsolutePath, NodeType::FILE,
					 absolute, fs, &ret);

			if (ret == -EEXIST)
			{
				debug("%s: File already exists, continuing...",
					  AbsolutePath);
			}
			else if (ret < 0)
			{
				error("Failed to create file %s: %d",
					  AbsolutePath, ret);
				assert(ret < 0);
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
				return -ENOENT;
			}
			delete File;
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
			return -ENOENT;
		}

		Fildes fd = {.Descriptor = GetFreeFileDescriptor()};

		if (fd.Descriptor < 0)
			return -EMFILE;

		fd.Mode = Mode;
		fd.Flags = Flags;
		fd.Handle = File;

		FileDescriptors.push_back(fd);

		char FileName[64];
		itoa(fd.Descriptor, FileName, 10);
		assert(fs->CreateLink(FileName, AbsolutePath, this->fdDir) != nullptr);

		int rfd = File->node->open(Flags, Mode);
		if (rfd <= 0)
			return fd.Descriptor;
		else
			return rfd;
	}

	int FileDescriptorTable::RemoveFileDescriptor(int FileDescriptor)
	{
		forItr(itr, FileDescriptors)
		{
			if (itr->Descriptor == FileDescriptor)
			{
				FileDescriptors.erase(itr);

				char FileName[64];
				itoa(FileDescriptor, FileName, 10);
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
				itoa(FileDescriptor, FileName, 10);
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
		Fildes &fd = this->GetDescriptor(FileDescriptor);
		if (fd == nullfd)
			return "";

		Node *node = fd.Handle->node;
		const char *path = new char[strlen(node->FullPath) + 1];
		strcpy((char *)path, node->FullPath);
		return path;
	}

	RefNode *FileDescriptorTable::GetRefNode(int FileDescriptor)
	{
		Fildes &fd = this->GetDescriptor(FileDescriptor);
		if (fd == nullfd)
			return nullptr;

		return fd.Handle;
	}

	void FileDescriptorTable::Fork(FileDescriptorTable *Parent)
	{
		foreach (auto &fd in Parent->FileDescriptors)
		{
			debug("Forking fd: %d", fd.Descriptor);
			RefNode *node = fs->Open(fd.Handle->node->FullPath,
									 thisProcess->CurrentWorkingDirectory);
			assert(node != nullptr);

			Fildes new_fd;
			new_fd.Descriptor = fd.Descriptor;
			new_fd.Flags = fd.Flags;
			new_fd.Mode = fd.Mode;
			new_fd.Handle = node;
			this->FileDescriptors.push_back(new_fd);
		}

		foreach (auto &fd in Parent->FildesDuplicates)
		{
			debug("Forking duplicated fd: %d", fd.Descriptor);
			RefNode *node = fs->Open(fd.Handle->node->FullPath,
									 thisProcess->CurrentWorkingDirectory);
			assert(node != nullptr);

			Fildes new_fd;
			new_fd.Descriptor = fd.Descriptor;
			new_fd.Flags = fd.Flags;
			new_fd.Mode = fd.Mode;
			new_fd.Handle = node;
			this->FildesDuplicates.push_back(new_fd);
		}
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

	ssize_t FileDescriptorTable::_read(int _fd, void *buf, size_t count)
	{
		Fildes &fd = this->GetDescriptor(_fd);
		if (fd == nullfd)
		{
			debug("invalid fd %d", _fd);
			return -EBADF;
		}

		return fd.Handle->read((uint8_t *)buf, count);
	}

	ssize_t FileDescriptorTable::_write(int _fd, const void *buf,
										size_t count)
	{
		Fildes &fd = this->GetDescriptor(_fd);
		if (fd == nullfd)
		{
			debug("invalid fd %d", _fd);
			return -EBADF;
		}

		return fd.Handle->write((uint8_t *)buf, count);
	}

	int FileDescriptorTable::_close(int _fd)
	{
		Fildes &fd = this->GetDescriptor(_fd);
		if (fd == nullfd)
		{
			debug("invalid fd %d", _fd);
			return -EBADF;
		}

		if (RemoveFileDescriptor(_fd) < 0)
		{
			debug("invalid fd %d", _fd);
			return -EBADF;
		}

		bool Found = false;
		foreach (auto dfd in FileDescriptors)
		{
			if (dfd.Handle == fd.Handle)
			{
				Found = true;
				break;
			}
		}

		if (!Found)
			foreach (auto dfd in FildesDuplicates)
			{
				if (dfd.Handle == fd.Handle)
				{
					Found = true;
					break;
				}
			}

		/* If the file descriptor is a duplicate,
			we don't need to close the handle,
			because it's a duplicate of another
			file descriptor. */
		if (!Found)
			delete fd.Handle;
		return 0;
	}

	off_t FileDescriptorTable::_lseek(int _fd, off_t offset, int whence)
	{
		Fildes &fd = this->GetDescriptor(_fd);
		if (fd == nullfd)
		{
			debug("invalid fd %d", _fd);
			return -EBADF;
		}

		return fd.Handle->seek(offset, whence);
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
			return -ENOENT;
		}

		Node *node = file->node;
		statbuf->st_dev = 0; /* FIXME: stub */
		statbuf->st_ino = node->IndexNode;
		statbuf->st_mode = node->Type | (node->Mode & ~S_IFMT);
		statbuf->st_nlink = 0; /* FIXME: stub */
		statbuf->st_uid = node->UserIdentifier;
		statbuf->st_gid = node->GroupIdentifier;
		statbuf->st_rdev = 0; /* FIXME: stub */
		statbuf->st_size = node->Size;
		statbuf->st_atime = node->AccessTime;
		statbuf->st_mtime = node->ModifyTime;
		statbuf->st_ctime = node->ChangeTime;
		statbuf->st_blksize = 0; /* FIXME: stub */
		statbuf->st_blocks = 0;	 /* FIXME: stub */
		statbuf->st_attr = 0;	 /* FIXME: stub */
		return 0;
	}

	int FileDescriptorTable::_fstat(int _fd, struct stat *statbuf)
	{
		Fildes &fd = this->GetDescriptor(_fd);
		if (fd == nullfd)
		{
			debug("invalid fd %d", _fd);
			return -EBADF;
		}

		Node *node = fd.Handle->node;
		statbuf->st_dev = 0; /* FIXME: stub */
		statbuf->st_ino = node->IndexNode;
		statbuf->st_mode = node->Type | (node->Mode & ~S_IFMT);
		statbuf->st_nlink = 0; /* FIXME: stub */
		statbuf->st_uid = node->UserIdentifier;
		statbuf->st_gid = node->GroupIdentifier;
		statbuf->st_rdev = 0; /* FIXME: stub */
		statbuf->st_size = node->Size;
		statbuf->st_atime = node->AccessTime;
		statbuf->st_mtime = node->ModifyTime;
		statbuf->st_ctime = node->ChangeTime;
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
			return -ENOENT;
		}

		Node *node = file->node;
		statbuf->st_dev = 0; /* FIXME: stub */
		statbuf->st_ino = node->IndexNode;
		statbuf->st_mode = node->Type | (node->Mode & ~S_IFMT);
		statbuf->st_nlink = 0; /* FIXME: stub */
		statbuf->st_uid = node->UserIdentifier;
		statbuf->st_gid = node->GroupIdentifier;
		statbuf->st_rdev = 0; /* FIXME: stub */
		statbuf->st_size = node->Size;
		statbuf->st_atime = node->AccessTime;
		statbuf->st_mtime = node->ModifyTime;
		statbuf->st_ctime = node->ChangeTime;
		statbuf->st_blksize = 0; /* FIXME: stub */
		statbuf->st_blocks = 0;	 /* FIXME: stub */
		statbuf->st_attr = 0;	 /* FIXME: stub */
		return 0;
	}

	int FileDescriptorTable::_dup(int oldfd)
	{
		Fildes &fd = this->GetDescriptor(oldfd);
		if (fd == nullfd)
		{
			debug("invalid fd %d", oldfd);
			return -EBADF;
		}

		int newfd = this->GetFreeFileDescriptor();
		if (newfd < 0)
			return -EMFILE;

		Fildes new_dfd{};
		new_dfd.Handle = fd.Handle;
		new_dfd.Mode = fd.Mode;

		new_dfd.Descriptor = newfd;
		this->FildesDuplicates.push_back(new_dfd);
		debug("Duplicated file descriptor %d to %d",
			  oldfd, newfd);
		return newfd;
	}

	int FileDescriptorTable::_dup2(int oldfd, int newfd)
	{
		Fildes &fd = this->GetDescriptor(oldfd);
		if (fd == nullfd)
		{
			debug("invalid fd %d", oldfd);
			return -EBADF;
		}

		if (newfd < 0)
		{
			debug("invalid fd %d", newfd);
			return -EBADF;
		}

		if (newfd == oldfd)
			return newfd;

		/* Even if it's not valid
			we ignore it. */
		this->_close(newfd);

		Fildes new_dfd{};
		new_dfd.Handle = fd.Handle;
		new_dfd.Mode = fd.Mode;

		new_dfd.Descriptor = newfd;
		this->FildesDuplicates.push_back(new_dfd);
		debug("Duplicated file descriptor %d to %d",
			  oldfd, newfd);
		return newfd;
	}

	int FileDescriptorTable::_ioctl(int _fd, unsigned long request, void *argp)
	{
		Fildes &fd = this->GetDescriptor(_fd);
		if (fd == nullfd)
		{
			debug("invalid fd %d", _fd);
			return -EBADF;
		}

		return fd.Handle->ioctl(request, argp);
	}

	FileDescriptorTable::FileDescriptorTable(void *Owner)
	{
		debug("+ %#lx", this);
		this->fdDir = fs->Create("fd", vfs::NodeType::DIRECTORY,
								 ((Tasking::PCB *)Owner));
	}

	FileDescriptorTable::~FileDescriptorTable()
	{
		debug("- %#lx", this);
		foreach (auto &fd in FileDescriptors)
		{
			debug("Removing fd: %d", fd.Descriptor);
			this->RemoveFileDescriptor(fd.Descriptor);
			delete fd.Handle;
		}
	}
}
