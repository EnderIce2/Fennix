#include <filesystem.hpp>

#include <smartptr.hpp>
#include <convert.h>
#include <printf.h>
#include <lock.hpp>
#include <cwalk.h>
#include <sys.h>

#include "../kernel.h"

// show debug messages
// #define DEBUG_FILESYSTEM 1

#ifdef DEBUG_FILESYSTEM
#define vfsdbg(m, ...) debug(m, ##__VA_ARGS__)
#else
#define vfsdbg(m, ...)
#endif

NewLock(VFSLock);

namespace VirtualFileSystem
{
    shared_ptr<char> Virtual::GetPathFromNode(Node *node)
    {
        vfsdbg("GetPathFromNode( Node: \"%s\" )", node->Name);
        Node *Parent = node;
        char **Path = nullptr;
        size_t Size = 1;
        size_t PathSize = 0;

        // Traverse the filesystem tree and build the path by adding each parent's Name field to the Path array
        while (Parent != FileSystemRoot && Parent != nullptr)
        {
            bool Found = false;
            foreach (const auto &Children in FileSystemRoot->Children)
                if (Children == Parent)
                {
                    Found = true;
                    break;
                }

            if (Found)
                break;

            if (strlen(Parent->Name) == 0)
                break;

            char **new_path = new char *[PathSize + 1];
            if (Path != nullptr)
            {
                memcpy(new_path, Path, sizeof(char *) * PathSize);
                delete[] Path;
            }

            Path = new_path;
            Path[PathSize] = Parent->Name;
            PathSize++;
            new_path = new char *[PathSize + 1];
            memcpy(new_path, Path, sizeof(char *) * PathSize);
            delete[] Path;
            Path = new_path;
            Path[PathSize] = (char *)"/";
            PathSize++;
            Parent = Parent->Parent;
        }

        // Calculate the total size of the final path string
        for (size_t i = 0; i < PathSize; i++)
        {
            Size += strlen(Path[i]);
        }

        // Allocate a new string for the final path
        shared_ptr<char> FinalPath;
        FinalPath.reset(new char[Size]);

        size_t Offset = 0;

        // Concatenate the elements of the Path array into the FinalPath string
        for (size_t i = PathSize - 1; i < PathSize; i--)
        {
            if (Path[i] == nullptr)
            {
                continue;
            }
            size_t ElementSize = strlen(Path[i]);
            memcpy(FinalPath.Get() + Offset, Path[i], ElementSize);
            Offset += ElementSize;
        }

        // Add a null terminator to the final path string
        FinalPath.Get()[Size - 1] = '\0';

        // Deallocate the Path array
        delete[] Path;

        vfsdbg("GetPathFromNode()->\"%s\"", FinalPath.Get());
        return FinalPath;
    }

    Node *Virtual::GetNodeFromPath(const char *Path, Node *Parent)
    {
        vfsdbg("GetNodeFromPath( Path: \"%s\" Parent: \"%s\" )", Path, Parent ? Parent->Name : "(null)");

        Node *ReturnNode = Parent;
        bool IsAbsolutePath = cwk_path_is_absolute(Path);

        if (!ReturnNode)
            ReturnNode = FileSystemRoot->Children[0]; // 0 - filesystem root

        if (IsAbsolutePath)
            ReturnNode = FileSystemRoot->Children[0]; // 0 - filesystem root

        cwk_segment segment;
        if (unlikely(!cwk_path_get_first_segment(Path, &segment)))
        {
            error("Path doesn't have any segments.");
            return nullptr;
        }

        do
        {
            char *SegmentName = new char[segment.end - segment.begin + 1];
            memcpy(SegmentName, segment.begin, segment.end - segment.begin);
            vfsdbg("GetNodeFromPath()->SegmentName: \"%s\"", SegmentName);
        GetNodeFromPathNextParent:
            foreach (auto Child in ReturnNode->Children)
            {
                vfsdbg("comparing \"%s\" with \"%s\"", Child->Name, SegmentName);
                if (strcmp(Child->Name, SegmentName) == 0)
                {
                    ReturnNode = Child;
                    goto GetNodeFromPathNextParent;
                }
            }
            delete[] SegmentName;
        } while (cwk_path_get_next_segment(&segment));

        const char *basename;
        cwk_path_get_basename(Path, &basename, nullptr);
        vfsdbg("BaseName: \"%s\" NodeName: \"%s\"", basename, ReturnNode->Name);

        if (strcmp(basename, ReturnNode->Name) == 0)
        {
            vfsdbg("GetNodeFromPath()->\"%s\"", ReturnNode->Name);
            return ReturnNode;
        }

        vfsdbg("GetNodeFromPath()->\"(null)\"");
        return nullptr;
    }

    shared_ptr<File> Virtual::ConvertNodeToFILE(Node *node)
    {
        shared_ptr<File> file = make_shared<File>();
        file->Status = FileStatus::OK;
        file->node = node;
        return file;
    }

    Node *Virtual::GetParent(const char *Path, Node *Parent)
    {
        vfsdbg("GetParent( Path: \"%s\" Parent: \"%s\" )", Path, Parent->Name);
        if (Parent)
        {
            vfsdbg("GetParent()->\"%s\"", Parent->Name);
            return Parent;
        }

        Node *ParentNode = nullptr;
        if (FileSystemRoot->Children.size() >= 1)
        {
            if (FileSystemRoot->Children[0] == nullptr)
                panic("Root node is null!");

            ParentNode = FileSystemRoot->Children[0]; // 0 - filesystem root
        }
        else
        {
            // TODO: Check if here is a bug or something...
            const char *PathCopy;
            PathCopy = (char *)Path;
            size_t length;
            cwk_path_get_root(PathCopy, &length); // not working?
            if (length > 0)
            {
                foreach (auto Child in FileSystemRoot->Children)
                {
                    if (strcmp(Child->Name, PathCopy) == 0)
                    {
                        ParentNode = Child;
                        break;
                    }
                }
            }
        }
        vfsdbg("GetParent()->\"%s\"", ParentNode->Name);
        return ParentNode;
    }

    Node *Virtual::AddNewChild(const char *Name, Node *Parent)
    {
        if (!Parent)
        {
            error("Parent is null!");
            return nullptr;
        }
        vfsdbg("AddNewChild( Name: \"%s\" Parent: \"%s\" )", Name, Parent->Name);

        Node *newNode = new Node;
        newNode->Parent = Parent;
        strcpy(newNode->Name, Name);

        newNode->Operator = Parent->Operator;
        Parent->Children.push_back(newNode);

        vfsdbg("AddNewChild()->\"%s\"", newNode->Name);
        return newNode;
    }

    Node *Virtual::GetChild(const char *Name, Node *Parent)
    {
        vfsdbg("GetChild( Name: \"%s\" Parent: \"%s\" )", Name, Parent->Name);
        if (!Parent)
        {
            vfsdbg("GetChild()->nullptr");
            return nullptr;
        }

        foreach (auto Child in Parent->Children)
            if (strcmp(Child->Name, Name) == 0)
            {
                vfsdbg("GetChild()->\"%s\"", Child->Name);
                return Child;
            }
        vfsdbg("GetChild()->nullptr (not found)");
        return nullptr;
    }

    FileStatus Virtual::RemoveChild(const char *Name, Node *Parent)
    {
        vfsdbg("RemoveChild( Name: \"%s\" Parent: \"%s\" )", Name, Parent->Name);
        for (size_t i = 0; i < Parent->Children.size(); i++)
        {
            if (strcmp(Parent->Children[i]->Name, Name) == 0)
            {
                delete Parent->Children[i];
                Parent->Children.remove(i);
                vfsdbg("RemoveChild()->OK");
                return FileStatus::OK;
            }
        }
        vfsdbg("RemoveChild()->NotFound");
        return FileStatus::NotFound;
    }

    shared_ptr<char> Virtual::NormalizePath(const char *Path, Node *Parent)
    {
        vfsdbg("NormalizePath( Path: \"%s\" Parent: \"%s\" )", Path, Parent->Name);
        char *NormalizedPath = new char[strlen((char *)Path) + 1];
        shared_ptr<char> RelativePath;

        cwk_path_normalize(Path, NormalizedPath, strlen((char *)Path) + 1);

        if (cwk_path_is_relative(NormalizedPath))
        {
            shared_ptr<char> ParentPath = GetPathFromNode(Parent);
            size_t PathSize = cwk_path_get_absolute(ParentPath.Get(), NormalizedPath, nullptr, 0);
            RelativePath.reset(new char[PathSize + 1]);
            cwk_path_get_absolute(ParentPath.Get(), NormalizedPath, RelativePath.Get(), PathSize + 1);
        }
        else
        {
            RelativePath.reset(new char[strlen(NormalizedPath) + 1]);
            strcpy(RelativePath.Get(), NormalizedPath);
        }
        delete[] NormalizedPath;
        vfsdbg("NormalizePath()->\"%s\"", RelativePath.Get());
        return RelativePath;
    }

    bool Virtual::PathExists(const char *Path, Node *Parent)
    {
        if (isempty((char *)Path))
        {
            vfsdbg("PathExists()->PathIsEmpty");
            return false;
        }

        if (Parent == nullptr)
            Parent = FileSystemRoot;

        vfsdbg("PathExists( Path: \"%s\" Parent: \"%s\" )", Path, Parent->Name);

        if (GetNodeFromPath(NormalizePath(Path, Parent).Get(), Parent))
        {
            vfsdbg("PathExists()->OK");
            return true;
        }

        vfsdbg("PathExists()->NotFound");
        return false;
    }

    Node *Virtual::CreateRoot(const char *RootName, FileSystemOperations *Operator)
    {
        if (Operator == nullptr)
            return nullptr;
        vfsdbg("Creating root %s", RootName);
        Node *newNode = new Node;
        strncpy(newNode->Name, RootName, FILENAME_LENGTH);
        newNode->Flags = NodeFlags::DIRECTORY;
        newNode->Operator = Operator;
        FileSystemRoot->Children.push_back(newNode);
        return newNode;
    }

    /* TODO: Further testing needed */
    Node *Virtual::Create(const char *Path, NodeFlags Flag, Node *Parent)
    {
        SmartLock(VFSLock);

        if (isempty((char *)Path))
            return nullptr;

        Node *RootNode = FileSystemRoot->Children[0];
        Node *CurrentParent = this->GetParent(Path, Parent);
        vfsdbg("Virtual::Create( Path: \"%s\" Parent: \"%s\" )", Path, Parent ? Parent->Name : CurrentParent->Name);

        shared_ptr<char> CleanPath = this->NormalizePath(Path, CurrentParent);
        vfsdbg("CleanPath: \"%s\"", CleanPath.Get());

        if (PathExists(CleanPath.Get(), CurrentParent))
        {
            error("Path %s already exists.", CleanPath.Get());
            goto CreatePathError;
        }

        cwk_segment segment;
        if (!cwk_path_get_first_segment(CleanPath.Get(), &segment))
        {
            error("Path doesn't have any segments.");
            goto CreatePathError;
        }

        do
        {
            char *SegmentName = new char[segment.end - segment.begin + 1];
            memcpy(SegmentName, segment.begin, segment.end - segment.begin);
            vfsdbg("SegmentName: \"%s\"", SegmentName);

            if (Parent)
                if (GetChild(SegmentName, RootNode) != nullptr)
                {
                    RootNode = GetChild(SegmentName, RootNode);
                    delete[] SegmentName;
                    continue;
                }

            if (GetChild(SegmentName, CurrentParent) == nullptr)
            {
                CurrentParent = AddNewChild(SegmentName, CurrentParent);
                CurrentParent->Flags = Flag;
            }
            else
            {
                CurrentParent = GetChild(SegmentName, CurrentParent);
            }

            delete[] SegmentName;
        } while (cwk_path_get_next_segment(&segment));

        vfsdbg("Virtual::Create()->\"%s\"", CurrentParent->Name);
        vfsdbg("Path created: \"%s\"", GetPathFromNode(CurrentParent).Get());
        return CurrentParent;

    CreatePathError:
        vfsdbg("Virtual::Create()->nullptr");
        return nullptr;
    }

    FileStatus Virtual::Delete(const char *Path, bool Recursive, Node *Parent)
    {
        SmartLock(VFSLock);
        vfsdbg("Virtual::Delete( Path: \"%s\" Parent: \"%s\" )", Path, Parent ? Parent->Name : "(null)");

        if (isempty((char *)Path))
            return InvalidParameter;

        if (Parent == nullptr)
            Parent = FileSystemRoot;

        shared_ptr<char> CleanPath = this->NormalizePath(Path, Parent);
        vfsdbg("CleanPath: \"%s\"", CleanPath.Get());

        if (!PathExists(CleanPath.Get(), Parent))
        {
            vfsdbg("Path %s doesn't exist.", CleanPath.Get());
            return InvalidPath;
        }

        Node *NodeToDelete = GetNodeFromPath(CleanPath.Get(), Parent);
        Node *ParentNode = GetParent(CleanPath.Get(), Parent);

        if (NodeToDelete->Flags == NodeFlags::DIRECTORY)
        {
            if (Recursive)
            {
                foreach (auto Child in NodeToDelete->Children)
                {
                    FileStatus Status = Delete(GetPathFromNode(Child).Get(), true);
                    if (Status != FileStatus::OK)
                    {
                        vfsdbg("Failed to delete child %s with status %d. (%s)", Child->Name, Status, Path);
                        return PartiallyCompleted;
                    }
                }
            }
            else if (NodeToDelete->Children.size() > 0)
            {
                vfsdbg("Directory %s is not empty.", CleanPath.Get());
                return DirectoryNotEmpty;
            }
        }

        if (RemoveChild(NodeToDelete->Name, ParentNode) != FileStatus::OK)
        {
            vfsdbg("Failed to remove child %s from parent %s. (%s)", NodeToDelete->Name, ParentNode->Name, Path);
            return NotFound;
        }

        vfsdbg("Virtual::Delete()->OK");
        return OK;
    }

    FileStatus Virtual::Delete(Node *Path, bool Recursive, Node *Parent) { return Delete(GetPathFromNode(Path).Get(), Recursive, Parent); }

    /* TODO: REWORK */
    shared_ptr<File> Virtual::Mount(const char *Path, FileSystemOperations *Operator)
    {
        SmartLock(VFSLock);
        shared_ptr<File> file = make_shared<File>();

        if (unlikely(!Operator))
        {
            file->Status = FileStatus::InvalidOperator;
            return file;
        }

        if (unlikely(isempty((char *)Path)))
        {
            file->Status = FileStatus::InvalidParameter;
            return file;
        }

        vfsdbg("Mounting %s", Path);
        const char *PathCopy;
        cwk_path_get_basename(Path, &PathCopy, 0);
        strcpy(file->Name, PathCopy);
        file->Status = FileStatus::OK;
        file->node = Create(Path, NodeFlags::MOUNTPOINT);
        file->node->Operator = Operator;
        return file;
    }

    FileStatus Virtual::Unmount(shared_ptr<File> File)
    {
        SmartLock(VFSLock);
        if (unlikely(File.Get()))
            return FileStatus::InvalidParameter;
        fixme("Unmounting %s", File->Name);
        return FileStatus::OK;
    }

    size_t Virtual::Read(shared_ptr<File> File, size_t Offset, uint8_t *Buffer, size_t Size)
    {
        SmartLock(VFSLock);
        if (unlikely(!File.Get()))
            return 0;

        if (unlikely(!File->node))
        {
            File->Status = FileStatus::InvalidNode;
            return 0;
        }

        if (unlikely(!File->node->Operator))
        {
            File->Status = FileStatus::InvalidOperator;
            return 0;
        }

        File->Status = FileStatus::OK;

        vfsdbg("Reading %s out->%016x", File->Name, Buffer);
        return File->node->Operator->Read(File->node, Offset, Size, Buffer);
    }

    size_t Virtual::Write(shared_ptr<File> File, size_t Offset, uint8_t *Buffer, size_t Size)
    {
        SmartLock(VFSLock);
        if (unlikely(!File.Get()))
            return 0;

        if (unlikely(!File->node))
        {
            File->Status = FileStatus::InvalidNode;
            return 0;
        }

        if (unlikely(!File->node->Operator))
        {
            File->Status = FileStatus::InvalidOperator;
            return 0;
        }

        File->Status = FileStatus::OK;

        vfsdbg("Writing %s out->%016x", File->Name, Buffer);
        return File->node->Operator->Write(File->node, Offset, Size, Buffer);
    }

    /* TODO: CHECK Open */
    shared_ptr<File> Virtual::Open(const char *Path, Node *Parent)
    {
        SmartLock(VFSLock);
        vfsdbg("Opening %s with parent %s", Path, Parent ? Parent->Name : "(null)");
        const char *basename;

        if (strcmp(Path, ".") == 0)
        {
            shared_ptr<File> file = make_shared<File>();
            file->node = Parent;
            if (unlikely(!file->node))
                file->Status = FileStatus::NotFound;
            cwk_path_get_basename(GetPathFromNode(Parent).Get(), &basename, nullptr);
            strcpy(file->Name, basename);
            return file;
        }

        if (strcmp(Path, "..") == 0)
        {
            shared_ptr<File> file = make_shared<File>();

            if (Parent->Parent != nullptr)
                file->node = Parent->Parent;

            if (!file->node)
                file->Status = FileStatus::NotFound;
            cwk_path_get_basename(GetPathFromNode(Parent).Get(), &basename, nullptr);
            strcpy(file->Name, basename);
            return file;
        }

        Node *CurrentParent = this->GetParent(Path, Parent);
        shared_ptr<char> CleanPath = NormalizePath(Path, CurrentParent);

        shared_ptr<File> file = make_shared<File>();
        /* TODO: Check for other errors */

        if (!PathExists(CleanPath.Get(), CurrentParent))
        {
            foreach (auto Child in FileSystemRoot->Children)
            {
                if (strcmp(Child->Name, CleanPath.Get()) == 0)
                {
                    file->node = Child;
                    if (file->node == nullptr)
                    {
                        file->Status = FileStatus::UnknownFileStatusError;
                        file->node = nullptr;
                        return file;
                    }
                    cwk_path_get_basename(GetPathFromNode(Child).Get(), &basename, nullptr);
                    strcpy(file->Name, basename);
                    return file;
                }
            }

            file->node = GetNodeFromPath(CleanPath.Get(), FileSystemRoot->Children[0]);
            if (file->node)
            {
                cwk_path_get_basename(GetPathFromNode(file->node).Get(), &basename, nullptr);
                strcpy(file->Name, basename);
                return file;
            }
        }
        else
        {
            file->node = GetNodeFromPath(CleanPath.Get(), CurrentParent);
            cwk_path_get_basename(CleanPath.Get(), &basename, nullptr);
            strcpy(file->Name, basename);
            return file;
        }

        file->Status = FileStatus::NotFound;
        return file;
    }

    FileStatus Virtual::Close(shared_ptr<File> File)
    {
        SmartLock(VFSLock);
        if (unlikely(!File.Get()))
            return FileStatus::InvalidHandle;
        vfsdbg("Closing %s", File->Name);
        return FileStatus::OK;
    }

    Virtual::Virtual()
    {
        trace("Initializing virtual file system...");
        FileSystemRoot = new Node;
        FileSystemRoot->Flags = NodeFlags::MOUNTPOINT;
        FileSystemRoot->Operator = nullptr;
        FileSystemRoot->Parent = nullptr;
        strncpy(FileSystemRoot->Name, "root", 4);
        cwk_path_set_style(CWK_STYLE_UNIX);
    }

    Virtual::~Virtual()
    {
        trace("Destroying virtual file system...");
    }
}
