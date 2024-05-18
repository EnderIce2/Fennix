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

#ifndef __FENNIX_KERNEL_FILESYSTEM_DEV_H__
#define __FENNIX_KERNEL_FILESYSTEM_DEV_H__

#include <types.h>

#include <filesystem.hpp>
#include <bitmap.hpp>
#include <termios.h>
#include <lock.hpp>
#include <vector>

namespace vfs
{
	class PTYDevice
	{
	private:
		Inode *pts;
		int id;
		termios term{};
		winsize termSize{};

		class PTYSlave
		{
		};

		class PTYMaster
		{
		};

	public:
		decltype(id) &ptyId = id;

		ssize_t Read(struct Inode *Node, void *Buffer, size_t Size, off_t Offset);
		ssize_t Write(struct Inode *Node, const void *Buffer, size_t Size, off_t Offset);
		int Ioctl(struct Inode *Node, unsigned long Request, void *Argp);

		PTYDevice(Inode *pts, int id);
		~PTYDevice();
	};

	class PTMXDevice
	{
	private:
		NewLock(PTMXLock);
		FileNode *ptmx;
		FileNode *pts;
		Bitmap ptysId;
		std::unordered_map<size_t, PTYDevice *> ptysList;

	public:
		int Open(struct Inode *Node, int Flags, mode_t Mode, struct Inode *Result);
		int Close(struct Inode *Node);

		PTMXDevice();
		~PTMXDevice();
	};
}

#endif // !__FENNIX_KERNEL_FILESYSTEM_DEV_H__
