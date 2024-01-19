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

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

/** Other users have execute permission. */
#define S_IXOTH 0001
/** Other users have write permission. */
#define S_IWOTH 0002
/** Other users have read permission. */
#define S_IROTH 0004
/** Other users have read, write, and execute permissions. */
#define S_IRWXO 0007
/** Group members have execute permission. */
#define S_IXGRP 0010
/** Group members have write permission. */
#define S_IWGRP 0020
/** Group members have read permission. */
#define S_IRGRP 0040
/** Group members have read, write, and execute permissions. */
#define S_IRWXG 0070
/** The file owner has execute permission. */
#define S_IXUSR 0100
/** The file owner has write permission. */
#define S_IWUSR 0200
/** The file owner has read permission. */
#define S_IRUSR 0400
/** The file owner has read, write,
 * and execute permissions. */
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

#define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#define S_ISCHR(mode) (((mode) & S_IFMT) == S_IFCHR)
#define S_ISBLK(mode) (((mode) & S_IFMT) == S_IFBLK)
#define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
#define S_ISFIFO(mode) (((mode) & S_IFMT) == S_IFIFO)
#define S_ISLNK(mode) (((mode) & S_IFMT) == S_IFLNK)
#define S_ISSOCK(mode) (((mode) & S_IFMT) == S_IFSOCK)

/**
 * @struct stat
 * @brief Structure holding information about a file, as returned by the stat function.
 *
 * The 'stat' structure provides information about a file, including its size, ownership, permissions,
 * and other attributes. It is used with the stat function to query file status.
 */
struct stat
{
	/** Device ID of the file. */
	dev_t st_dev;
	/** Inode number. */
	ino_t st_ino;
	/** File type and mode. */
	mode_t st_mode;
	/** Number of hard links. */
	nlink_t st_nlink;
	/** User ID of the file's owner. */
	uid_t st_uid;
	/** Group ID of the file's owner. */
	gid_t st_gid;
	/** Device ID for special files. */
	dev_t st_rdev;
	/** Size of the file in bytes. */
	off_t st_size;
	/** Time of last access. */
	time_t st_atime;
	/** Time of last modification. */
	time_t st_mtime;
	/** Time of last status change. */
	time_t st_ctime;
	/** Optimal I/O block size. */
	blksize_t st_blksize;
	/** Number of blocks allocated. */
	blkcnt_t st_blocks;
	/** Additional file attributes. */
	mode_t st_attr;
};

/**
 * @struct stat64
 * @brief Extended structure for large file support, holding information about a file.
 *
 * The 'stat64' structure is similar to 'struct stat' but is extended to support large files on 32-bit systems.
 * It is used with the stat64 function for large file support.
 */
struct stat64
{
	/** Device ID of the file. */
	dev_t st_dev;
	/** Inode number. */
	ino64_t st_ino;
	/** File type and mode. */
	mode_t st_mode;
	/** Number of hard links. */
	nlink_t st_nlink;
	/** User ID of the file's owner. */
	uid_t st_uid;
	/** Group ID of the file's owner. */
	gid_t st_gid;
	/** Device ID for special files. */
	dev_t st_rdev;
	/** Size of the file in bytes. */
	off64_t st_size;
	/** Time of last access. */
	time_t st_atime;
	/** Time of last modification. */
	time_t st_mtime;
	/** Time of last status change. */
	time_t st_ctime;
	/** Optimal I/O block size. */
	blksize_t st_blksize;
	/** Number of blocks allocated. */
	blkcnt64_t st_blocks;
	/** Additional file attributes. */
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

namespace vfs
{
	enum NodeType : mode_t
	{
		NODE_TYPE_NONE = 0x0,
		FILE = S_IFREG,
		DIRECTORY = S_IFDIR,
		CHARDEVICE = S_IFCHR,
		BLOCKDEVICE = S_IFBLK,
		PIPE = S_IFIFO,
		SYMLINK = S_IFLNK,
		MOUNTPOINT = S_IFDIR
	};

	class RefNode;

	/**
	 * Virtual filesystem node
	 *
	 * @note https://isocpp.org/wiki/faq/freestore-mgmt#delete-this
	 */
	class Node
	{
	private:
		NewLock(NodeLock);

	public:
		virtual int open(int Flags, mode_t Mode);
		virtual int close();
		virtual size_t read(uint8_t *Buffer, size_t Size, off_t Offset);
		virtual size_t write(uint8_t *Buffer, size_t Size, off_t Offset);
		virtual int ioctl(unsigned long Request, void *Argp);
		// virtual int stat(struct stat *Stat);
		// virtual int lstat(struct stat *Stat);
		// virtual int fstat(struct stat *Stat);
		// virtual int unlink();
		// virtual int mkdir(mode_t Mode);
		// virtual int rmdir();
		// virtual int rename(const char *NewName);
		// virtual int chmod(mode_t Mode);
		// virtual int chown(uid_t User, gid_t Group);
		// virtual int truncate(off_t Size);
		// virtual int symlink(const char *Target);
		// virtual int readlink(char *Buffer, size_t Size);
		// virtual int mount(Node *Target);
		// virtual int umount();

		typedef int (*open_t)(int, mode_t);
		typedef int (*close_t)();
		typedef size_t (*read_t)(uint8_t *, size_t, off_t);
		typedef size_t (*write_t)(uint8_t *, size_t, off_t);
		typedef int (*ioctl_t)(unsigned long, void *);

		open_t open_ptr = nullptr;
		close_t close_ptr = nullptr;
		read_t read_ptr = nullptr;
		write_t write_ptr = nullptr;
		ioctl_t ioctl_ptr = nullptr;

		class Virtual *vFS = nullptr;
		Node *Parent = nullptr;
		const char *Name;
		const char *FullPath;
		NodeType Type;
		ino_t IndexNode;

		const char *Symlink;
		Node *SymlinkTarget;

		mode_t Mode;
		uid_t UserIdentifier;
		gid_t GroupIdentifier;

		dev_t DeviceMajor;
		dev_t DeviceMinor;

		time_t AccessTime;
		time_t ModifyTime;
		time_t ChangeTime;

		off_t Size;
		std::vector<Node *> Children;

		std::vector<RefNode *> References;
		RefNode *CreateReference();
		void RemoveReference(RefNode *Reference);

		/**
		 * Create a new node
		 *
		 * @param Parent The parent node
		 * @param Name The name of the node
		 * @param Type The type of the node
		 * @param NoParent If true, the Parent will
		 * be used as a hint for the parent node, but it
		 * won't be set as the parent node.
		 * @param fs The virtual filesystem (only if
		 * NoParent is set)
		 * @param Err If not nullptr, the function will
		 * write the error code to the given address.
		 */
		Node(Node *Parent,
			 const char *Name,
			 NodeType Type,
			 bool NoParent = false,
			 Virtual *fs = nullptr,
			 int *Err = nullptr);

		virtual ~Node();
	};

	class RefNode
	{
	private:
		std::atomic_int64_t FileOffset = 0;
		off_t FileSize = 0;
		Node *n;
		RefNode *SymlinkTo;

	public:
		void *SpecialData;

		decltype(FileSize) &Size = FileSize;
		decltype(n) &node = n;

		size_t read(uint8_t *Buffer, size_t Size);
		size_t write(uint8_t *Buffer, size_t Size);
		off_t seek(off_t Offset, int Whence);
		int ioctl(unsigned long Request, void *Argp);

		RefNode(Node *node);
		~RefNode();

		friend class Virtual;
		friend class FileDescriptorTable;
	};

	class Virtual
	{
	private:
		Node *FileSystemRoot = nullptr;
		NewLock(VirtualLock);

		Node *GetParent(const char *Path, Node *Parent);
		/** @note This function is NOT thread safe */
		Node *GetNodeFromPath_Unsafe(const char *Path, Node *Parent = nullptr);

	public:
		Node *nRoot = nullptr;
		Node *GetNodeFromPath(const char *Path, Node *Parent = nullptr);

		bool PathIsRelative(const char *Path);

		Node *GetRootNode() { return FileSystemRoot; }

		const char *NormalizePath(const char *Path, Node *Parent = nullptr);
		bool PathExists(const char *Path, Node *Parent = nullptr);

		Node *Create(const char *Path, NodeType Type, Node *Parent = nullptr);
		Node *CreateLink(const char *Path, const char *Target, Node *Parent);

		int Delete(const char *Path, bool Recursive = false, Node *Parent = nullptr);
		int Delete(Node *Path, bool Recursive = false, Node *Parent = nullptr);

		/**
		 * Open a file
		 * @param Path The path to the file, relative or absolute. The buffer shouldn't be modified while the function is running.
		 * @param Parent Pointer to the parent node, if nullptr, the root node will be used.
		 * @return A pointer to the vfs::ReferenceNode, or nullptr if the file doesn't exist.
		 */
		RefNode *Open(const char *Path, Node *Parent = nullptr);

		Node *CreateIfNotExists(const char *Path, NodeType Type, Node *Parent);

		Virtual();
		~Virtual();

		friend class Node;
	};

	class FileDescriptorTable
	{
	public:
		struct Fildes
		{
			RefNode *Handle = nullptr;
			mode_t Mode = 0;
			int Flags = 0;
			int Descriptor = -1;

			int operator==(const Fildes &other)
			{
				return this->Handle == other.Handle &&
					   this->Mode == other.Mode &&
					   this->Flags == other.Flags &&
					   this->Descriptor == other.Descriptor;
			}

			int operator!=(const Fildes &other)
			{
				return !(*this == other);
			}
		} __attribute__((packed)) nullfd;

	private:
		std::vector<Fildes> FileDescriptors;
		std::vector<Fildes> FildesDuplicates;
		vfs::Node *fdDir = nullptr;

		Fildes &GetFileDescriptor(int FileDescriptor);
		FileDescriptorTable::Fildes &GetDupFildes(int FileDescriptor);

		int ProbeMode(mode_t Mode, int Flags);
		int AddFileDescriptor(const char *AbsolutePath, mode_t Mode, int Flags);
		int RemoveFileDescriptor(int FileDescriptor);
		int GetFreeFileDescriptor();

	public:
		Fildes &GetDescriptor(int FileDescriptor);
		const char *GetAbsolutePath(int FileDescriptor);
		std::vector<Fildes> &GetFileDescriptors() { return FileDescriptors; }
		std::vector<Fildes> &GetFileDescriptorsDuplicates() { return FildesDuplicates; }
		RefNode *GetRefNode(int FileDescriptor);
		int GetFlags(int FileDescriptor);
		int SetFlags(int FileDescriptor, int Flags);
		void Fork(FileDescriptorTable *Parent);

		int _open(const char *pathname, int flags, mode_t mode);
		int _creat(const char *pathname, mode_t mode);
		ssize_t _read(int fd, void *buf, size_t count);
		ssize_t _write(int fd, const void *buf, size_t count);
		int _close(int fd);
		off_t _lseek(int fd, off_t offset, int whence);
		int _stat(const char *pathname, struct stat *statbuf);
		int _fstat(int fd, struct stat *statbuf);
		int _lstat(const char *pathname, struct stat *statbuf);
		int _dup(int oldfd);
		int _dup2(int oldfd, int newfd);
		int _ioctl(int fd, unsigned long request, void *argp);

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
