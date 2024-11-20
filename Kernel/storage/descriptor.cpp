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
#include <stropts.h>
#include <task.hpp>
#include <printf.h>
#include <lock.hpp>
#include <cwalk.h>

#include "../kernel.h"

namespace vfs
{
	int FileDescriptorTable::GetFlags(int FileDescriptor)
	{
		auto it = this->FileMap.find(FileDescriptor);
		if (it == this->FileMap.end())
			ReturnLogError(-EBADF, "Invalid fd %d", FileDescriptor);

		return it->second.Flags;
	}

	int FileDescriptorTable::SetFlags(int FileDescriptor, int Flags)
	{
		auto it = this->FileMap.find(FileDescriptor);
		if (it == this->FileMap.end())
			ReturnLogError(-EBADF, "Invalid fd %d", FileDescriptor);

		it->second.Flags = Flags;
		return 0;
	}

	int FileDescriptorTable::AddFileDescriptor(const char *AbsolutePath,
											   mode_t Mode, int Flags)
	{
		Tasking::PCB *pcb = thisProcess;

		auto ProbeMode = [](mode_t Mode, int Flags) -> int
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
		};

		if (ProbeMode(Mode, Flags) < 0)
			return -EACCES;

		fixme("Do not follow symlinks when O_CREAT and O_EXCL are set");

		if (Flags & O_CREAT)
		{
			FileNode *ret = fs->Create(pcb->CWD, AbsolutePath, Mode);
			if (Flags & O_EXCL && ret == nullptr)
			{
				debug("%s: File already exists?, returning EEXIST",
					  AbsolutePath);
				return -EEXIST;
			}
		}

		if (Flags & O_CLOEXEC)
		{
			fixme("O_CLOEXEC");
		}

		FileNode *File = fs->GetByPath(AbsolutePath, pcb->CWD);

		if (!File)
		{
			error("Failed to open file %s", AbsolutePath);
			return -ENOENT;
		}

		if (Flags & O_TRUNC)
		{
			debug("Truncating file %s", AbsolutePath);
			File->Truncate(0);
		}

		Fildes fd{};

		if (Flags & O_APPEND)
		{
			debug("Appending to file %s", AbsolutePath);
			struct kstat stat;
			File->Stat(&stat);
			fd.Offset = File->Seek(stat.Size);
		}

		fd.Mode = Mode;
		fd.Flags = Flags;
		fd.Node = File;

		int fdn = this->GetFreeFileDescriptor();
		if (fdn < 0)
			return fdn;

		this->FileMap.insert({fdn, fd});

		char linkName[64];
		snprintf(linkName, 64, "%d", fdn);
		assert(fs->CreateLink(linkName, this->fdDir, AbsolutePath) != nullptr);

		File->Open(Flags, Mode);
		return fdn;
	}

	int FileDescriptorTable::RemoveFileDescriptor(int FileDescriptor)
	{
		auto it = this->FileMap.find(FileDescriptor);
		if (it == this->FileMap.end())
			ReturnLogError(-EBADF, "Invalid fd %d", FileDescriptor);

		fs->Remove(it->second.Node);
		this->FileMap.erase(it);
		return 0;
	}

	int FileDescriptorTable::GetFreeFileDescriptor()
	{
		Tasking::PCB *pcb = thisProcess;

		for (size_t i = 0; i < pcb->SoftLimits.OpenFiles; i++)
		{
			auto it = this->FileMap.find(i);
			if (it == this->FileMap.end())
				return i;
		}

		return -EMFILE;
	}

	void FileDescriptorTable::Fork(FileDescriptorTable *Parent)
	{
		this->FileMap = Parent->FileMap;

		for (const auto &fd : this->FileMap)
		{
			if (fd.second.Flags & O_CLOEXEC)
			{
				debug("O_CLOEXEC flag set, removing fd %d", fd.first);
				this->FileMap.erase(fd.first);
			}
		}
	}

	int FileDescriptorTable::usr_open(const char *pathname, int flags, mode_t mode)
	{
		if (pathname == nullptr)
			return -EFAULT;

		return AddFileDescriptor(pathname, mode, flags);
	}

	int FileDescriptorTable::usr_creat(const char *pathname, mode_t mode)
	{
		return usr_open(pathname, O_WRONLY | O_CREAT | O_TRUNC, mode);
	}

	ssize_t FileDescriptorTable::usr_read(int fd, void *buf, size_t count)
	{
		auto it = this->FileMap.find(fd);
		if (it == this->FileMap.end())
			ReturnLogError(-EBADF, "Invalid fd %d", fd);

		return it->second.Node->Read(buf, count, it->second.Offset);
	}

	ssize_t FileDescriptorTable::usr_write(int fd, const void *buf, size_t count)
	{
		auto it = this->FileMap.find(fd);
		if (it == this->FileMap.end())
			ReturnLogError(-EBADF, "Invalid fd %d", fd);

		return it->second.Node->Write(buf, count, it->second.Offset);
	}

	int FileDescriptorTable::usr_close(int fd)
	{
		auto it = this->FileMap.find(fd);
		if (it == this->FileMap.end())
			ReturnLogError(-EBADF, "Invalid fd %d", fd);

		return RemoveFileDescriptor(fd);
	}

	off_t FileDescriptorTable::usr_lseek(int fd, off_t offset, int whence)
	{
		auto it = this->FileMap.find(fd);
		if (it == this->FileMap.end())
			ReturnLogError(-EBADF, "Invalid fd %d", fd);

		off_t &newOffset = it->second.Offset;

		switch (whence)
		{
		case SEEK_SET:
		{
			newOffset = it->second.Node->Seek(offset);
			break;
		}
		case SEEK_CUR:
		{
			newOffset = it->second.Node->Seek(newOffset + offset);
			break;
		}
		case SEEK_END:
		{
			struct kstat stat
			{
			};
			it->second.Node->Stat(&stat);
			newOffset = it->second.Node->Seek(stat.Size + offset);
			break;
		}
		default:
			return -EINVAL;
		}

		return newOffset;
	}

	int FileDescriptorTable::usr_stat(const char *pathname,
									  struct kstat *statbuf)
	{
		FileNode *node = fs->GetByPath(pathname, nullptr);
		if (node == nullptr)
			ReturnLogError(-ENOENT, "Failed to find %s", pathname);

		if (node->IsSymbolicLink())
		{
			std::unique_ptr<char[]> buffer(new char[1024]);
			ssize_t ret = node->ReadLink(buffer.get(), 1024);
			if (ret < 0)
				return ret;

			FileNode *target = fs->GetByPath(buffer.get(), nullptr);
			if (target == nullptr)
				return -ENOENT;

			return target->Stat(statbuf);
		}

		return node->Stat(statbuf);
	}

	int FileDescriptorTable::usr_fstat(int fd, struct kstat *statbuf)
	{
		auto it = this->FileMap.find(fd);
		if (it == this->FileMap.end())
			ReturnLogError(-EBADF, "Invalid fd %d", fd);

		vfs::FileDescriptorTable::Fildes &fildes = it->second;

		return fildes.Node->Stat(statbuf);
	}

	int FileDescriptorTable::usr_lstat(const char *pathname,
									   struct kstat *statbuf)
	{
		FileNode *node = fs->GetByPath(pathname, nullptr);
		if (node == nullptr)
			ReturnLogError(-ENOENT, "Failed to find %s", pathname);
		return node->Stat(statbuf);
	}

	int FileDescriptorTable::usr_dup(int oldfd)
	{
		auto it = this->FileMap.find(oldfd);
		if (it == this->FileMap.end())
			ReturnLogError(-EBADF, "Invalid fd %d", oldfd);

		int newfd = this->GetFreeFileDescriptor();
		if (newfd < 0)
			return -EMFILE;

		Fildes new_dfd{};
		new_dfd.Node = it->second.Node;
		new_dfd.Mode = it->second.Mode;

		this->FileMap.insert({newfd, new_dfd});

		debug("Duplicated file descriptor %d to %d", oldfd, newfd);
		return newfd;
	}

	int FileDescriptorTable::usr_dup2(int oldfd, int newfd)
	{
		if (newfd < 0)
			ReturnLogError(-EBADF, "Invalid newfd %d", newfd);

		auto it = this->FileMap.find(oldfd);
		if (it == this->FileMap.end())
			ReturnLogError(-EBADF, "Invalid oldfd %d", oldfd);

		if (newfd == oldfd)
			return newfd;

		/* Even if it's not valid we ignore it. */
		this->usr_close(newfd);

		Fildes new_dfd{};
		new_dfd.Node = it->second.Node;
		new_dfd.Mode = it->second.Mode;

		this->FileMap.insert({newfd, new_dfd});
		debug("Duplicated file descriptor %d to %d", oldfd, newfd);
		return newfd;
	}

	int FileDescriptorTable::usr_ioctl(int fd, unsigned long request, void *argp)
	{
		auto it = this->FileMap.find(fd);
		if (it == this->FileMap.end())
			ReturnLogError(-EBADF, "Invalid fd %d", fd);

		return it->second.Node->Ioctl(request, argp);
	}

	FileDescriptorTable::FileDescriptorTable(void *_Owner)
		: Owner(_Owner)
	{
		debug("+ %#lx", this);

		mode_t Mode = S_IXOTH | S_IROTH |
					  S_IXGRP | S_IRGRP |
					  S_IXUSR | S_IRUSR |
					  S_IFDIR;

		this->fdDir = fs->Create(((Tasking::PCB *)_Owner)->ProcDirectory, "fd", Mode);
	}
}
