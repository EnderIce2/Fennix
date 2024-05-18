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

	dev_t Virtual::EarlyReserveDevice()
	{
		RegisterLock.store(true);
		size_t len = DeviceMap.size();
		return len;
	}

	int Virtual::LateRegisterFileSystem(dev_t Device, FileSystemInfo *fsi, Inode *Root)
	{
		auto it = DeviceMap.find(Device);
		if (it != DeviceMap.end())
			ReturnLogError(-EEXIST, "Device %d already registered", Device);

		FSMountInfo fsmi{.fsi = fsi, .Root = Root};
		DeviceMap.insert({Device, fsmi});
		RegisterLock.store(false);
		return 0;
	}

	dev_t Virtual::RegisterFileSystem(FileSystemInfo *fsi, Inode *Root)
	{
		RegisterLock.store(true);
		size_t len = DeviceMap.size();
		FSMountInfo fsmi{.fsi = fsi, .Root = Root};
		DeviceMap.insert({len, fsmi});
		RegisterLock.store(false);
		return len;
	}

	int Virtual::UnregisterFileSystem(dev_t Device)
	{
		auto it = DeviceMap.find(Device);
		if (it == DeviceMap.end())
			ReturnLogError(-ENOENT, "Device %d not found", Device);

		if (it->second.fsi->SuperOps.Synchronize)
			it->second.fsi->SuperOps.Synchronize(it->second.fsi, NULL);
		if (it->second.fsi->SuperOps.Destroy)
			it->second.fsi->SuperOps.Destroy(it->second.fsi);
		DeviceMap.erase(it);
		return 0;
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
		snprintf(rootName, sizeof(rootName), "root-%ld", Index);

		return this->CreateCacheNode(nullptr, RootNode, rootName, 0);
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

	FileNode *Virtual::GetByPath(const char *Path, FileNode *Parent)
	{
		FileNode *fn = this->CacheLookup(Path);
		if (fn)
			return fn;

		if (Parent == nullptr)
			Parent = thisProcess ? thisProcess->Info.RootNode : this->GetRoot(0);

		auto it = DeviceMap.find(Parent->Node->Device);
		if (it == DeviceMap.end())
			ReturnLogError(nullptr, "Device %d not found", Parent->Node->Device);

		struct cwk_segment segment;
		if (!cwk_path_get_first_segment(Path, &segment))
			ReturnLogError(nullptr, "Path has no segments");

		Inode *Node = NULL;
		FileNode *__Parent = Parent;
		do
		{
			if (it->second.fsi->Ops.Lookup == NULL)
				ReturnLogError(nullptr, "Lookup not supported for %d", it->first);

			std::string segmentName(segment.begin, segment.size);
			int ret = it->second.fsi->Ops.Lookup(__Parent->Node, segmentName.c_str(), &Node);
			if (ret < 0)
				ReturnLogError(nullptr, "Lookup for %d failed with %d", it->first, ret);
			__Parent = this->CreateCacheNode(__Parent, Node, segmentName.c_str(), 0);
		} while (cwk_path_get_next_segment(&segment));

		FileNode *ret = __Parent;
		if (!ret->IsDirectory())
			return ret;

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
