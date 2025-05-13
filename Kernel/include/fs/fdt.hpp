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

#pragma once

#include <fs/node.hpp>
#include <unordered_map>

namespace vfs
{
	class FileDescriptorTable
	{
	public:
		struct Fildes
		{
			enum FildesType
			{
				FD_INODE,
				FD_PIPE,
				FD_SOCKET,
			} Type;
			mode_t Mode = 0;
			int Flags = 0;
			Node node = nullptr;
			int References = 0;
			off_t Offset = 0;

			int operator==(const Fildes &other)
			{
				return Type == other.Type &&
					   Mode == other.Mode &&
					   Flags == other.Flags &&
					   node.get() == other.node.get() &&
					   References == other.References &&
					   Offset == other.Offset;
			}
		};

	private:
		Node fdDir = nullptr;
		void *Owner;

		int AddFileDescriptor(const char *AbsolutePath, mode_t Mode, int Flags);
		int RemoveFileDescriptor(int FileDescriptor);
		int GetFreeFileDescriptor();

	public:
		std::unordered_map<int, Fildes> FileMap;

		int GetFlags(int FileDescriptor);
		int SetFlags(int FileDescriptor, int Flags);
		void Fork(FileDescriptorTable *Parent);

		int usr_open(const char *pathname, int flags, mode_t mode);
		int usr_creat(const char *pathname, mode_t mode);
		ssize_t usr_read(int fd, void *buf, size_t count);
		ssize_t usr_write(int fd, const void *buf, size_t count);
		ssize_t usr_pread(int fd, void *buf, size_t count, off_t offset);
		ssize_t usr_pwrite(int fd, const void *buf, size_t count, off_t offset);
		int usr_close(int fd);
		off_t usr_lseek(int fd, off_t offset, int whence);
		int usr_stat(const char *pathname, kstat *statbuf);
		int usr_fstat(int fd, kstat *statbuf);
		int usr_lstat(const char *pathname, kstat *statbuf);
		int usr_dup(int oldfd);
		int usr_dup2(int oldfd, int newfd);
		int usr_ioctl(int fd, unsigned long request, void *argp);

		FileDescriptorTable(void *Owner);
		~FileDescriptorTable() = default;
	};
}
