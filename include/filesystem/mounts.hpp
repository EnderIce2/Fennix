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

#include <filesystem/termios.hpp>
#include <filesystem.hpp>
#include <bitmap.hpp>
#include <task.hpp>
#include <lock.hpp>
#include <vector>

namespace vfs
{
	class vfsRoot : public Node
	{
	public:
		vfsRoot(const char *Name, Virtual *vfs_ctx);
		~vfsRoot() {}
	};

	class NullDevice : public Node
	{
	public:
		size_t read(uint8_t *Buffer,
					size_t Size,
					off_t Offset) final;
		size_t write(uint8_t *Buffer,
					 size_t Size,
					 off_t Offset) final;

		NullDevice();
		~NullDevice();
	};

	class RandomDevice : public Node
	{
	public:
		size_t read(uint8_t *Buffer,
					size_t Size,
					off_t Offset) final;
		size_t write(uint8_t *Buffer,
					 size_t Size,
					 off_t Offset) final;

		RandomDevice();
		~RandomDevice();
	};

	class ZeroDevice : public Node
	{
	public:
		size_t read(uint8_t *Buffer,
					size_t Size,
					off_t Offset) final;
		size_t write(uint8_t *Buffer,
					 size_t Size,
					 off_t Offset) final;

		ZeroDevice();
		~ZeroDevice();
	};

	class KConDevice : public Node
	{
	public:
		size_t read(uint8_t *Buffer,
					size_t Size,
					off_t Offset) final;
		size_t write(uint8_t *Buffer,
					 size_t Size,
					 off_t Offset) final;
		int ioctl(unsigned long Request,
				  void *Argp) final;

		termios term{};
		winsize termSize{};

		KConDevice();
		~KConDevice();
	};

	class TTYDevice : public Node
	{
	public:
		size_t write(uint8_t *Buffer,
					 size_t Size,
					 off_t Offset) final;
		int ioctl(unsigned long Request,
				  void *Argp) final;

		TTYDevice();
		~TTYDevice();
	};

	class MasterPTY
	{
		NewLock(PTYLock);

	public:
		size_t read(uint8_t *Buffer,
					size_t Size,
					off_t Offset);
		size_t write(uint8_t *Buffer,
					 size_t Size,
					 off_t Offset);

		MasterPTY();
		~MasterPTY();
	};

	class SlavePTY
	{
		NewLock(PTYLock);

	public:
		size_t read(uint8_t *Buffer,
					size_t Size,
					off_t Offset);
		size_t write(uint8_t *Buffer,
					 size_t Size,
					 off_t Offset);

		SlavePTY();
		~SlavePTY();
	};

	class PTYDevice : public Node
	{
	private:
		Node *pts;
		int id;
		int fildes;
		bool isMaster;
		termios term{};
		winsize termSize{};

		MasterPTY *MasterDev;
		SlavePTY *SlaveDev;

	public:
		decltype(id) &ptyId = id;
		decltype(fildes) &fd = fildes;

		int open(int Flags, mode_t Mode) final;
		int close() final;
		size_t read(uint8_t *Buffer,
					size_t Size,
					off_t Offset) final;
		size_t write(uint8_t *Buffer,
					 size_t Size,
					 off_t Offset) final;
		int ioctl(unsigned long Request,
				  void *Argp) final;

		int OpenMaster(int Flags, mode_t Mode);

		PTYDevice(Node *pts, int id);
		~PTYDevice();
	};

	class PTMXDevice : public Node
	{
	private:
		NewLock(PTMXLock);
		Node *pts;
		Bitmap ptysId;
		std::vector<PTYDevice *> ptysList;

	public:
		int open(int Flags, mode_t Mode) final;

		/**
		 * Remove a PTY from the list
		 *
		 * @param fd The file descriptor of the PTY
		 * @param pcb The process that owns the PTY
		 *
		 * @note if pcb is nullptr, the current process
		 * will be used.
		 *
		 */
		void RemovePTY(int fd, Tasking::PCB *pcb = nullptr);

		PTMXDevice();
		~PTMXDevice();
	};
}

#endif // !__FENNIX_KERNEL_FILESYSTEM_DEV_H__
