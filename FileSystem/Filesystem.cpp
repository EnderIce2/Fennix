#include <filesystem.hpp>

#include <smartptr.hpp>
#include <convert.h>
#include <printf.h>
#include <lock.hpp>
#include <cwalk.h>
#include <sys.h>

#include "../kernel.h"

NewLock(VFSLock);

namespace FileSystem
{
    char *Virtual::GetPathFromNode(FileSystemNode *Node)
    {
        vfsdbg("GetPathFromNode( Node: \"%s\" )", Node->Name);
        FileSystemNode *Parent = Node;
        Vector<char *> Path;
        size_t Size = 1;

        while (Parent != FileSystemRoot && Parent != nullptr)
        {
            foreach (auto var in FileSystemRoot->Children)
                if (var == Parent)
                    goto PathFromNodeContinue;
            Path.push_back(Parent->Name);
            Path.push_back((char *)"/");
            Parent = Parent->Parent;
        }

    PathFromNodeContinue:
        Path.reverse();

        foreach (auto var in Path)
            Size += strlen(var);

        char *FinalPath = new char[Size];

        foreach (auto var in Path)
            strcat(FinalPath, var);

        // for (size_t i = 0; i < Path.size(); i++)
        //     strcat(FinalPath, Path[i]);

        // for (size_t i = 0; i < Path.size(); i++)
        //     Size += strlen(Path[i]);
        vfsdbg("GetPathFromNode()->\"%s\"", FinalPath);
        return FinalPath;
    }

    FileSystemNode *Virtual::GetNodeFromPath(FileSystemNode *Parent, const char *Path)
    {
        vfsdbg("GetNodeFromPath( Parent: \"%s\" Path: \"%s\" )", Parent->Name, Path);

        if (Parent == nullptr)
            Parent = FileSystemRoot;

        if (strcmp(Parent->Name, Path))
        {
            cwk_segment segment;
            if (!cwk_path_get_first_segment(Path, &segment))
            {
                error("Path doesn't have any segments.");
                return nullptr;
            }

            do
            {
                char *SegmentName = new char[segment.end - segment.begin + 1];
                memcpy(SegmentName, segment.begin, segment.end - segment.begin);
            GetNodeFromPathNextParent:
                foreach (auto var in Parent->Children)
                {
                    if (!strcmp(var->Name, SegmentName))
                    {
                        Parent = var;
                        goto GetNodeFromPathNextParent;
                    }
                }
                delete[] SegmentName;
            } while (cwk_path_get_next_segment(&segment));
            const char *basename;
            cwk_path_get_basename(Path, &basename, nullptr);
            if (!strcmp(basename, Parent->Name))
            {
                vfsdbg("GetNodeFromPath()->\"%s\"", Parent->Name);
                return Parent;
            }

            vfsdbg("GetNodeFromPath()->\"%s\"", nullptr);
            return nullptr;
        }
        else
        {
            vfsdbg("GetNodeFromPath()->\"%s\"", Parent->Name);
            return Parent;
        }
    }

    FileSystemNode *AddNewChild(FileSystemNode *Parent, const char *Name)
    {
        vfsdbg("AddNewChild( Parent: \"%s\" Name: \"%s\" )", Parent->Name, Name);
        FileSystemNode *newNode = new FileSystemNode;
        newNode->Parent = Parent;
        strcpy(newNode->Name, Name);
        if (Parent)
            newNode->Operator = Parent->Operator;
        else
            newNode->Operator = nullptr;

        if (Parent)
            Parent->Children.push_back(newNode);
        vfsdbg("AddNewChild()->\"%s\"", newNode->Name);
        return newNode;
    }

    FileSystemNode *GetChild(FileSystemNode *Parent, const char *Name)
    {
        vfsdbg("GetChild( Parent: \"%s\" Name: \"%s\" )", Parent->Name, Name);
        if (Parent)
            foreach (auto var in Parent->Children)
                if (strcmp(var->Name, Name) == 0)
                {
                    vfsdbg("GetChild()->\"%s\"", var->Name);
                    return var;
                }
        vfsdbg("GetChild()->nullptr");
        return nullptr;
    }

    FileStatus RemoveChild(FileSystemNode *Parent, const char *Name)
    {
        vfsdbg("RemoveChild( Parent: \"%s\" Name: \"%s\" )", Parent->Name, Name);
        for (uint64_t i = 0; i < Parent->Children.size(); i++)
            if (strcmp(Parent->Children[i]->Name, Name) == 0)
            {
                Parent->Children.remove(i);
                vfsdbg("RemoveChild()->OK");
                return FileStatus::OK;
            }
        vfsdbg("RemoveChild()->NOT_FOUND");
        return FileStatus::NOT_FOUND;
    }

    char *Virtual::NormalizePath(FileSystemNode *Parent, const char *Path)
    {
        vfsdbg("NormalizePath( Parent: \"%s\" Path: \"%s\" )", Parent->Name, Path);
        char *NormalizedPath = new char[strlen((char *)Path) + 1];
        char *RelativePath = nullptr;

        cwk_path_normalize(Path, NormalizedPath, strlen((char *)Path) + 1);

        if (cwk_path_is_relative(NormalizedPath))
        {
            char *ParentPath = GetPathFromNode(Parent);
            size_t PathSize = cwk_path_get_absolute(ParentPath, NormalizedPath, nullptr, 0);
            RelativePath = new char[PathSize + 1];
            cwk_path_get_absolute(ParentPath, NormalizedPath, RelativePath, PathSize + 1);
            delete[] ParentPath;
        }
        else
        {
            RelativePath = new char[strlen(NormalizedPath) + 1];
            strcpy(RelativePath, NormalizedPath);
        }
        delete[] NormalizedPath;
        vfsdbg("NormalizePath()->\"%s\"", RelativePath);
        return RelativePath;
    }

    FileStatus Virtual::FileExists(FileSystemNode *Parent, const char *Path)
    {
        vfsdbg("FileExists( Parent: \"%s\" Path: \"%s\" )", Parent->Name, Path);
        if (isempty((char *)Path))
            return FileStatus::INVALID_PATH;
        if (Parent == nullptr)
            Parent = FileSystemRoot;

        char *NormalizedPath = NormalizePath(Parent, Path);
        FileSystemNode *Node = GetNodeFromPath(Parent, NormalizedPath);

        if (Node == nullptr)
        {
            vfsdbg("FileExists()->NOT_FOUND");
            return FileStatus::NOT_FOUND;
        }
        else
        {
            vfsdbg("FileExists()->OK");
            return FileStatus::OK;
        }
    }

    FileSystemNode *Virtual::Create(FileSystemNode *Parent, const char *Path)
    {
        SmartLock(VFSLock);

        if (isempty((char *)Path))
            return nullptr;

        vfsdbg("Virtual::Create( Parent: \"%s\" Path: \"%s\" )", Parent->Name, Path);

        FileSystemNode *CurrentParent = nullptr;

        if (Parent == nullptr)
        {
            if (FileSystemRoot->Children.size() >= 1)
            {
                if (FileSystemRoot->Children[0] == nullptr)
                    panic("Root node is null!");

                CurrentParent = FileSystemRoot->Children[0]; // 0 - filesystem root
            }
            else
            {
                // TODO: check if here is a bug or something...
                const char *PathCopy;
                size_t length;
                PathCopy = (char *)Path;
                cwk_path_get_root(PathCopy, &length); // not working?
                foreach (auto var in FileSystemRoot->Children)
                    if (!strcmp(var->Name, PathCopy))
                    {
                        CurrentParent = var;
                        break;
                    }
            }
        }
        else
            CurrentParent = Parent;

        char *CleanPath = NormalizePath(CurrentParent, Path);

        if (FileExists(CurrentParent, CleanPath) != FileStatus::NOT_FOUND)
        {
            error("File %s already exists.", CleanPath);
            goto CreatePathError;
        }

        cwk_segment segment;
        if (!cwk_path_get_first_segment(CleanPath, &segment))
        {
            error("Path doesn't have any segments.");
            goto CreatePathError;
        }

        warn("Virtual::Create( ) is not working properly.");
        do
        {
            char *SegmentName = new char[segment.end - segment.begin + 1];
            memcpy(SegmentName, segment.begin, segment.end - segment.begin);

            if (GetChild(CurrentParent, SegmentName) == nullptr)
                CurrentParent = AddNewChild(CurrentParent, SegmentName);
            else
                CurrentParent = GetChild(CurrentParent, SegmentName);

            delete[] SegmentName;
        } while (cwk_path_get_next_segment(&segment));

        delete CleanPath;
        vfsdbg("Virtual::Create()->\"%s\"", CurrentParent->Name);
        return CurrentParent;

    CreatePathError:
        vfsdbg("Virtual::Create()->nullptr");
        delete CleanPath;
        return nullptr;
    }

    FileSystemNode *Virtual::CreateRoot(FileSystemOpeations *Operator, const char *RootName)
    {
        if (Operator == nullptr)
            return nullptr;
        vfsdbg("Setting root to %s", RootName);
        FileSystemNode *newNode = new FileSystemNode;
        strcpy(newNode->Name, RootName);
        newNode->Flags = NodeFlags::FS_DIRECTORY;
        newNode->Operator = Operator;
        FileSystemRoot->Children.push_back(newNode);
        return newNode;
    }

    FILE *Virtual::Mount(FileSystemOpeations *Operator, const char *Path)
    {
        SmartLock(VFSLock);

        if (Operator == nullptr)
            return nullptr;

        if (isempty((char *)Path))
            return nullptr;

        vfsdbg("Mounting %s", Path);
        FILE *file = new FILE;
        cwk_path_get_basename(Path, &file->Name, 0);
        file->Status = FileStatus::OK;
        file->Node = Create(nullptr, Path);
        file->Node->Operator = Operator;
        file->Node->Flags = NodeFlags::FS_MOUNTPOINT;
        return file;
    }

    FileStatus Virtual::Unmount(FILE *File)
    {
        SmartLock(VFSLock);
        if (File == nullptr)
            return FileStatus::INVALID_PARAMETER;
        vfsdbg("Unmounting %s", File->Name);
        return FileStatus::OK;
    }

    FILE *Virtual::Open(const char *Path, FileSystemNode *Parent)
    {
        SmartLock(VFSLock);
        vfsdbg("Opening %s with parent %s", Path, Parent->Name);

        if (strcmp(Path, ".") == 0)
        {
            FILE *file = new FILE;
            FileStatus filestatus = FileStatus::OK;
            file->Node = Parent;
            if (file->Node == nullptr)
                file->Status = FileStatus::NOT_FOUND;
            const char *basename;
            cwk_path_get_basename(GetPathFromNode(Parent), &basename, nullptr);
            file->Name = basename;
            return file;
        }

        if (strcmp(Path, "..") == 0)
        {
            if (Parent->Parent != nullptr)
                Parent = Parent->Parent;

            FILE *file = new FILE;
            FileStatus filestatus = FileStatus::OK;
            file->Node = Parent;
            if (file->Node == nullptr)
                file->Status = FileStatus::NOT_FOUND;
            const char *basename;
            cwk_path_get_basename(GetPathFromNode(Parent), &basename, nullptr);
            file->Name = basename;
            return file;
        }

        if (Parent == nullptr)
        {
            if (FileSystemRoot->Children.size() >= 1)
                Parent = FileSystemRoot->Children[0]; // 0 - filesystem root
            else
            {
                // TODO: check if here is a bug or something...
                const char *PathCopy;
                size_t length;
                PathCopy = (char *)Path;
                cwk_path_get_root(PathCopy, &length); // not working?
                foreach (auto var in FileSystemRoot->Children)
                    if (!strcmp(var->Name, PathCopy))
                    {
                        Parent = var;
                        break;
                    }
            }
        }

        char *CleanPath = NormalizePath(Parent, Path);

        FILE *file = new FILE;
        FileStatus filestatus = FileStatus::OK;
        filestatus = FileExists(Parent, CleanPath);
        /* TODO: Check for other errors */

        if (filestatus != FileStatus::OK)
        {
            foreach (auto var in FileSystemRoot->Children)
                if (!strcmp(var->Name, CleanPath))
                {
                    file->Node = var;
                    if (file->Node == nullptr)
                        goto OpenNodeFail;
                    const char *basename;
                    cwk_path_get_basename(GetPathFromNode(var), &basename, nullptr);
                    file->Name = basename;
                    goto OpenNodeExit;
                }

            file->Node = GetNodeFromPath(FileSystemRoot->Children[0], CleanPath);
            if (file->Node != nullptr)
            {
                const char *basename;
                cwk_path_get_basename(GetPathFromNode(file->Node), &basename, nullptr);
                file->Name = basename;
                goto OpenNodeExit;
            }

        OpenNodeFail:
            file->Status = filestatus;
            file->Node = nullptr;
        }
        else
        {
            file->Node = GetNodeFromPath(Parent, CleanPath);
            if (file->Node == nullptr)
                file->Status = FileStatus::NOT_FOUND;
            const char *basename;
            cwk_path_get_basename(CleanPath, &basename, nullptr);
            file->Name = basename;
            return file;
        }
    OpenNodeExit:
        return file;
    }

    uint64_t Virtual::Read(FILE *File, uint64_t Offset, uint8_t *Buffer, uint64_t Size)
    {
        SmartLock(VFSLock);
        if (File == nullptr)
            return 0;

        File->Status = FileStatus::OK;

        if (File->Node == nullptr)
        {
            File->Status = FileStatus::INVALID_PARAMETER;
            return 0;
        }

        if (File->Node->Operator == nullptr)
        {
            File->Status = FileStatus::INVALID_PARAMETER;
            return 0;
        }
        vfsdbg("Reading %s out->%016x", File->Name, Buffer);
        return File->Node->Operator->Read(File->Node, Offset, Size, Buffer);
    }

    uint64_t Virtual::Write(FILE *File, uint64_t Offset, uint8_t *Buffer, uint64_t Size)
    {
        SmartLock(VFSLock);
        if (File == nullptr)
            return 0;

        File->Status = FileStatus::OK;

        if (File->Node == nullptr)
        {
            File->Status = FileStatus::INVALID_PARAMETER;
            return 0;
        }

        if (File->Node->Operator == nullptr)
        {
            File->Status = FileStatus::INVALID_PARAMETER;
            return 0;
        }
        vfsdbg("Writing %s out->%016x", File->Name, Buffer);
        return File->Node->Operator->Write(File->Node, Offset, Size, Buffer);
    }

    FileStatus Virtual::Close(FILE *File)
    {
        SmartLock(VFSLock);
        if (File == nullptr)
            return FileStatus::INVALID_HANDLE;
        vfsdbg("Closing %s", File->Name);
        delete File;
        return FileStatus::OK;
    }

    Virtual::Virtual()
    {
        trace("Initializing virtual file system...");
        FileSystemRoot = new FileSystemNode;
        FileSystemRoot->Flags = NodeFlags::FS_MOUNTPOINT;
        FileSystemRoot->Operator = nullptr;
        FileSystemRoot->Parent = nullptr;
        strcpy(FileSystemRoot->Name, "root");
        cwk_path_set_style(CWK_STYLE_UNIX);
    }

    Virtual::~Virtual()
    {
        warn("Tried to uninitialize Virtual File System!");
    }
}
