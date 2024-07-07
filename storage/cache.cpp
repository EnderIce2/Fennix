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
	FileNode *Virtual::CacheSearchReturnLast(FileNode *Parent, const char **Path)
	{
		assert(Parent != nullptr);

		struct cwk_segment segment;
		if (!cwk_path_get_first_segment(*Path, &segment))
			ReturnLogError(nullptr, "Failed to get first segment of path");

		size_t segments = 0;
		while (cwk_path_get_next_segment(&segment))
			segments++;

		if (segments == 0)
			return Parent;

		const char *path = *Path;
		if (strncmp(path, "\002root-", 6) == 0) /* FIXME: deduce the index */
		{
			path += 6;
			while (*path != '\0' && *path != '\003')
				path++;
			if (*path == '\003')
				path++;
		}
		else
			path = *Path;

		FileNode *__Parent = Parent;
		if (this->PathIsAbsolute(path))
		{
			while (__Parent->Parent)
				__Parent = __Parent->Parent;
		}

		cwk_path_get_first_segment(path, &segment);
		do
		{
			std::string segmentName(segment.begin, segment.size);

			bool found = false;
			for (FileNode *fn : __Parent->Children)
			{
				if (fn->Name != segmentName)
					continue;

				cwk_segment __seg = segment;
				assert(cwk_path_get_next_segment(&__seg)); /* There's something wrong */

				__Parent = fn;
				found = true;
				break;
			}

			if (!found)
			{
				*Path = segment.begin;
				break;
			}
		} while (cwk_path_get_next_segment(&segment));

		return __Parent;
	}

	FileNode *Virtual::CacheRecursiveSearch(FileNode *Root, const char *NameOrPath, bool IsName)
	{
		if (Root == nullptr)
			return nullptr;

		debug("%s cache search for \"%s\" in \"%s\"", IsName ? "Relative" : "Absolute", NameOrPath, Root->Path.c_str());

		struct cwk_segment segment;
		if (!cwk_path_get_first_segment(NameOrPath, &segment))
			ReturnLogError(nullptr, "Failed to get first segment of path");

		size_t segments = 0;
		while (cwk_path_get_next_segment(&segment))
			segments++;

		if (IsName && segments == 0)
		{
			for (FileNode *fn : Root->Children)
			{
				if (fn->Name == NameOrPath)
					return fn;
			}

			ReturnLogError(nullptr, "Failed to find \"%s\" in \"%s\"", NameOrPath, Root->Path.c_str());
		}

		const char *path = NameOrPath;
		if (strncmp(path, "\002root-", 6) == 0) /* FIXME: deduce the index */
		{
			path += 6;
			while (*path != '\0' && *path != '\003')
				path++;
			if (*path == '\003')
				path++;
		}
		else
			path = NameOrPath;

		FileNode *__Parent = Root;
		if (this->PathIsAbsolute(path))
		{
			/* Get the root if Root is not the root 【・_・?】 */
			while (__Parent->Parent)
				__Parent = __Parent->Parent;
		}

		cwk_path_get_first_segment(path, &segment);
		do
		{
			std::string segmentName(segment.begin, segment.size);

			bool found = false;
			for (FileNode *fn : __Parent->Children)
			{
				if (fn->Name != segmentName)
					continue;

				cwk_segment __seg = segment;
				if (!cwk_path_get_next_segment(&__seg))
					return fn;

				__Parent = fn;
				found = true;
				break;
			}

			if (!found)
				break;
		} while (cwk_path_get_next_segment(&segment));

		debug("Failed to find \"%s\" in \"%s\"", NameOrPath, Root->Path.c_str());
		return nullptr;
	}

	FileNode *Virtual::CacheLookup(const char *Path)
	{
		FileNode *rootNode = thisProcess ? thisProcess->Info.RootNode : this->GetRoot(0);

		FileNode *ret = CacheRecursiveSearch(rootNode, Path, false);
		if (ret)
			return ret;

		debug("Path \"%s\" not found", Path);
		return nullptr;
	}

	FileNode *Virtual::CreateCacheNode(FileNode *Parent, Inode *Node, const char *Name, mode_t Mode)
	{
		FileNode *fn = new FileNode;
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
			return -EINVAL;

		if (Node->Parent)
		{
			Node->Parent->Children.erase(std::find(Node->Parent->Children.begin(), Node->Parent->Children.end(), Node));
			// delete Node;
			fixme("Node deletion is disabled for now (for debugging purposes)");
		}

		return 0;
	}
}
