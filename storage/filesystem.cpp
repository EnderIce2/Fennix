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

#include <convert.h>
#include <printf.h>
#include <rand.hpp>
#include <cwalk.h>

#include "../kernel.h"

namespace vfs
{
	bool Virtual::PathIsRelative(const char *Path)
	{
		return cwk_path_is_relative(Path);
	}

	void Virtual::AddRoot(Inode *Root)
	{
		SmartLock(VirtualLock);
		FileSystemRoots->Children.push_back(Root);
	}

	FileNode *Virtual::GetRoot(size_t Index)
	{
		if (Index >= FileSystemRoots->Children.size())
			return nullptr;

		Inode *RootNode = FileSystemRoots->Children[Index];

		char rootName[128]{};
		snprintf(rootName, sizeof(rootName), "\002root-%ld\003", Index);

		auto it = FileRoots.find(Index);
		if (it != FileRoots.end())
			return it->second;

		FileNode *ret = this->CreateCacheNode(nullptr, RootNode, rootName, 0);
		FileRoots.insert({Index, ret});
		return ret;
	}

	FileNode *Virtual::Create(FileNode *Parent, const char *Name, mode_t Mode)
	{
		FileNode *existingNode = this->GetByPath(Name, Parent);
		if (existingNode != nullptr)
			ReturnLogError(existingNode, "File %s already exists", Name);

		if (Parent == nullptr)
		{
			assert(thisProcess != nullptr);
			Parent = thisProcess->Info.RootNode;
		}

		auto it = DeviceMap.find(Parent->Node->Device);
		if (it == DeviceMap.end())
			ReturnLogError(nullptr, "Device %d not found", Parent->Node->Device);

		Inode *Node = NULL;
		if (it->second.fsi->Ops.Create == NULL)
			ReturnLogError(nullptr, "Create not supported for %d", it->first);

		int ret = it->second.fsi->Ops.Create(Parent->Node, Name, Mode, &Node);
		if (ret < 0)
			ReturnLogError(nullptr, "Create for %d failed with %d", it->first, ret);

		return this->CreateCacheNode(Parent, Node, Name, Mode);
	}

	FileNode *Virtual::ForceCreate(FileNode *Parent, const char *Name, mode_t Mode)
	{
		fixme("ForceCreate: %s", Name);
		return this->Create(Parent, Name, Mode);
	}

	FileNode *Virtual::Mount(FileNode *Parent, Inode *Node, const char *Path)
	{
		char *path = strdup(Path);
		char *lastSlash = strrchr(path, '/');
		if (lastSlash == path)
			lastSlash++;
		*lastSlash = '\0';

		FileNode *parentNode = this->GetByPath(path, Parent);
		free(path);
		lastSlash = strrchr(Path, '/');
		lastSlash++;
		return this->CreateCacheNode(parentNode, Node, lastSlash, Node->Mode);
	}

	int Virtual::Unmount(const char *Path)
	{
		FileNode *node = this->GetByPath(Path, nullptr);
		if (node == nullptr)
			ReturnLogError(-ENOENT, "Path %s not found", Path);

		return this->RemoveCacheNode(node);
	}

	FileNode *Virtual::GetByPath(const char *Path, FileNode *Parent)
	{
		debug("GetByPath: %s", Path);
		if (Parent == nullptr)
			Parent = thisProcess ? thisProcess->Info.RootNode : this->GetRoot(0);

		if (strcmp(Path, ".") == 0)
			return Parent;

		if (strcmp(Path, "..") == 0)
			return Parent->Parent ? Parent->Parent : Parent;

		FileNode *fn = this->CacheRecursiveSearch(Parent, Path, this->PathIsRelative(Path));
		if (fn)
			return fn;

		if (strncmp(Path, "\002root-", 6) == 0) /* FIXME: deduce the index */
		{
			Path += 7;
			while (*Path != '\0' && *Path != '\003')
				Path++;
			if (*Path == '\003')
				Path++;
		}

		FileNode *__Parent = CacheSearchReturnLast(Parent, &Path);

		struct cwk_segment segment;
		if (!cwk_path_get_first_segment(Path, &segment))
		{
			auto it = DeviceMap.find(Parent->Node->Device);
			if (unlikely(it == DeviceMap.end()))
				ReturnLogError(nullptr, "Device %d not found", Parent->Node->Device);

			if (it->second.fsi->Ops.Lookup == NULL)
				ReturnLogError(nullptr, "Lookup not supported for %d", it->first);

			Inode *Node = NULL;
			int ret = it->second.fsi->Ops.Lookup(Parent->Node, Path, &Node);
			if (ret < 0)
				ReturnLogError(nullptr, "Lookup for \"%s\"(%d) failed with %d", Path, it->first, ret);

			if (Parent->Node == Node) /* root / */
			{
				debug("Returning root (%#lx)", Node);
				return Parent;
			}
			ReturnLogError(nullptr, "Path has no segments");
		}

		Inode *Node = NULL;
		do
		{
			auto it = DeviceMap.find(__Parent->Node->Device);
			if (unlikely(it == DeviceMap.end()))
				ReturnLogError(nullptr, "Device %d not found", __Parent->Node->Device);

			if (it->second.fsi->Ops.Lookup == NULL)
				ReturnLogError(nullptr, "Lookup not supported for %d", it->first);

			std::string segmentName(segment.begin, segment.size);
			int ret = it->second.fsi->Ops.Lookup(__Parent->Node, segmentName.c_str(), &Node);
			if (ret < 0)
				ReturnLogError(nullptr, "Lookup for \"%s\"(%d) failed with %d", segmentName.c_str(), it->first, ret);
			__Parent = this->CreateCacheNode(__Parent, Node, segmentName.c_str(), 0);
		} while (cwk_path_get_next_segment(&segment));

		FileNode *ret = __Parent;
		if (!ret->IsDirectory())
			return ret;

		auto it = DeviceMap.find(__Parent->Node->Device);
		if (unlikely(it == DeviceMap.end()))
			ReturnLogError(nullptr, "Device %d not found", __Parent->Node->Device);

		size_t dirAllocLen = sizeof(struct kdirent) + strlen(Path);
		struct kdirent *dirent = (struct kdirent *)malloc(dirAllocLen);
		size_t offset = 2; /* Skip . and .. */
		while (it->second.fsi->Ops.ReadDir(Node, dirent, dirAllocLen, offset++, 1) > 0)
		{
			Inode *ChildNode = NULL;
			int luRet = it->second.fsi->Ops.Lookup(Node, dirent->d_name, &ChildNode);
			if (luRet < 0)
			{
				debug("Lookup for %d failed with %d", it->first, luRet);
				break;
			}

			this->CreateCacheNode(ret, ChildNode, dirent->d_name, 0);
		}
		free(dirent);
		return ret;
	}

	std::string Virtual::GetByNode(FileNode *Node)
	{
		assert(Node != nullptr);
		if (Node->Parent == nullptr)
		{
			if (Node->Node->Flags & I_FLAG_ROOT)
				return Node->fsi->RootName;
			assert(Node->Parent != nullptr);
		}

		std::string path;

		auto appendPath = [&path](const char *name)
		{
			if (path.size() > 0)
				path += "/";
			path += name;
		};

		FileNode *current = Node;
		while (current->Parent != nullptr)
		{
			appendPath(current->Name.c_str());
			current = current->Parent;
		}

		return path;
	}

	FileNode *Virtual::CreateLink(const char *Path, FileNode *Parent, const char *Target)
	{
		auto it = DeviceMap.find(Parent->Node->Device);
		if (it == DeviceMap.end())
			ReturnLogError(nullptr, "Device %d not found", Parent->Node->Device);

		Inode *Node = NULL;

		if (it->second.fsi->Ops.SymLink == NULL)
			ReturnLogError(nullptr, "SymLink not supported for %d", it->first);

		int ret = it->second.fsi->Ops.SymLink(Parent->Node, Path, Target, &Node);
		if (ret < 0)
			ReturnLogError(nullptr, "SymLink for %d failed with %d", it->first, ret);
		return this->CreateCacheNode(Parent, Node, Path, 0);
	}

	FileNode *Virtual::CreateLink(const char *Path, FileNode *Parent, FileNode *Target)
	{
		return this->CreateLink(Path, Parent, Target->Path.c_str());
	}

	bool Virtual::PathExists(const char *Path, FileNode *Parent)
	{
		FileNode *fn = this->CacheLookup(Path);
		if (fn)
			return true;

		FileNode *Node = this->GetByPath(Path, Parent);
		if (Node)
			return true;
		return false;
	}

	int Virtual::Remove(FileNode *Node)
	{
		auto it = DeviceMap.find(Node->Node->Device);
		if (it == DeviceMap.end())
			ReturnLogError(-ENODEV, "Device %d not found", Node->Node->Device);

		if (it->second.fsi->Ops.Remove == NULL)
			ReturnLogError(-ENOTSUP, "Remove not supported for %d", it->first);

		int ret = it->second.fsi->Ops.Remove(Node->Parent->Node, Node->Name.c_str());
		if (ret < 0)
			ReturnLogError(ret, "Remove for %d failed with %d", it->first, ret);

		this->RemoveCacheNode(Node);
		return 0;
	}
}
