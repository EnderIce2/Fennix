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
#include <vector>

namespace VirtualFileSystem
{
#define FILENAME_LENGTH 256

    struct Node;

    typedef size_t (*OperationMount)(const char *, unsigned long, const void *);
    typedef size_t (*OperationUmount)(int);
    typedef size_t (*OperationRead)(Node *node, size_t Offset, size_t Size, uint8_t *Buffer);
    typedef size_t (*OperationWrite)(Node *node, size_t Offset, size_t Size, uint8_t *Buffer);
    typedef void (*OperationOpen)(Node *node, uint8_t Mode, uint8_t Flags);
    typedef void (*OperationClose)(Node *node);
    typedef size_t (*OperationSync)(void);
    typedef void (*OperationCreate)(Node *node, char *Name, uint16_t NameLength);
    typedef void (*OperationMkdir)(Node *node, char *Name, uint16_t NameLength);

#define MountFSFunction(name) size_t name(const char *unknown0, unsigned long unknown1, const uint8_t *unknown2)
#define UMountFSFunction(name) size_t name(int unknown0)

#define ReadFSFunction(name) size_t name(VirtualFileSystem::Node *node, size_t Offset, size_t Size, uint8_t *Buffer)
#define WriteFSFunction(name) size_t name(VirtualFileSystem::Node *node, size_t Offset, size_t Size, uint8_t *Buffer)
#define OpenFSFunction(name) void name(VirtualFileSystem::Node *node, uint8_t Mode, uint8_t Flags)
#define CloseFSFunction(name) void name(VirtualFileSystem::Node *node)
#define SyncFSFunction(name) size_t name(void)
#define CreateFSFunction(name) void name(VirtualFileSystem::Node *node, char *Name, uint16_t NameLength)
#define MkdirFSFunction(name) void name(VirtualFileSystem::Node *node, char *Name, uint16_t NameLength)

    enum FileStatus
    {
        OK,
        NotFound,
        NotEmpty,
        NotSupported,
        AccessDenied,
        Timeout,
        SectorNotFound,
        PartiallyCompleted,

        InvalidName,
        InvalidParameter,
        InvalidHandle,
        InvalidPath,
        InvalidDevice,
        InvalidOperator,
        InvalidNode,

        FileExists,
        FileIsADirectory,
        FileIsInvalid,

        DirectoryNotEmpty,
        NotADirectory,

        UnknownFileStatusError
    };

    enum NodeFlags
    {
        NODE_FLAG_ERROR = 0x0,
        FILE = 0x01,
        DIRECTORY = 0x02,
        CHARDEVICE = 0x03,
        BLOCKDEVICE = 0x04,
        PIPE = 0x05,
        SYMLINK = 0x06,
        MOUNTPOINT = 0x08
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

    struct Node
    {
        char Name[FILENAME_LENGTH];
        uint64_t IndexNode = 0;
        uint64_t Mask = 0;
        uint64_t Mode = 0;
        NodeFlags Flags = NodeFlags::NODE_FLAG_ERROR;
        uint64_t UserIdentifier = 0, GroupIdentifier = 0;
        uintptr_t Address = 0;
        size_t Length = 0;
        Node *Parent = nullptr;
        FileSystemOperations *Operator = nullptr;
        /* For root node:
        0 - root "/"
        1 - etc
        ...
        */
        std::vector<Node *> Children;
    };

    struct File
    {
        char Name[FILENAME_LENGTH];
        FileStatus Status;
        Node *node;
    };

    /* Manage / etc.. */
    class Virtual
    {
    private:
        Node *FileSystemRoot = nullptr;

    public:
        std::shared_ptr<char> GetPathFromNode(Node *node);
        Node *GetNodeFromPath(const char *Path, Node *Parent = nullptr);
        std::shared_ptr<File> ConvertNodeToFILE(Node *node);

        Node *GetParent(const char *Path, Node *Parent);
        Node *GetRootNode() { return FileSystemRoot; }

        Node *AddNewChild(const char *Name, Node *Parent);
        Node *GetChild(const char *Name, Node *Parent);
        FileStatus RemoveChild(const char *Name, Node *Parent);

        std::shared_ptr<char> NormalizePath(const char *Path, Node *Parent = nullptr);
        bool PathExists(const char *Path, Node *Parent = nullptr);
        Node *CreateRoot(const char *RootName, FileSystemOperations *Operator);
        Node *Create(const char *Path, NodeFlags Flag, Node *Parent = nullptr);

        FileStatus Delete(const char *Path, bool Recursive = false, Node *Parent = nullptr);
        FileStatus Delete(Node *Path, bool Recursive = false, Node *Parent = nullptr);

        std::shared_ptr<File> Mount(const char *Path, FileSystemOperations *Operator);
        FileStatus Unmount(std::shared_ptr<File> File);

        size_t Read(std::shared_ptr<File> File, size_t Offset, uint8_t *Buffer, size_t Size);
        size_t Write(std::shared_ptr<File> File, size_t Offset, uint8_t *Buffer, size_t Size);

        std::shared_ptr<File> Open(const char *Path, Node *Parent = nullptr);
        FileStatus Close(std::shared_ptr<File> File);

        Virtual();
        ~Virtual();
    };
}

#endif // !__FENNIX_KERNEL_FILESYSTEM_H__
