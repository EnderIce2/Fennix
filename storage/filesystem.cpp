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

namespace vfs
{
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
		vfsdbg("PathIsRelative()->\"%s\"",
			   IsRelative ? "true" : "false");
		return IsRelative;
	}

	Node *Virtual::GetParent(const char *Path, Node *Parent)
	{
		vfsdbg("GetParent( Path: \"%s\" Parent: \"%s\" )",
			   Path, Parent ? Parent->Name : "(nil)");

		if (Parent)
		{
			vfsdbg("GetParent()->\"%s\"", Parent->Name);
			return Parent;
		}

		Parent = FileSystemRoot->Children[0];

		size_t length;
		cwk_path_get_root(Path, &length);
		if (length > 0)
		{
			foreach (auto Child in FileSystemRoot->Children)
			{
				if (strcmp(Child->Name, Path) == 0)
				{
					Parent = Child;
					break;
				}
			}
		}

		vfsdbg("GetParent()->\"%s\"", ParentNode->Name);
		return Parent;
	}

	const char *Virtual::NormalizePath(const char *Path, Node *Parent)
	{
		assert(Parent != nullptr);

		vfsdbg("NormalizePath( Path: \"%s\" Parent: \"%s\" )",
			   Path, Parent->Name);

		size_t PathSize = strlen((char *)Path) + 1;
		char *NormalizedPath = new char[PathSize];

		{
			Memory::SmartHeap sh(PathSize);
			memcpy(sh, (char *)Path, PathSize);
			cwk_path_normalize(sh, NormalizedPath, PathSize);
		}

		const char *FinalPath;
		if (cwk_path_is_relative(NormalizedPath))
		{
			size_t PathSize = cwk_path_join(Parent->FullPath,
											NormalizedPath,
											nullptr, 0);

			FinalPath = new char[PathSize + 1];
			cwk_path_join(Parent->FullPath, NormalizedPath,
						  (char *)FinalPath, PathSize + 1);

			delete[] NormalizedPath;
		}
		else
			FinalPath = NormalizedPath;

		vfsdbg("NormalizePath()->\"%s\"", FinalPath);
		return FinalPath;
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

		const char *CleanPath = NormalizePath(Path, Parent);
		bool ret = GetNodeFromPath(CleanPath, Parent) != nullptr;
		delete[] CleanPath;
		vfsdbg("PathExists()->\"%s\"",
			   ret ? "true" : "false");
		return ret;
	}

	Node *Virtual::Create(const char *Path, NodeType Type, Node *Parent)
	{
		if (isempty((char *)Path))
			return nullptr;

		SmartLock(VirtualLock);
		Node *RootNode = FileSystemRoot->Children[0];
		Node *CurrentParent = this->GetParent(Path, Parent);
		vfsdbg("Virtual::Create( Path: \"%s\" Parent: \"%s\" )",
			   Path, Parent ? Parent->Name : CurrentParent->Name);

		const char *CleanPath = this->NormalizePath(Path, CurrentParent);
		vfsdbg("CleanPath: \"%s\"", CleanPath);

		VirtualLock.Unlock();
		if (PathExists(CleanPath, CurrentParent))
		{
			error("Path \"%s\" already exists.", CleanPath);
			goto CreatePathError;
		}
		VirtualLock.Lock(__FUNCTION__);

		cwk_segment segment;
		if (!cwk_path_get_first_segment(CleanPath, &segment))
		{
			error("Path doesn't have any segments.");
			goto CreatePathError;
		}

		do
		{
			char *SegmentName = new char[segment.end - segment.begin + 1];
			memcpy(SegmentName, segment.begin, segment.end - segment.begin);
			vfsdbg("SegmentName: \"%s\"", SegmentName);

			auto GetChild = [](const char *Name, Node *Parent)
			{
				vfsdbg("GetChild( Name: \"%s\" Parent: \"%s\" )",
					   Name, Parent->Name);

				if (!Parent)
				{
					vfsdbg("GetChild()->nullptr");
					return (Node *)nullptr;
				}

				foreach (auto Child in Parent->Children)
				{
					if (strcmp(Child->Name, Name) == 0)
					{
						vfsdbg("GetChild()->\"%s\"", Child->Name);
						return Child;
					}
				}

				vfsdbg("GetChild()->nullptr (not found)");
				return (Node *)nullptr;
			};

			if (Parent)
			{
				if (GetChild(SegmentName, RootNode) != nullptr)
				{
					RootNode = GetChild(SegmentName, RootNode);
					delete[] SegmentName;
					continue;
				}
			}

			if (GetChild(SegmentName, CurrentParent) != nullptr)
				CurrentParent = GetChild(SegmentName, CurrentParent);
			else
			{
				CurrentParent = new Node(CurrentParent,
										 SegmentName,
										 NodeType::DIRECTORY);
			}

			delete[] SegmentName;
		} while (cwk_path_get_next_segment(&segment));

		CurrentParent->Type = Type;
		// CurrentParent->FullPath = CleanPath;

		vfsdbg("Virtual::Create()->\"%s\"", CurrentParent->Name);
#ifdef DEBUG
		VirtualLock.Unlock();
		debug("Path created: \"%s\"",
			  CurrentParent->FullPath);
		VirtualLock.Lock(__FUNCTION__);
#endif
		return CurrentParent;

	CreatePathError:
		delete[] CleanPath;
		vfsdbg("Virtual::Create()->nullptr");
		return nullptr;
	}

	Node *Virtual::CreateLink(const char *Path, const char *Target, Node *Parent)
	{
		Node *node = this->Create(Path, NodeType::SYMLINK, Parent);
		if (node)
		{
			node->Symlink = new char[strlen(Target) + 1];
			strncpy((char *)node->Symlink,
					Target,
					strlen(Target));

			node->SymlinkTarget = node->vFS->GetNodeFromPath(node->Symlink);
			return node;
		}

		error("Failed to create link \"%s\" -> \"%s\"",
			  Path, Target);
		return nullptr;
	}

	int Virtual::Delete(const char *Path, bool Recursive, Node *Parent)
	{
		vfsdbg("Virtual::Delete( Path: \"%s\" Parent: \"%s\" )",
			   Path, Parent ? Parent->Name : "(null)");

		if (isempty((char *)Path))
			return -EINVAL;

		if (Parent == nullptr)
			Parent = FileSystemRoot;

		const char *CleanPath = this->NormalizePath(Path, Parent);
		vfsdbg("CleanPath: \"%s\"", CleanPath);

		if (!PathExists(CleanPath, Parent))
		{
			vfsdbg("Path \"%s\" doesn't exist.", CleanPath);
			delete[] CleanPath;
			return -ENOENT;
		}

		Node *NodeToDelete = GetNodeFromPath(CleanPath, Parent);

		if (!NodeToDelete->References.empty())
			fixme("Path \"%s\" is referenced by %d objects.",
				  CleanPath, NodeToDelete->References.size());

		delete[] CleanPath;
		delete NodeToDelete;
		return 0;
	}

	int Virtual::Delete(Node *Path, bool Recursive, Node *Parent)
	{
		return Delete(Path->FullPath, Recursive, Parent);
	}

	RefNode *Virtual::Open(const char *Path, Node *Parent)
	{
		vfsdbg("Opening \"%s\" with parent \"%s\"",
			   Path, Parent ? Parent->Name : "(null)");

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
		const char *CleanPath = NormalizePath(Path, CurrentParent);

		if (PathExists(CleanPath, CurrentParent))
		{
			Node *node = GetNodeFromPath(CleanPath, CurrentParent);
			if (node)
			{
				delete[] CleanPath;
				/* TODO: Check if dir or file? */
				return node->CreateReference();
			}
		}

		return nullptr;
	}

	Node *Virtual::CreateIfNotExists(const char *Path, NodeType Type, Node *Parent)
	{
		Node *node = GetNodeFromPath(Path, Parent);
		if (node)
			return node;
		return Create(Path, Type, Parent);
	}

	Virtual::Virtual()
	{
		SmartLock(VirtualLock);
		trace("Initializing virtual file system...");
		FileSystemRoot = new Node(nullptr, "<root>", NodeType::MOUNTPOINT);
		FileSystemRoot->vFS = this;
	}

	Virtual::~Virtual()
	{
		SmartLock(VirtualLock);
		stub;
		/* TODO: sync, cache */
	}
}
