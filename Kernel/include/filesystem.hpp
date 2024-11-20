#ifndef __FENNIX_KERNEL_FILESYSTEM_H__
#define __FENNIX_KERNEL_FILESYSTEM_H__

#include <types.h>

#include <vector.hpp>

// show debug messages
// #define DEBUG_FILESYSTEM 1

#ifdef DEBUG_FILESYSTEM
#define vfsdbg(m, ...) debug(m, ##__VA_ARGS__)
#else
#define vfsdbg(m, ...)
#endif

namespace FileSystem
{
#define FILENAME_LENGTH 256

    struct FileSystemNode;

    typedef uint64_t (*OperationMount)(const char *, unsigned long, const void *);
    typedef uint64_t (*OperationUmount)(int);
    typedef uint64_t (*OperationRead)(FileSystemNode *Node, uint64_t Offset, uint64_t Size, uint8_t *Buffer);
    typedef uint64_t (*OperationWrite)(FileSystemNode *Node, uint64_t Offset, uint64_t Size, uint8_t *Buffer);
    typedef void (*OperationOpen)(FileSystemNode *Node, uint8_t Mode, uint8_t Flags);
    typedef void (*OperationClose)(FileSystemNode *Node);
    typedef uint64_t (*OperationSync)(void);
    typedef void (*OperationCreate)(FileSystemNode *Node, char *Name, uint16_t NameLength);
    typedef void (*OperationMkdir)(FileSystemNode *Node, char *Name, uint16_t NameLength);

#define MountFSFunction(name) uint64_t name(const char *unknown0, unsigned long unknown1, const uint8_t *unknown2)
#define UMountFSFunction(name) uint64_t name(int unknown0)

#define ReadFSFunction(name) uint64_t name(FileSystem::FileSystemNode *Node, uint64_t Offset, uint64_t Size, uint8_t *Buffer)
#define WriteFSFunction(name) uint64_t name(FileSystem::FileSystemNode *Node, uint64_t Offset, uint64_t Size, uint8_t *Buffer)
#define OpenFSFunction(name) void name(FileSystem::FileSystemNode *Node, uint8_t Mode, uint8_t Flags)
#define CloseFSFunction(name) void name(FileSystem::FileSystemNode *Node)
#define SyncFSFunction(name) uint64_t name(void)
#define CreateFSFunction(name) void name(FileSystem::FileSystemNode *Node, char *Name, uint16_t NameLength)
#define MkdirFSFunction(name) void name(FileSystem::FileSystemNode *Node, char *Name, uint16_t NameLength)

    enum FileStatus
    {
        OK = 0,
        NOT_FOUND = 1,
        ACCESS_DENIED = 2,
        INVALID_NAME = 3,
        INVALID_PARAMETER = 4,
        INVALID_HANDLE = 5,
        INVALID_PATH = 6,
        INVALID_FILE = 7,
        INVALID_DEVICE = 8,
        NOT_EMPTY = 9,
        NOT_SUPPORTED = 10,
        INVALID_DRIVE = 11,
        VOLUME_IN_USE = 12,
        TIMEOUT = 13,
        NO_MORE_FILES = 14,
        END_OF_FILE = 15,
        FILE_EXISTS = 16,
        PIPE_BUSY = 17,
        PIPE_DISCONNECTED = 18,
        MORE_DATA = 19,
        NO_DATA = 20,
        PIPE_NOT_CONNECTED = 21,
        MORE_ENTRIES = 22,
        DIRECTORY_NOT_EMPTY = 23,
        NOT_A_DIRECTORY = 24,
        FILE_IS_A_DIRECTORY = 25,
        DIRECTORY_NOT_ROOT = 26,
        DIRECTORY_NOT_EMPTY_2 = 27,
        END_OF_MEDIA = 28,
        NO_MEDIA = 29,
        UNRECOGNIZED_MEDIA = 30,
        SECTOR_NOT_FOUND = 31
    };

    enum NodeFlags
    {
        FS_ERROR = 0x0,
        FS_FILE = 0x01,
        FS_DIRECTORY = 0x02,
        FS_CHARDEVICE = 0x03,
        FS_BLOCKDEVICE = 0x04,
        FS_PIPE = 0x05,
        FS_SYMLINK = 0x06,
        FS_MOUNTPOINT = 0x08
    };

    struct FileSystemOperations
    {
        char Name[FILENAME_LENGTH];
        OperationMount Mount = nullptr;
        OperationUmount Umount = nullptr;
        OperationRead Read = nullptr;
        OperationWrite Write = nullptr;
        OperationOpen Open = nullptr;
        OperationClose Close = nullptr;
        OperationCreate Create = nullptr;
        OperationMkdir MakeDirectory = nullptr;
    };

    struct FileSystemNode
    {
        char Name[FILENAME_LENGTH];
        uint64_t IndexNode = 0;
        uint64_t Mask = 0;
        uint64_t Mode = 0;
        uint64_t Flags = NodeFlags::FS_ERROR;
        uint64_t UserIdentifier = 0, GroupIdentifier = 0;
        uint64_t Address = 0;
        uint64_t Length = 0;
        FileSystemNode *Parent = nullptr;
        FileSystemOperations *Operator = nullptr;
        /* For root node:
        0 - root "/"
        1 - etc
        ...
        */
        Vector<FileSystemNode *> Children;
    };

    struct FILE
    {
        const char *Name;
        FileStatus Status;
        FileSystemNode *Node;
    };

    /* Manage / etc.. */
    class Virtual
    {
    private:
        FileSystemNode *FileSystemRoot = nullptr;

    public:
        FileSystemNode *GetRootNode() { return FileSystemRoot; }
        FILE *ConvertNodeToFILE(FileSystemNode *Node)
        {
            FILE *File = new FILE;
            File->Name = Node->Name;
            File->Status = FileStatus::OK;
            File->Node = Node;
            return File;
        }
        char *GetPathFromNode(FileSystemNode *Node);
        FileSystemNode *GetNodeFromPath(FileSystemNode *Parent, const char *Path);
        char *NormalizePath(FileSystemNode *Parent, const char *Path);

        FileStatus FileExists(FileSystemNode *Parent, const char *Path);
        FILE *Mount(FileSystemOperations *Operator, const char *Path);
        FileStatus Unmount(FILE *File);
        FILE *Open(const char *Path, FileSystemNode *Parent = nullptr);
        uint64_t Read(FILE *File, uint64_t Offset, uint8_t *Buffer, uint64_t Size);
        uint64_t Write(FILE *File, uint64_t Offset, uint8_t *Buffer, uint64_t Size);
        FileStatus Close(FILE *File);
        FileSystemNode *CreateRoot(FileSystemOperations *Operator, const char *RootName);
        FileSystemNode *Create(FileSystemNode *Parent, const char *Path);

        Virtual();
        ~Virtual();
    };
}

#endif // !__FENNIX_KERNEL_FILESYSTEM_H__
