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

#include <smart_ptr.hpp>
#include <lock.hpp>
#include <errno.h>
#include <vector>
#include <atomic>
#include <string>

#define FILENAME_LENGTH 256
#define PATH_MAX 256

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define S_IXOTH 0001
#define S_IWOTH 0002
#define S_IROTH 0004
#define S_IRWXO 0007
#define S_IXGRP 0010
#define S_IWGRP 0020
#define S_IRGRP 0040
#define S_IRWXG 0070
#define S_IXUSR 0100
#define S_IWUSR 0200
#define S_IRUSR 0400
#define S_IRWXU 0700

#define O_RDONLY 00
#define O_WRONLY 01
#define O_RDWR 02
#define O_CREAT 0100
#define O_EXCL 0200
#define O_TRUNC 01000
#define O_APPEND 02000
#define O_CLOEXEC 02000000

#define S_IFIFO 0010000
#define S_IFCHR 0020000
#define S_IFDIR 0040000
#define S_IFBLK 0060000
#define S_IFREG 0100000
#define S_IFLNK 0120000
#define S_IFSOCK 0140000

#define S_IFMT 0170000

#define S_ISDIR(mode) (((mode)&S_IFMT) == S_IFDIR)
#define S_ISCHR(mode) (((mode)&S_IFMT) == S_IFCHR)
#define S_ISBLK(mode) (((mode)&S_IFMT) == S_IFBLK)
#define S_ISREG(mode) (((mode)&S_IFMT) == S_IFREG)
#define S_ISFIFO(mode) (((mode)&S_IFMT) == S_IFIFO)
#define S_ISLNK(mode) (((mode)&S_IFMT) == S_IFLNK)
#define S_ISSOCK(mode) (((mode)&S_IFMT) == S_IFSOCK)

struct stat
{
	dev_t st_dev;
	ino_t st_ino;
	mode_t st_mode;
	nlink_t st_nlink;

	uid_t st_uid;
	gid_t st_gid;
	dev_t st_rdev;
	off_t st_size;
	time_t st_atime;
	time_t st_mtime;
	time_t st_ctime;
	blksize_t st_blksize;
	blkcnt_t st_blocks;
	mode_t st_attr;
};

struct stat64
{
	dev_t st_dev;
	ino64_t st_ino;
	mode_t st_mode;
	nlink_t st_nlink;
	uid_t st_uid;
	gid_t st_gid;
	dev_t st_rdev;
	off64_t st_size;
	time_t st_atime;
	time_t st_mtime;
	time_t st_ctime;
	blksize_t st_blksize;
	blkcnt64_t st_blocks;
	mode_t st_attr;
};

static inline int ConvertFileFlags(const char *Mode)
{
	int Flags = 0;

	if (strchr(Mode, '+'))
		Flags |= O_RDWR;
	else if (*Mode == 'r')
		Flags |= O_RDONLY;
	else
		Flags |= O_WRONLY;

	if (strchr(Mode, 'x'))
		Flags |= O_EXCL;

	if (strchr(Mode, 'e'))
		Flags |= O_CLOEXEC;

	if (*Mode != 'r')
		Flags |= O_CREAT;

	if (*Mode == 'w')
		Flags |= O_TRUNC;

	if (*Mode == 'a')
		Flags |= O_APPEND;

	return Flags;
}

namespace VirtualFileSystem
{
	struct Node;

	typedef std::size_t (*OperationRead)(Node *node, std::size_t Size, uint8_t *Buffer, off_t &RefOffset);
	typedef std::size_t (*OperationWrite)(Node *node, std::size_t Size, uint8_t *Buffer, off_t &RefOffset);
	typedef void (*OperationCreate)(Node *node, char *Name, uint16_t NameLength, off_t &RefOffset);
	typedef void (*OperationMkdir)(Node *node, char *Name, uint16_t NameLength, off_t &RefOffset);
	typedef std::size_t (*OperationSeek)(Node *node, std::size_t Offset, int Whence, off_t &RefOffset);

#define ReadFSFunction(name) \
	std::size_t name(VirtualFileSystem::Node *node, std::size_t Size, uint8_t *Buffer, off_t &RefOffset)
#define WriteFSFunction(name) \
	std::size_t name(VirtualFileSystem::Node *node, std::size_t Size, uint8_t *Buffer, off_t &RefOffset)
#define CreateFSFunction(name) \
	void name(VirtualFileSystem::Node *node, char *Name, uint16_t NameLength, off_t &RefOffset)
#define MkdirFSFunction(name) \
	void name(VirtualFileSystem::Node *node, char *Name, uint16_t NameLength, off_t &RefOffset)
#define SeekFSFunction(name) \
	off_t name(VirtualFileSystem::Node *node, off_t Offset, uint8_t Whence, off_t &RefOffset)

	enum NodeFlags
	{
		NODE_FLAG_ERROR = 0x0,
		FILE = S_IFREG,
		DIRECTORY = S_IFDIR,
		CHARDEVICE = S_IFCHR,
		BLOCKDEVICE = S_IFBLK,
		PIPE = S_IFIFO,
		SYMLINK = S_IFLNK,
		MOUNTPOINT = S_IFDIR
	};

	struct FileSystemOperations
	{
		/**
		 * Name of the filesystem operations
		 *
		 * @note Mandatory
		 */
		char Name[FILENAME_LENGTH];

		/**
		 * Pointer to the read function
		 *
		 * @note Mandatory
		 */
		OperationRead Read = nullptr;

		/**
		 * Pointer to the write function
		 *
		 * @note Mandatory
		 */
		OperationWrite Write = nullptr;

		/**
		 * Pointer to the create function
		 *
		 * @note Optional
		 */
		OperationCreate Create = nullptr;

		/**
		 * Pointer to the mkdir function
		 *
		 * @note Optional
		 */
		OperationMkdir MakeDirectory = nullptr;

		/**
		 * Pointer to the seek function
		 *
		 * @note Optional
		 */
		OperationSeek Seek = nullptr;
	};

#define RefNode VirtualFileSystem::ReferenceNode

	class ReferenceNode
	{
	private:
		std::atomic_int64_t Offset = 0;
		off_t FileSize = 0;
		NewLock(RefNodeLock);
		Node *node;
		RefNode *SymlinkTo;

	public:
		decltype(FileSize) &Length = FileSize;
		Node *GetNode() const { return node; }
		std::string AbsolutePath;

		size_t Read(uint8_t *Buffer, size_t Size);
		size_t Write(uint8_t *Buffer, size_t Size);
		off_t Seek(off_t Offset, int Whence);

		ReferenceNode(Node *node);
		~ReferenceNode();

		friend class Virtual;
		friend class FileDescriptorTable;
	};

	class Node
	{
	private:
		NewLock(NodeLock);

	public:
		class Virtual *FileSystem = nullptr;
		ino_t IndexNode = 0;
		const char *Name;
		const char *Symlink;
		Node *SymlinkTarget = nullptr;
		mode_t Mode = 0;
		NodeFlags Flags = NodeFlags::NODE_FLAG_ERROR;
		uid_t UserIdentifier = 0;
		gid_t GroupIdentifier = 0;
		uintptr_t Address = 0;
		off_t Length = 0;
		Node *Parent = nullptr;
		FileSystemOperations *Operator = nullptr;
		/* For root node:
		0 - root "/"
		1 - etc
		...
		*/
		std::vector<Node *> Children;

		/**
		 * References to this node (open files, etc..)
		 */
		std::vector<ReferenceNode *> References;

		/**
		 * Create a new reference to this node
		 * @return Pointer to the new reference
		 */
		ReferenceNode *CreateReference();

		/**
		 * Remove a reference to this node
		 * @param Reference Pointer to the reference to remove
		 */
		void RemoveReference(ReferenceNode *Reference);

		Node() {}
		~Node() {}
	};

	class Virtual
	{
	private:
		Node *FileSystemRoot = nullptr;
		NewLock(VirtualLock);

		/**
		 * @note This function is NOT thread safe
		 */
		Node *AddNewChild(const char *Name, Node *Parent);

		/**
		 * @note This function is NOT thread safe
		 */
		Node *GetChild(const char *Name, Node *Parent);

		/**
		 * @note This function is NOT thread safe
		 */
		int RemoveChild(const char *Name, Node *Parent);

		/**
		 * @note This function is NOT thread safe
		 */
		Node *GetParent(const char *Path, Node *Parent);

		/**
		 * @note This function is NOT thread safe
		 */
		Node *GetNodeFromPath_Unsafe(const char *Path, Node *Parent = nullptr);

	public:
		std::string GetPathFromNode(Node *node);
		Node *GetNodeFromPath(const char *Path, Node *Parent = nullptr);

		bool PathIsRelative(const char *Path);

		Node *GetRootNode() { return FileSystemRoot; }

		std::string NormalizePath(const char *Path, Node *Parent = nullptr);
		bool PathExists(const char *Path, Node *Parent = nullptr);
		Node *CreateRoot(const char *RootName, FileSystemOperations *Operator);
		Node *Create(const char *Path, NodeFlags Flag, Node *Parent = nullptr);

		int Delete(const char *Path, bool Recursive = false, Node *Parent = nullptr);
		int Delete(Node *Path, bool Recursive = false, Node *Parent = nullptr);

		Node *Mount(const char *Path, FileSystemOperations *Operator);
		int Unmount(Node *File);

		/**
		 * Open a file
		 * @param Path The path to the file, relative or absolute. The buffer shouldn't be modified while the function is running.
		 * @param Parent Pointer to the parent node, if nullptr, the root node will be used.
		 * @return A pointer to the VirtualFileSystem::ReferenceNode, or nullptr if the file doesn't exist.
		 */
		RefNode *Open(const char *Path, Node *Parent = nullptr);

		Virtual();
		~Virtual();
	};

	class FileDescriptorTable
	{
	public:
		struct FileDescriptor
		{
			RefNode *Handle{};
			mode_t Mode = 0;
			int Flags = 0;
			int Descriptor = -1;
		};

	private:
		std::vector<FileDescriptor> FileDescriptors;
		VirtualFileSystem::Node *fdDir = nullptr;

		FileDescriptor GetFileDescriptor(int FileDescriptor);
		int ProbeMode(mode_t Mode, int Flags);
		int AddFileDescriptor(const char *AbsolutePath, mode_t Mode, int Flags);
		int RemoveFileDescriptor(int FileDescriptor);
		int GetFreeFileDescriptor();

	public:
		std::string GetAbsolutePath(int FileDescriptor);
		std::vector<FileDescriptor> &GetFileDescriptors() { return FileDescriptors; }

		int _open(const char *pathname, int flags, mode_t mode);
		int _creat(const char *pathname, mode_t mode);
		ssize_t _read(int fd, void *buf, size_t count);
		ssize_t _write(int fd, const void *buf, size_t count);
		int _close(int fd);
		off_t _lseek(int fd, off_t offset, int whence);
		int _stat(const char *pathname, struct stat *statbuf);
		int _fstat(int fd, struct stat *statbuf);
		int _lstat(const char *pathname, struct stat *statbuf);

		FileDescriptorTable(void *Owner);
		~FileDescriptorTable();
	};
}

int fopen(const char *pathname, const char *mode);
int creat(const char *pathname, mode_t mode);
ssize_t fread(int fd, void *buf, size_t count);
ssize_t fwrite(int fd, const void *buf, size_t count);
int fclose(int fd);
off_t lseek(int fd, off_t offset, int whence);
int stat(const char *pathname, struct stat *statbuf);
int fstat(int fd, struct stat *statbuf);
int lstat(const char *pathname, struct stat *statbuf);

#endif // !__FENNIX_KERNEL_FILESYSTEM_H__
