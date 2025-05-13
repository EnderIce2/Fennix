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

#ifndef __FENNIX_API_FILESYSTEM_H__
#define __FENNIX_API_FILESYSTEM_H__

#include <types.h>

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

/**
 * File type mask for the upper 32 bits of mode_t.
 *
 * @note Maybe it will be used in the future.
 */
#define S_IFMT32 037777600000

/**
 * File type mask.
 *
 * This mask is used to extract the file type
 * from the mode field of a stat structure.
 *
 * Doing bitwise AND with this mask will return
 * the file type.
 * Example: st_mode & S_IFMT
 *
 * Doing bitwise negation and AND with this mask
 * will return the permissions.
 * Example: st_mode & ~S_IFMT
 */
#define S_IFMT 0170000

/* Whiteout */
#define S_IFWHT 0160000
/* Socket */
#define S_IFSOCK 0140000
/* Symbolic link */
#define S_IFLNK 0120000
/* Regular file */
#define S_IFREG 0100000
/* Block device */
#define S_IFBLK 0060000
/* Directory */
#define S_IFDIR 0040000
/* Character device */
#define S_IFCHR 0020000
/* FIFO */
#define S_IFIFO 0010000

#define S_ISUID 04000
#define S_ISGID 02000
#define S_ISVTX 01000

/** Owner: RWX */
#define S_IRWXU 0700
/** Owner: R   */
#define S_IRUSR 0400
/** Owner:  W  */
#define S_IWUSR 0200
/** Owner:   X */
#define S_IXUSR 0100

/** Group: RWX */
#define S_IRWXG 0070
/** Group: R   */
#define S_IRGRP 0040
/** Group:  W  */
#define S_IWGRP 0020
/** Group:   X */
#define S_IXGRP 0010

/** Other: RWX */
#define S_IRWXO 0007
/** Other: R   */
#define S_IROTH 0004
/** Other:  W  */
#define S_IWOTH 0002
/** Other:   X */
#define S_IXOTH 0001

#define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#define S_ISCHR(mode) (((mode) & S_IFMT) == S_IFCHR)
#define S_ISBLK(mode) (((mode) & S_IFMT) == S_IFBLK)
#define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
#define S_ISFIFO(mode) (((mode) & S_IFMT) == S_IFIFO)
#define S_ISLNK(mode) (((mode) & S_IFMT) == S_IFLNK)
#define S_ISSOCK(mode) (((mode) & S_IFMT) == S_IFSOCK)

#define DT_UNKNOWN 0x0
#define DT_FIFO 0x1
#define DT_CHR 0x2
#define DT_DIR 0x4
#define DT_BLK 0x6
#define DT_REG 0x8
#define DT_LNK 0xA
#define DT_SOCK 0xC
#define DT_WHT 0xE

#define IFTODT(x) ((x) >> 12 & 0xF)
#define DTTOIF(x) ((x) << 12)

#define SYMLOOP_MAX 40

#ifndef __cplusplus
#define static_assert _Static_assert
#endif

#ifdef __LP64__
static_assert(sizeof(dev_t) == 8, "dev_t must be 64 bits");
static_assert(sizeof(ino_t) == 8, "ino_t must be 64 bits");
static_assert(sizeof(mode_t) == 4, "mode_t must be 32 bits");
static_assert(sizeof(nlink_t) == 4, "nlink_t must be 32 bits");
static_assert(sizeof(uid_t) == 4, "uid_t must be 32 bits");
static_assert(sizeof(gid_t) == 4, "gid_t must be 32 bits");
static_assert(sizeof(off_t) == 8, "off_t must be 64 bits");
static_assert(sizeof(time_t) == 8, "time_t must be 64 bits");
static_assert(sizeof(blksize_t) == 8, "blksize_t must be 64 bits");
static_assert(sizeof(blkcnt_t) == 8, "blkcnt_t must be 64 bits");
#else
static_assert(sizeof(dev_t) == 4, "dev_t must be 32 bits");
static_assert(sizeof(ino_t) == 4, "ino_t must be 32 bits");
static_assert(sizeof(mode_t) == 4, "mode_t must be 32 bits");
static_assert(sizeof(nlink_t) == 4, "nlink_t must be 32 bits");
static_assert(sizeof(uid_t) == 4, "uid_t must be 32 bits");
static_assert(sizeof(gid_t) == 4, "gid_t must be 32 bits");
static_assert(sizeof(off_t) == 4, "off_t must be 32 bits");
static_assert(sizeof(time_t) == 4, "time_t must be 32 bits");
static_assert(sizeof(blksize_t) == 4, "blksize_t must be 32 bits");
static_assert(sizeof(blkcnt_t) == 4, "blkcnt_t must be 32 bits");
#endif

#undef static_assert

struct kstat
{
	/** Device ID of the file. */
	dev_t Device;

	/** Inode number. */
	ino_t Index;

	/** File type and mode. */
	mode_t Mode;

	/** Number of hard links. */
	nlink_t HardLinks;

	/** User ID of the file's owner. */
	uid_t UserID;

	/** Group ID of the file's owner. */
	gid_t GroupID;

	/** Device ID for special files. */
	dev_t RawDevice;

	/** Size of the file in bytes. */
	off_t Size;

	/** Time of last access. */
	time_t AccessTime;

	/** Time of last modification. */
	time_t ModifyTime;

	/** Time of last status change. */
	time_t ChangeTime;

	/** Optimal I/O block size. */
	blksize_t BlockSize;

	/** Number of blocks allocated. */
	blkcnt_t Blocks;

	/** Additional file attributes. */
	mode_t Attribute;

#ifdef __cplusplus

	dev_t MakeDevice(int Major, int Minor)
	{
		return ((Major & 0xFFF) << 8) |
			   (Minor & 0xFF);
	}

	int GetMajor()
	{
		return ((unsigned int)(Device) >> 8) & 0xFFF;
	}

	int GetMinor()
	{
		return Device & 0xFF;
	}

	void SetFileType(mode_t Type)
	{
		Mode = (Mode & ~S_IFMT) |
			   (Type & S_IFMT);
	}

	mode_t GetFileType() { return Mode & S_IFMT; }
	void ClearFileType() { Mode = Mode & ~S_IFMT; }
	bool IsType(mode_t Type) { return (Mode & S_IFMT) == Type; }

	void SetPermissions(mode_t Permissions)
	{
		Mode = (Mode & S_IFMT) |
			   (Permissions & ~S_IFMT);
	}

	mode_t GetPermissions() { return Mode & ~S_IFMT; }
	void ClearPermissions() { Mode = Mode & S_IFMT; }

#endif // __cplusplus
};

struct kdirent
{
	ino_t d_ino;
	off_t d_off;
	unsigned short d_reclen;
	unsigned char d_type;
	char d_name[];
};

struct Inode
{
	dev_t Device, RawDevice;
	ino_t Index;
	mode_t Mode;
	uint32_t Flags;
	off_t Offset;

	uintptr_t KernelData;
	void *PrivateData;

#ifdef __cplusplus

	/* ... */

	void SetDevice(int Major, int Minor)
	{
		this->RawDevice = ((Major & 0xFFF) << 8) |
						  (Minor & 0xFF);
	}

	int GetMajor()
	{
		return ((unsigned int)(this->RawDevice) >> 8) & 0xFFF;
	}

	int GetMinor()
	{
		return this->RawDevice & 0xFF;
	}

	Inode()
	{
		Device = 0;
		RawDevice = 0;
		Index = 0;
		Mode = 0;
		Flags = 0;
		Offset = 0;
		KernelData = 0x0;
		PrivateData = nullptr;
	}

	~Inode() = default;

#else // __cplusplus

#define INODE_MAKEDEV(major, minor)   \
	((dev_t)(((major & 0xFFF) << 8) | \
			 (minor & 0xFF)))

#define INODE_MAJOR(rdev) \
	((int)(((rdev) >> 8) & 0xFFF))

#define INODE_MINOR(rdev) \
	((int)((rdev) & 0xFF))

#endif // __cplusplus
};

struct InodeOperations
{
	int (*Lookup)(struct Inode *Parent, const char *Name, struct Inode **Result);
	int (*Create)(struct Inode *Parent, const char *Name, mode_t Mode, struct Inode **Result);
	int (*Remove)(struct Inode *Parent, const char *Name);
	int (*Rename)(struct Inode *Parent, const char *OldName, const char *NewName);
	ssize_t (*Read)(struct Inode *Node, void *Buffer, size_t Size, off_t Offset);
	ssize_t (*Write)(struct Inode *Node, const void *Buffer, size_t Size, off_t Offset);
	int (*Truncate)(struct Inode *Node, off_t Size);
	int (*Open)(struct Inode *Node, int Flags, mode_t Mode);
	int (*Close)(struct Inode *Node);
	int (*Ioctl)(struct Inode *Node, unsigned long Request, void *Argp);
	ssize_t (*ReadDir)(struct Inode *Node, struct kdirent *Buffer, size_t Size, off_t Offset, off_t Entries);
	int (*MkDir)(struct Inode *Parent, const char *Name, mode_t Mode, struct Inode **Result);
	int (*RmDir)(struct Inode *Parent, const char *Name);
	int (*SymLink)(struct Inode *Parent, const char *Name, const char *Target, struct Inode **Result);
	ssize_t (*ReadLink)(struct Inode *Node, char *Buffer, size_t Size);
	off_t (*Seek)(struct Inode *Node, off_t Offset);
	int (*Stat)(struct Inode *Node, struct kstat *Stat);
} __attribute__((packed));

struct FileSystemInfo;
struct SuperBlockOperations
{
	int (*AllocateInode)(struct FileSystemInfo *Info, struct Inode **Result);
	int (*DeleteInode)(struct FileSystemInfo *Info, struct Inode *Node);

	/**
	 * Synchronize the filesystem.
	 *
	 * Write all pending changes to the disk.
	 *
	 * @param Info Inode to synchronize.
	 * @param Node Inode to synchronize. If NULL, synchronize all inodes.
	 *
	 * @return Zero on success, otherwise an error code.
	 */
	int (*Synchronize)(struct FileSystemInfo *Info, struct Inode *Node);

	/**
	 * Destroy the filesystem.
	 *
	 * Unregister the filesystem and free all resources.
	 *
	 * @param Info Filesystem to destroy.
	 *
	 * @return Zero on success, otherwise an error code.
	 */
	int (*Destroy)(struct FileSystemInfo *Info);

	/**
	 * Probe the filesystem.
	 *
	 * Check if the filesystem is supported by the driver.
	 *
	 * @param Device Device to probe.
	 *
	 * @return Zero on success, otherwise an error code.
	 */
	int (*Probe)(void *Device);

	/**
	 * Mount the filesystem.
	 *
	 * Mount the filesystem on the given device.
	 *
	 * @param FS Filesystem to mount.
	 * @param Root Pointer to the root inode.
	 * @param Device Device to mount.
	 *
	 * @return Zero on success, otherwise an error code.
	 */
	int (*Mount)(struct FileSystemInfo *FS, struct Inode **Root, void *Device);

	/**
	 * Unmount the filesystem.
	 *
	 * Unmount the filesystem from the given device.
	 *
	 * @param FS Filesystem to unmount.
	 *
	 * @return Zero on success, otherwise an error code.
	 */
	int (*Unmount)(struct FileSystemInfo *FS);
} __attribute__((packed));

struct FileSystemInfo
{
	const char *Name;

	int Flags;
	int Capabilities;

	struct SuperBlockOperations SuperOps;
	struct InodeOperations Ops;

	void *PrivateData;
} __attribute__((packed));

#ifndef __kernel__
dev_t RegisterMountPoint(FileSystemInfo *fsi, Inode *Root);
int UnregisterMountPoint(dev_t Device);

dev_t RegisterFileSystem(struct FileSystemInfo *Info, struct Inode *Root);
int UnregisterFileSystem(dev_t Device);
#endif // !__kernel__

#endif // !__FENNIX_API_FILESYSTEM_H__
