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
		virtual size_t read(uint8_t *Buffer,
							size_t Size,
							off_t Offset);
		virtual size_t write(uint8_t *Buffer,
							 size_t Size,
							 off_t Offset);

		NullDevice();
		~NullDevice();
	};

	class RandomDevice : public Node
	{
	public:
		virtual size_t read(uint8_t *Buffer,
							size_t Size,
							off_t Offset);
		virtual size_t write(uint8_t *Buffer,
							 size_t Size,
							 off_t Offset);

		RandomDevice();
		~RandomDevice();
	};

	class ZeroDevice : public Node
	{
	public:
		virtual size_t read(uint8_t *Buffer,
							size_t Size,
							off_t Offset);
		virtual size_t write(uint8_t *Buffer,
							 size_t Size,
							 off_t Offset);

		ZeroDevice();
		~ZeroDevice();
	};

	class TTYDevice : public Node
	{
	public:
		virtual size_t write(uint8_t *Buffer,
							 size_t Size,
							 off_t Offset);
		virtual int ioctl(unsigned long Request,
						  void *Argp);

		TTYDevice();
		~TTYDevice();
	};

	class PTMXDevice : public Node
	{
	private:
		Node *pts = nullptr;

	public:
		virtual size_t read(uint8_t *Buffer,
							size_t Size,
							off_t Offset);
		virtual size_t write(uint8_t *Buffer,
							 size_t Size,
							 off_t Offset);

		PTMXDevice();
		~PTMXDevice();
	};
}

#endif // !__FENNIX_KERNEL_FILESYSTEM_DEV_H__
