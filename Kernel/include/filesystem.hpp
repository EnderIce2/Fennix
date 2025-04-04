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

#ifndef __FENNIX_KERNEL_FILESYSTEM_H__
#define __FENNIX_KERNEL_FILESYSTEM_H__

#include <types.h>

#include <interface/fs.h>
#include <unordered_map>
#include <lock.hpp>
#include <errno.h>
#include <atomic>
#include <string>
#include <list>

static_assert(DTTOIF(DT_FIFO) == S_IFIFO);
static_assert(IFTODT(S_IFCHR) == DT_CHR);

/**
 * This macro is used to check if a filesystem operation is available.
 *
 * TL;DR
 *
 * @code
 * if FileSystemInfo.Ops.op == nullptr
 *     return -err
 * else
 *     return FileSystemInfo.Ops.op(this->Node, ...);
 * @endcode
 *
 * @param op The operation to check.
 * @param err The error to return if the operation is not available.
 * @param ... The arguments to pass to the operation.
 *
 * @return The result of the operation.
 */
#define __check_op(op, err, ...) \
	if (fsi->Ops.op == nullptr)  \
		return -err;             \
	else                         \
		return fsi->Ops.op(this->Node, ##__VA_ARGS__)

#define FSROOT(num) "\x06root-" #num "\x06"

class FileNode
{
public:
	std::string Name, Path;
	FileNode *Parent;
	std::vector<FileNode *> Children;
	Inode *Node;
	FileSystemInfo *fsi;

	std::string GetName();
	std::string GetPath();

	bool IsDirectory() { return S_ISDIR(Node->Mode); }
	bool IsCharacterDevice() { return S_ISCHR(Node->Mode); }
	bool IsBlockDevice() { return S_ISBLK(Node->Mode); }
	bool IsRegularFile() { return S_ISREG(Node->Mode); }
	bool IsFIFO() { return S_ISFIFO(Node->Mode); }
	bool IsSymbolicLink() { return S_ISLNK(Node->Mode); }
	bool IsSocket() { return S_ISSOCK(Node->Mode); }

	int Lookup(const char *Name, Inode **Node) { __check_op(Lookup, ENOTSUP, Name, Node); }
	int Create(const char *Name, mode_t Mode, Inode **Node) { __check_op(Create, EROFS, Name, Mode, Node); }
	int Remove(const char *Name) { __check_op(Remove, EROFS, Name); }
	int Rename(const char *OldName, const char *NewName) { __check_op(Rename, EROFS, OldName, NewName); }
	ssize_t Read(auto Buffer, size_t Size, off_t Offset) { __check_op(Read, ENOTSUP, (void *)Buffer, Size, Offset); }
	ssize_t Write(const auto Buffer, size_t Size, off_t Offset) { __check_op(Write, EROFS, (const void *)Buffer, Size, Offset); }
	int Truncate(off_t Size) { __check_op(Truncate, EROFS, Size); }
	int Open(int Flags, mode_t Mode) { __check_op(Open, ENOTSUP, Flags, Mode); }
	int Close() { __check_op(Close, ENOTSUP); }
	int Ioctl(unsigned long Request, void *Argp) { __check_op(Ioctl, ENOTSUP, Request, Argp); }
	ssize_t ReadDir(struct kdirent *Buffer, size_t Size, off_t Offset, off_t Entries) { __check_op(ReadDir, ENOTSUP, Buffer, Size, Offset, Entries); }
	int MkDir(const char *Name, mode_t Mode, struct Inode **Result) { __check_op(MkDir, EROFS, Name, Mode, Result); }
	int RmDir(const char *Name) { __check_op(RmDir, EROFS, Name); }
	int SymLink(const char *Name, const char *Target, struct Inode **Result) { __check_op(SymLink, EROFS, Name, Target, Result); }
	ssize_t ReadLink(auto Buffer, size_t Size) { __check_op(ReadLink, ENOTSUP, (char *)Buffer, Size); }
	off_t Seek(off_t Offset) { __check_op(Seek, ENOTSUP, Offset); }
	int Stat(struct kstat *Stat) { __check_op(Stat, ENOTSUP, Stat); }

	~FileNode() = delete;
};

#undef __check_op

namespace vfs
{
	struct vfsInode
	{
		Inode Node;
		std::string Name;
		std::string FriendlyName;
		std::vector<Inode *> Children;
	};

	class Virtual
	{
	private:
		NewLock(VirtualLock);

		struct FSMountInfo
		{
			FileSystemInfo *fsi;
			Inode *Root;
		};

		struct CacheNode
		{
			FileNode *fn;
			std::atomic_int References;
		};

		std::unordered_map<dev_t, FSMountInfo> DeviceMap;
		std::atomic_bool RegisterLock = false;

		FileNode *CacheSearchReturnLast(FileNode *Parent, const char **Path);
		FileNode *CacheRecursiveSearch(FileNode *Root, const char *NameOrPath, bool IsName);
		FileNode *CacheLookup(const char *Path);
		FileNode *CreateCacheNode(FileNode *Parent, Inode *Node, const char *Name, mode_t Mode);

		int RemoveCacheNode(FileNode *Node);

	public:
		vfsInode *FileSystemRoots = nullptr;
		std::unordered_map<ino_t, FileNode *> FileRoots;

		bool PathIsRelative(const char *Path);
		bool PathIsAbsolute(const char *Path) { return !PathIsRelative(Path); }

		/**
		 * Reserve a device number for a filesystem
		 *
		 * @note After this function is called, the filesystem must
		 *  call LateRegisterFileSystem to release the lock
		 */
		dev_t EarlyReserveDevice();

		/**
		 * Register a filesystem after the device number has been reserved
		 */
		int LateRegisterFileSystem(dev_t Device, FileSystemInfo *fsi, Inode *Root);

		dev_t RegisterFileSystem(FileSystemInfo *fsi, Inode *Root);
		int UnregisterFileSystem(dev_t Device);

		void AddRoot(Inode *Root);
		void AddRootAt(Inode *Root, size_t Index);
		bool SetRootAt(Inode *Root, size_t Index);
		void RemoveRoot(Inode *Root);
		FileNode *GetRoot(size_t Index);
		bool RootExists(size_t Index);

		FileNode *Create(FileNode *Parent, const char *Name, mode_t Mode);
		FileNode *ForceCreate(FileNode *Parent, const char *Name, mode_t Mode);

		FileNode *Mount(FileNode *Parent, Inode *Node, const char *Path);
		int Unmount(const char *Path);

		FileNode *GetByPath(const char *Path, FileNode *Parent);
		std::string GetByNode(FileNode *Node);
		FileNode *CreateLink(const char *Path, FileNode *Parent, const char *Target);
		FileNode *CreateLink(const char *Path, FileNode *Parent, FileNode *Target);
		bool PathExists(const char *Path, FileNode *Parent);

		int Remove(FileNode *Node);

		void Initialize();
		Virtual();
		~Virtual();
	};

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
			FileNode *Node = nullptr;
			int References = 0;
			off_t Offset = 0;

			int operator==(const Fildes &other)
			{
				return Type == other.Type &&
					   Mode == other.Mode &&
					   Flags == other.Flags &&
					   Node == other.Node &&
					   References == other.References &&
					   Offset == other.Offset;
			}
		};

	private:
		FileNode *fdDir = nullptr;
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
		int usr_stat(const char *pathname, struct kstat *statbuf);
		int usr_fstat(int fd, struct kstat *statbuf);
		int usr_lstat(const char *pathname, struct kstat *statbuf);
		int usr_dup(int oldfd);
		int usr_dup2(int oldfd, int newfd);
		int usr_ioctl(int fd, unsigned long request, void *argp);

		FileDescriptorTable(void *Owner);
		~FileDescriptorTable() = default;
	};
}

#endif // !__FENNIX_KERNEL_FILESYSTEM_H__
