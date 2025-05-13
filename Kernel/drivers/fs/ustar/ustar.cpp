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

#include <driver.hpp>
#include <fs/ustar.hpp>
#include <interface/fs.h>
#include <memory.hpp>
#include <debug.h>

using namespace vfs;

namespace Driver::UnixStandardTAR
{
	dev_t DriverID;

	int USTAR_AllocateInode(FileSystemInfo *Info, Inode **Result)
	{
		assert(!"NOT IMPLEMENTED");
	}

	int USTAR_DeleteInode(FileSystemInfo *Info, Inode *Node)
	{
		assert(!"NOT IMPLEMENTED");
	}

	int USTAR_Synchronize(FileSystemInfo *Info, Inode *Node)
	{
		assert(!"NOT IMPLEMENTED");
	}

	int USTAR_Destroy(FileSystemInfo *Info)
	{
		assert(!"NOT IMPLEMENTED");
	}

	int USTAR_Probe(void *Device)
	{
		assert(!"NOT IMPLEMENTED");
	}

	int USTAR_Mount(FileSystemInfo *FS, Inode **Root, void *Device)
	{
		assert(!"NOT IMPLEMENTED");
	}

	int USTAR_Unmount(FileSystemInfo *FS)
	{
		assert(!"NOT IMPLEMENTED");
	}

	int USTAR_Lookup(Inode *Parent, const char *Name, Inode **Result)
	{
		assert(!"NOT IMPLEMENTED");
	}

	int USTAR_Create(Inode *Parent, const char *Name, mode_t Mode, Inode **Result)
	{
		assert(!"NOT IMPLEMENTED");
	}

	int USTAR_Remove(Inode *Parent, const char *Name)
	{
		assert(!"NOT IMPLEMENTED");
	}

	int USTAR_Rename(Inode *Parent, const char *OldName, const char *NewName)
	{
		assert(!"NOT IMPLEMENTED");
	}

	ssize_t USTAR_Read(Inode *Node, void *Buffer, size_t Size, off_t Offset)
	{
		assert(!"NOT IMPLEMENTED");
	}

	ssize_t USTAR_Write(Inode *Node, const void *Buffer, size_t Size, off_t Offset)
	{
		assert(!"NOT IMPLEMENTED");
	}

	int USTAR_Truncate(Inode *Node, off_t Size)
	{
		assert(!"NOT IMPLEMENTED");
	}

	int USTAR_Open(Inode *Node, int Flags, mode_t Mode)
	{
		assert(!"NOT IMPLEMENTED");
	}

	int USTAR_Close(Inode *Node)
	{
		assert(!"NOT IMPLEMENTED");
	}

	int USTAR_Ioctl(Inode *Node, unsigned long Request, void *Argp)
	{
		assert(!"NOT IMPLEMENTED");
	}

	ssize_t USTAR_ReadDir(Inode *Node, kdirent *Buffer, size_t Size, off_t Offset, off_t Entries)
	{
		assert(!"NOT IMPLEMENTED");
	}

	int USTAR_MkDir(Inode *Parent, const char *Name, mode_t Mode, Inode **Result)
	{
		assert(!"NOT IMPLEMENTED");
	}

	int USTAR_RmDir(Inode *Parent, const char *Name)
	{
		assert(!"NOT IMPLEMENTED");
	}

	int USTAR_SymLink(Inode *Parent, const char *Name, const char *Target, Inode **Result)
	{
		assert(!"NOT IMPLEMENTED");
	}

	ssize_t USTAR_ReadLink(Inode *Node, char *Buffer, size_t Size)
	{
		assert(!"NOT IMPLEMENTED");
	}

	off_t USTAR_Seek(Inode *Node, off_t Offset)
	{
		assert(!"NOT IMPLEMENTED");
	}

	int USTAR_Stat(Inode *Node, kstat *Stat)
	{
		assert(!"NOT IMPLEMENTED");
	}

	static SuperBlockOperations ustarSuperOps = {
		.AllocateInode = USTAR_AllocateInode,
		.DeleteInode = USTAR_DeleteInode,
		.Synchronize = USTAR_Synchronize,
		.Destroy = USTAR_Destroy,
		.Probe = USTAR_Probe,
		.Mount = USTAR_Mount,
		.Unmount = USTAR_Unmount};

	static InodeOperations ustarInodeOps = {
		.Lookup = USTAR_Lookup,
		.Create = USTAR_Create,
		.Remove = USTAR_Remove,
		.Rename = USTAR_Rename,
		.Read = USTAR_Read,
		.Write = USTAR_Write,
		.Truncate = USTAR_Truncate,
		.Open = USTAR_Open,
		.Close = USTAR_Close,
		.Ioctl = USTAR_Ioctl,
		.ReadDir = USTAR_ReadDir,
		.MkDir = USTAR_MkDir,
		.RmDir = USTAR_RmDir,
		.SymLink = USTAR_SymLink,
		.ReadLink = USTAR_ReadLink,
		.Seek = USTAR_Seek,
		.Stat = USTAR_Stat};

	int Entry()
	{
		FileSystemInfo *fsi = new FileSystemInfo;
		fsi->Name = "Unix Standard TAR";
		fsi->SuperOps = ustarSuperOps;
		fsi->Ops = ustarInodeOps;
		v0::RegisterFileSystem(DriverID, fsi);
		return 0;
	}

	int Final() { return 0; }
	int Panic() { return 0; }
	int Probe() { return 0; }

	REGISTER_BUILTIN_DRIVER(ustar,
							"Unix Standard TAR Driver",
							"enderice2",
							1, 0, 0,
							Entry,
							Final,
							Panic,
							Probe);
}
