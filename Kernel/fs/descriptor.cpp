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

#include <fs/vfs.hpp>

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

	int FileDescriptorTable::AddFileDescriptor(const char *AbsolutePath, mode_t Mode, int Flags)
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
				if (!(Mode & S_IRUSR) || !(Mode & S_IWUSR))
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
			eNode ret = fs->Create(pcb->CWD, AbsolutePath, Mode);
			if (Flags & O_EXCL && ret == false)
			{
				debug("%s: File already exists?, returning %s", AbsolutePath, ret.what());
				return -ret.Error;
			}
		}

		if (Flags & O_CLOEXEC)
		{
			fixme("O_CLOEXEC");
		}

		eNode ret = fs->Lookup(pcb->CWD, AbsolutePath);
		if (ret == false)
		{
			error("Failed to open file %s, %s", AbsolutePath, ret.what());
			return -ret.Error;
		}
		Node node = ret;

		if (Flags & O_TRUNC)
		{
			debug("Truncating file %s", AbsolutePath);
			fs->Truncate(node, 0);
		}

		Fildes fd{};

		if (Flags & O_APPEND)
		{
			debug("Appending to file %s", AbsolutePath);
			struct kstat stat;
			fs->Stat(node, &stat);
			fd.Offset = fs->Seek(node, stat.Size);
		}

		fd.Mode = Mode;
		fd.Flags = Flags;
		fd.node = node;

		int fdn = this->GetFreeFileDescriptor();
		if (fdn < 0)
			return fdn;

		this->FileMap.insert({fdn, fd});

		char linkName[64];
		snprintf(linkName, 64, "%d", fdn);
		assert(fs->CreateLink(this->fdDir, linkName, AbsolutePath) == true);
		fs->Open(node, Flags, Mode);
		return fdn;
	}

	int FileDescriptorTable::RemoveFileDescriptor(int FileDescriptor)
	{
		auto it = this->FileMap.find(FileDescriptor);
		if (it == this->FileMap.end())
			ReturnLogError(-EBADF, "Invalid fd %d", FileDescriptor);

		fs->Remove(it->second.node);
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

		return fs->Read(it->second.node, buf, count, it->second.Offset);
	}

	ssize_t FileDescriptorTable::usr_pread(int fd, void *buf, size_t count, off_t offset)
	{
		auto it = this->FileMap.find(fd);
		if (it == this->FileMap.end())
			ReturnLogError(-EBADF, "Invalid fd %d", fd);

		return fs->Read(it->second.node, buf, count, offset);
	}

	ssize_t FileDescriptorTable::usr_write(int fd, const void *buf, size_t count)
	{
		auto it = this->FileMap.find(fd);
		if (it == this->FileMap.end())
			ReturnLogError(-EBADF, "Invalid fd %d", fd);

		return fs->Write(it->second.node, buf, count, it->second.Offset);
	}

	ssize_t FileDescriptorTable::usr_pwrite(int fd, const void *buf, size_t count, off_t offset)
	{
		auto it = this->FileMap.find(fd);
		if (it == this->FileMap.end())
			ReturnLogError(-EBADF, "Invalid fd %d", fd);

		return fs->Write(it->second.node, buf, count, offset);
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
			newOffset = fs->Seek(it->second.node, offset);
			break;
		}
		case SEEK_CUR:
		{
			newOffset = fs->Seek(it->second.node, newOffset + offset);
			break;
		}
		case SEEK_END:
		{
			struct kstat stat{};
			fs->Stat(it->second.node, &stat);
			newOffset = fs->Seek(it->second.node, stat.Size + offset);
			break;
		}
		default:
			return -EINVAL;
		}

		return newOffset;
	}

	int FileDescriptorTable::usr_stat(const char *pathname, kstat *statbuf)
	{
		Node root = thisProcess->Info.RootNode;
		eNode ret = fs->Lookup(root, pathname);
		if (ret == false)
			ReturnLogError(-ret.Error, "Error on %s, %s", pathname, ret.what());
		Node node = ret;

		if (node->IsSymbolicLink())
		{
			std::unique_ptr<char[]> buffer(new char[1024]);
			ssize_t len = fs->ReadLink(node, buffer.get(), 1024);
			if (len < 0)
				return len;

			ret = fs->Lookup(root, buffer.get());
			if (ret == false)
				return -ret.Error;
			return fs->Stat(ret.Value, statbuf);
		}

		return fs->Stat(node, statbuf);
	}

	int FileDescriptorTable::usr_fstat(int fd, kstat *statbuf)
	{
		auto it = this->FileMap.find(fd);
		if (it == this->FileMap.end())
			ReturnLogError(-EBADF, "Invalid fd %d", fd);

		vfs::FileDescriptorTable::Fildes &fildes = it->second;
		return fs->Stat(fildes.node, statbuf);
	}

	int FileDescriptorTable::usr_lstat(const char *pathname, kstat *statbuf)
	{
		Node root = thisProcess->Info.RootNode;
		eNode ret = fs->Lookup(root, pathname);
		if (ret == false)
			ReturnLogError(-ret.Error, "Error on %s, %s", pathname, ret.what());

		return fs->Stat(ret.Value, statbuf);
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
		new_dfd.node = it->second.node;
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
		new_dfd.node = it->second.node;
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

		return fs->Ioctl(it->second.node, request, argp);
	}

	FileDescriptorTable::FileDescriptorTable(void *_Owner)
		: Owner(_Owner)
	{
		debug("+ %#lx", this);

		/* d r-x r-x r-x */
		mode_t Mode = S_IROTH | S_IXOTH |
					  S_IRGRP | S_IXGRP |
					  S_IRUSR | S_IXUSR |
					  S_IFDIR;
		Tasking::PCB *pcb = (Tasking::PCB *)_Owner;
		this->fdDir = fs->Create(pcb->ProcDirectory, "fd", Mode);
	}
}
