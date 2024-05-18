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
	FileNode *Virtual::__CacheRecursiveSearch(FileNode *Root, const char *NameOrPath, bool IsName)
	{
		if (Root == nullptr)
			return nullptr;

		if (IsName)
		{
			if (strcmp(Root->Name.c_str(), NameOrPath) == 0)
				return Root;
		}
		else
		{
			if (strcmp(Root->Path.c_str(), NameOrPath) == 0)
				return Root;
		}

		for (const auto &Child : Root->Children)
		{
			FileNode *ret = __CacheRecursiveSearch(Child, NameOrPath, IsName);
			if (ret)
				return ret;
		}

		debug("Failed to find %s in %s", NameOrPath, Root->Path.c_str());
		return nullptr;
	}

	FileNode *Virtual::CacheLookup(const char *Path)
	{
		FileNode *rootNode = thisProcess ? thisProcess->Info.RootNode : this->GetRoot(0);

		FileNode *ret = __CacheRecursiveSearch(rootNode, Path, false);
		if (ret)
			return ret;

		debug("Path \"%s\" not found", Path);
		return nullptr;
		__unreachable;

		debug("Path \"%s\" not found; attempting to search by segments", Path);
		/* FIXME: This may not be the greatest idea */

		struct cwk_segment segment;
		if (!cwk_path_get_first_segment(Path, &segment))
			return __CacheRecursiveSearch(rootNode, Path, true);

		do
		{
			std::string segmentStr(segment.begin, segment.size);
			ret = __CacheRecursiveSearch(rootNode, segmentStr.c_str(), true);
			if (ret)
				return ret;
		} while (cwk_path_get_next_segment(&segment));

		return nullptr;
	}

	FileNode *Virtual::CreateCacheNode(FileNode *Parent, Inode *Node, const char *Name, mode_t Mode)
	{
		FileNode *fn = new FileNode();
		fn->Name = Name;
		if (Parent)
		{
			fn->Path = Parent->Path + "/" + Name;
			Parent->Children.push_back(fn);
		}
		else
			fn->Path = Name;
		fn->Parent = Parent;
		fn->Node = Node;
		fn->fsi = DeviceMap[Node->Device].fsi;
		if (fn->fsi == nullptr)
			warn("Failed to find filesystem for device %d", Node->Device);

		debug("Created cache node %s", fn->Path.c_str());
		return fn;
	}

	int Virtual::RemoveCacheNode(FileNode *Node)
	{
		if (Node == nullptr)
			return -1;

		if (Node->Parent)
		{
			Node->Parent->Children.erase(std::find(Node->Parent->Children.begin(), Node->Parent->Children.end(), Node));
			// delete Node;
			fixme("Node deletion is disabled for now (for debugging purposes)");
		}

		return 0;
	}
}
