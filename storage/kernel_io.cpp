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
#include <errno.h>

#include "../kernel.h"

using Tasking::PCB;
using vfs::FileDescriptorTable;

static bool CheckForScheduler()
{
	if (TaskManager == nullptr)
		return false;
	return true;
}

int fopen(const char *pathname, const char *mode)
{
	if (!CheckForScheduler())
		return -ENOSYS;

	PCB *pcb = thisProcess;
	FileDescriptorTable *fdt = pcb->FileDescriptors;
	int fd = fdt->_open(pathname, ConvertFileFlags(mode), 0666);
	return fd;
}

int creat(const char *pathname, mode_t mode)
{
	if (!CheckForScheduler())
		return -ENOSYS;

	PCB *pcb = thisProcess;
	FileDescriptorTable *fdt = pcb->FileDescriptors;
	int fd = fdt->_creat(pathname, mode);
	return fd;
}

ssize_t fread(int fd, void *buf, size_t count)
{
	if (!CheckForScheduler())
		return -ENOSYS;

	PCB *pcb = thisProcess;
	FileDescriptorTable *fdt = pcb->FileDescriptors;
	ssize_t r = fdt->_read(fd, buf, count);
	return r;
}

ssize_t fwrite(int fd, const void *buf, size_t count)
{
	if (!CheckForScheduler())
		return -ENOSYS;

	PCB *pcb = thisProcess;
	FileDescriptorTable *fdt = pcb->FileDescriptors;
	ssize_t r = fdt->_write(fd, buf, count);
	return r;
}

int fclose(int fd)
{
	if (!CheckForScheduler())
		return -ENOSYS;

	PCB *pcb = thisProcess;
	FileDescriptorTable *fdt = pcb->FileDescriptors;
	int r = fdt->_close(fd);
	return r;
}

off_t lseek(int fd, off_t offset, int whence)
{
	if (!CheckForScheduler())
		return -ENOSYS;

	PCB *pcb = thisProcess;
	FileDescriptorTable *fdt = pcb->FileDescriptors;
	off_t r = fdt->_lseek(fd, offset, whence);
	return r;
}

int stat(const char *pathname, struct stat *statbuf)
{
	if (!CheckForScheduler())
		return -ENOSYS;

	PCB *pcb = thisProcess;
	FileDescriptorTable *fdt = pcb->FileDescriptors;
	int r = fdt->_stat(pathname, statbuf);
	return r;
}

int fstat(int fd, struct stat *statbuf)
{
	if (!CheckForScheduler())
		return -ENOSYS;

	PCB *pcb = thisProcess;
	FileDescriptorTable *fdt = pcb->FileDescriptors;
	int r = fdt->_fstat(fd, statbuf);
	return r;
}

int lstat(const char *pathname, struct stat *statbuf)
{
	if (!CheckForScheduler())
		return -ENOSYS;

	PCB *pcb = thisProcess;
	FileDescriptorTable *fdt = pcb->FileDescriptors;
	int r = fdt->_lstat(pathname, statbuf);
	return r;
}
