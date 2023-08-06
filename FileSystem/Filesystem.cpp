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

#include <smart_ptr.hpp>
#include <convert.h>
#include <printf.h>
#include <cwalk.h>

#include "../kernel.h"

// show debug messages
// #define DEBUG_FILESYSTEM 1

#ifdef DEBUG_FILESYSTEM
#define vfsdbg(m, ...) debug(m, ##__VA_ARGS__)
#else
#define vfsdbg(m, ...)
#endif

namespace VirtualFileSystem
{
	std::string Virtual::GetPathFromNode(Node *File)
	{
		vfsdbg("GetPathFromNode( Node: \"%s\" )", File->Name);
		SmartLock(VirtualLock);

		Node *Parent = File;
		char **Path = nullptr;
		size_t PathSize = 0;
		std::string FinalPath;

		while (Parent != FileSystemRoot && Parent != nullptr)
		{
			if (File == FileSystemRoot->Children[0])
			{
				FinalPath = "/";
				break;
			}

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
			Path[PathSize] = (char *)Parent->Name;
			PathSize++;
			new_path = new char *[PathSize + 1];
			memcpy(new_path, Path, sizeof(char *) * PathSize);
			delete[] Path;
			Path = new_path;
			Path[PathSize] = (char *)"/";
			PathSize++;
			Parent = Parent->Parent;
		}

		for (size_t i = PathSize - 1; i < PathSize; i--)
		{
			if (Path[i] == nullptr)
				continue;
			FinalPath += Path[i];
		}

		FinalPath += "\0";
		delete[] Path, Path = nullptr;
		vfsdbg("GetPathFromNode()->\"%s\"", FinalPath.c_str());
		return FinalPath;
	}

	Node *Virtual::GetNodeFromPath_Unsafe(const char *Path, Node *Parent)
	{
		vfsdbg("GetNodeFromPath( Path: \"%s\" Parent: \"%s\" )",
			   Path, Parent ? Parent->Name : "(null)");

		if (strcmp(Path, "/") == 0)
			return FileSystemRoot->Children[0]; // 0 - filesystem root

		if (strcmp(Path, ".") == 0)
			return Parent;

		if (strcmp(Path, "..") == 0)
		{
			if (Parent)
			{
				if (Parent->Parent)
					return Parent->Parent;
				else
					return Parent;
			}
			else
				return nullptr;
		}

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
			errno = ENOENT;
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
				vfsdbg("comparing \"%s\" with \"%s\"",
					   Child->Name, SegmentName);
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
		vfsdbg("BaseName: \"%s\" NodeName: \"%s\"",
			   basename, ReturnNode->Name);

		if (strcmp(basename, ReturnNode->Name) == 0)
		{
			vfsdbg("GetNodeFromPath()->\"%s\"", ReturnNode->Name);
			return ReturnNode;
		}

		vfsdbg("GetNodeFromPath()->\"(null)\"");
		errno = ENOENT;
		return nullptr;
	}

	Node *Virtual::GetNodeFromPath(const char *Path, Node *Parent)
	{
		SmartLock(VirtualLock);
		return GetNodeFromPath_Unsafe(Path, Parent);
	}

	bool Virtual::PathIsRelative(const char *Path)
	{
		vfsdbg("PathIsRelative( Path: \"%s\" )", Path);
		bool IsRelative = cwk_path_is_relative(Path);
		vfsdbg("PathIsRelative()->%s", IsRelative ? "true" : "false");
		return IsRelative;
	}

	Node *Virtual::GetParent(const char *Path, Node *Parent)
	{
		vfsdbg("GetParent( Path: \"%s\" Parent: \"%s\" )",
			   Path, Parent->Name);

		if (Parent)
		{
			vfsdbg("GetParent()->\"%s\"", Parent->Name);
			return Parent;
		}

		Node *ParentNode = nullptr;
		if (FileSystemRoot->Children.size() >= 1)
		{
			assert(FileSystemRoot->Children[0] != nullptr);
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
		vfsdbg("AddNewChild( Name: \"%s\" Parent: \"%s\" )",
			   Name, Parent->Name);

		Node *newNode = new Node;
		newNode->Parent = Parent;
		newNode->Name = new char[strlen(Name) + 1];
		strncpy((char *)newNode->Name, Name, strlen(Name));

		newNode->Operator = Parent->Operator;
		newNode->FileSystem = this;
		Parent->Children.push_back(newNode);

		vfsdbg("AddNewChild()->\"%s\"", newNode->Name);
		return newNode;
	}

	Node *Virtual::GetChild(const char *Name, Node *Parent)
	{
		vfsdbg("GetChild( Name: \"%s\" Parent: \"%s\" )",
			   Name, Parent->Name);

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

	int Virtual::RemoveChild(const char *Name, Node *Parent)
	{
		vfsdbg("RemoveChild( Name: \"%s\" Parent: \"%s\" )",
			   Name, Parent->Name);

		forItr(itr, Parent->Children)
		{
			if (strcmp((*itr)->Name, Name) == 0)
			{
				delete *itr, *itr = nullptr;
				Parent->Children.erase(itr);
				vfsdbg("RemoveChild()->OK");
				return 0;
			}
		}


		vfsdbg("RemoveChild()->NotFound");
		return -1;
	}

	std::string Virtual::NormalizePath(const char *Path, Node *Parent)
	{
		vfsdbg("NormalizePath( Path: \"%s\" Parent: \"%s\" )",
			   Path, Parent->Name);

		char *NormalizedPath = new char[strlen((char *)Path) + 1];
		std::string RelativePath;

		cwk_path_normalize(Path, NormalizedPath, strlen((char *)Path) + 1);

		if (cwk_path_is_relative(NormalizedPath))
		{
			std::string ParentPath = GetPathFromNode(Parent);
			size_t PathSize = cwk_path_join(ParentPath.c_str(),
											NormalizedPath,
											nullptr, 0);

			RelativePath.resize(PathSize + 1);

			cwk_path_join(ParentPath.c_str(), NormalizedPath,
						  (char *)RelativePath.c_str(),
						  PathSize + 1);
		}
		else
		{
			RelativePath = NormalizedPath;
		}
		delete[] NormalizedPath;
		vfsdbg("NormalizePath()->\"%s\"", RelativePath.get());
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

		vfsdbg("PathExists( Path: \"%s\" Parent: \"%s\" )",
			   Path, Parent->Name);

		if (GetNodeFromPath(NormalizePath(Path, Parent).c_str(), Parent))
		{
			vfsdbg("PathExists()->OK");
			return true;
		}

		vfsdbg("PathExists()->NotFound");
		return false;
	}

	Node *Virtual::CreateRoot(const char *RootName,
							  FileSystemOperations *Operator)
	{
		if (Operator == nullptr)
			return nullptr;

		debug("Creating root %s", RootName);

		SmartLock(VirtualLock);
		Node *newNode = new Node;
		newNode->Name = RootName;
		newNode->Flags = NodeFlags::DIRECTORY;
		newNode->Operator = Operator;
		newNode->FileSystem = this;
		FileSystemRoot->Children.push_back(newNode);
		return newNode;
	}

	/* TODO: Further testing needed */
	Node *Virtual::Create(const char *Path, NodeFlags Flag, Node *Parent)
	{
		if (isempty((char *)Path))
			return nullptr;

		SmartLock(VirtualLock);
		Node *RootNode = FileSystemRoot->Children[0];
		Node *CurrentParent = this->GetParent(Path, Parent);
		vfsdbg("Virtual::Create( Path: \"%s\" Parent: \"%s\" )",
			   Path, Parent ? Parent->Name : CurrentParent->Name);

		VirtualLock.Unlock();
		std::string CleanPath = this->NormalizePath(Path, CurrentParent);
		VirtualLock.Lock(__FUNCTION__);
		vfsdbg("CleanPath: \"%s\"", CleanPath.get());

		VirtualLock.Unlock();
		if (PathExists(CleanPath.c_str(), CurrentParent))
		{
			error("Path %s already exists.", CleanPath.c_str());
			goto CreatePathError;
		}
		VirtualLock.Lock(__FUNCTION__);

		cwk_segment segment;
		if (!cwk_path_get_first_segment(CleanPath.c_str(), &segment))
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
			{
				if (GetChild(SegmentName, RootNode) != nullptr)
				{
					RootNode = GetChild(SegmentName, RootNode);
					delete[] SegmentName;
					continue;
				}
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
#ifdef DEBUG
		VirtualLock.Unlock();
		debug("Path created: \"%s\"",
			  GetPathFromNode(CurrentParent).c_str());
		VirtualLock.Lock(__FUNCTION__);
#endif
		return CurrentParent;

	CreatePathError:
		vfsdbg("Virtual::Create()->nullptr");
		return nullptr;
	}

	int Virtual::Delete(const char *Path, bool Recursive, Node *Parent)
	{
		vfsdbg("Virtual::Delete( Path: \"%s\" Parent: \"%s\" )",
			   Path, Parent ? Parent->Name : "(null)");

		if (isempty((char *)Path))
		{
			errno = EINVAL;
			return -1;
		}

		if (Parent == nullptr)
			Parent = FileSystemRoot;

		std::string CleanPath = this->NormalizePath(Path, Parent);
		vfsdbg("CleanPath: \"%s\"", CleanPath.c_str());

		if (!PathExists(CleanPath.c_str(), Parent))
		{
			vfsdbg("Path %s doesn't exist.", CleanPath.c_str());
			errno = ENOENT;
			return -1;
		}

		Node *NodeToDelete = GetNodeFromPath(CleanPath.c_str(), Parent);

		if (NodeToDelete->Flags == NodeFlags::DIRECTORY)
		{
			SmartLock(VirtualLock);
			if (Recursive)
			{
				foreach (auto Child in NodeToDelete->Children)
				{
					VirtualLock.Unlock();
					int Status = Delete(GetPathFromNode(Child).c_str(), true);
					VirtualLock.Lock(__FUNCTION__);
					if (Status != 0)
					{
						error("Failed to delete child %s with status %d. (%s)",
							   Child->Name, Status, Path);
						errno = EIO;
						return -1;
					}
				}
			}
			else if (NodeToDelete->Children.size() > 0)
			{
				error("Directory %s is not empty.", CleanPath.c_str());
				errno = ENOTEMPTY;
				return -1;
			}
		}

		SmartLock(VirtualLock);
		Node *ParentNode = GetParent(CleanPath.c_str(), Parent);
		if (RemoveChild(NodeToDelete->Name, ParentNode) != 0)
		{
			error("Failed to remove child %s from parent %s. (%s)", NodeToDelete->Name, ParentNode->Name, Path);
			errno = EIO;
			return -1;
		}

		debug("Deleted %s", CleanPath.c_str());
		vfsdbg("Virtual::Delete()->OK");
		return 0;
	}

	int Virtual::Delete(Node *Path, bool Recursive, Node *Parent)
	{
		std::string PathString = GetPathFromNode(Path);
		return Delete(PathString.c_str(), Recursive, Parent);
	}

	/* TODO: REWORK */
	Node *Virtual::Mount(const char *Path, FileSystemOperations *Operator)
	{
		if (unlikely(!Operator))
		{
			errno = EFAULT;
			return nullptr;
		}

		if (unlikely(isempty((char *)Path)))
		{
			errno = EINVAL;
			return nullptr;
		}

		vfsdbg("Mounting %s", Path);
		const char *PathCopy;
		cwk_path_get_basename(Path, &PathCopy, 0);
		Node *MountPoint = Create(Path, NodeFlags::MOUNTPOINT);
		MountPoint->Operator = Operator;
		return MountPoint;
	}

	int Virtual::Unmount(Node *File)
	{
		if (unlikely(!File))
		{
			errno = EINVAL;
			return -1;
		}

		if (unlikely(File->Flags != NodeFlags::MOUNTPOINT))
		{
			errno = ENOTDIR;
			return -1;
		}

		fixme("Unmounting %s",
			  File->Name);
		errno = ENOSYS;
		return -1;
	}

	RefNode *Virtual::Open(const char *Path, Node *Parent)
	{
		vfsdbg("Opening %s with parent %s", Path, Parent ? Parent->Name : "(null)");

		if (strcmp(Path, "/") == 0)
			return FileSystemRoot->CreateReference();

		if (!Parent)
			Parent = FileSystemRoot->Children[0];

		if (strcmp(Path, ".") == 0)
			return Parent->CreateReference();

		if (strcmp(Path, "..") == 0)
		{
			if (Parent->Parent)
				return Parent->Parent->CreateReference();
			else
				return Parent->CreateReference();
		}

		Node *CurrentParent = this->GetParent(Path, Parent);
		std::string CleanPath = NormalizePath(Path, CurrentParent);

		/* TODO: Check for other errors */
		if (!PathExists(CleanPath.c_str(), CurrentParent))
		{
			{
				SmartLock(VirtualLock);
				foreach (auto Child in FileSystemRoot->Children)
				{
					if (strcmp(Child->Name, CleanPath.c_str()) != 0)
						continue;

					return Child->CreateReference();
				}
			}

			Node *node = GetNodeFromPath(CleanPath.c_str(), FileSystemRoot->Children[0]);
			if (node)
				return node->CreateReference();
		}
		else
		{
			Node *node = GetNodeFromPath(CleanPath.c_str(), CurrentParent);
			if (node)
				return node->CreateReference();
		}

		errno = ENOENT;
		return nullptr;
	}

	Virtual::Virtual()
	{
		SmartLock(VirtualLock);
		trace("Initializing virtual file system...");
		FileSystemRoot = new Node;
		FileSystemRoot->Flags = NodeFlags::MOUNTPOINT;
		FileSystemRoot->Operator = nullptr;
		FileSystemRoot->Parent = nullptr;
		FileSystemRoot->Name = "root";
		FileSystemRoot->FileSystem = this;
		cwk_path_set_style(CWK_STYLE_UNIX);
	}

	Virtual::~Virtual()
	{
		SmartLock(VirtualLock);
		stub;
		/* TODO: sync, cache */
	}
}
