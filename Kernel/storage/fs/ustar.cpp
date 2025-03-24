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

#include <filesystem/ustar.hpp>

#include <memory.hpp>
#include <functional>
#include <debug.h>

#include "../../kernel.h"

#define TMAGIC "ustar"
#define TMAGLEN 6
#define TVERSION "00"
#define TVERSLEN 2

namespace vfs
{
	int USTAR::Lookup(struct Inode *_Parent, const char *Name, struct Inode **Result)
	{
		auto Parent = (USTARInode *)_Parent;

		const char *basename;
		size_t length;
		cwk_path_get_basename(Name, &basename, &length);
		if (basename == NULL)
		{
			if (strcmp(Name, "/") == 0)
			{
				auto &it = Files.at(0);
				*Result = &it->Node;
				return 0;
			}

			error("Invalid name %s", Name);
			return -EINVAL;
		}

		if (_Parent)
		{
			for (const auto &child : Parent->Children)
			{
				if (child->Deleted || strcmp(child->Name.c_str(), basename) != 0)
					continue;

				*Result = &child->Node;
				return 0;
			}

			return -ENOENT;
		}

		auto fileItr = Files.begin();
		while (fileItr != Files.end())
		{
			USTARInode *node = fileItr->second;

			if (node->Deleted || strcmp(node->Name.c_str(), basename) != 0)
			{
				fileItr++;
				continue;
			}

			*Result = &fileItr->second->Node;
			return 0;
		}
		return -ENOENT;
	}

	int USTAR::Create(struct Inode *_Parent, const char *Name, mode_t Mode, struct Inode **Result)
	{
		USTARInode *Parent = (USTARInode *)_Parent;

		Inode inode{};
		inode.Mode = Mode;
		inode.Device = this->DeviceID;
		inode.RawDevice = 0;
		inode.Index = NextInode;
		inode.Offset = 0;
		inode.PrivateData = this;
		inode.Flags = I_FLAG_CACHE_KEEP;

		const char *basename;
		size_t length;
		cwk_path_get_basename(Name, &basename, &length);

		auto SetMode = [&](mode_t &Mode, FileHeader *header)
		{
			if (Mode & S_IFREG)
				header->typeflag[0] = REGTYPE;
			else if (Mode & S_IFLNK)
				header->typeflag[0] = SYMTYPE;
			else if (Mode & S_IFCHR)
				header->typeflag[0] = CHRTYPE;
			else if (Mode & S_IFBLK)
				header->typeflag[0] = BLKTYPE;
			else if (Mode & S_IFDIR)
				header->typeflag[0] = DIRTYPE;
			else if (Mode & S_IFIFO)
				header->typeflag[0] = FIFOTYPE;

			mode_t final = 0;
			if (Mode & S_ISUID)
				final |= TSUID;
			else if (Mode & S_ISGID)
				final |= TSGID;
			else if (Mode & S_ISVTX)
				final |= TSVTX;
			else if (Mode & S_IRUSR)
				final |= TUREAD;
			else if (Mode & S_IWUSR)
				final |= TUWRITE;
			else if (Mode & S_IXUSR)
				final |= TUEXEC;
			else if (Mode & S_IRGRP)
				final |= TGREAD;
			else if (Mode & S_IWGRP)
				final |= TGWRITE;
			else if (Mode & S_IXGRP)
				final |= TGEXEC;
			else if (Mode & S_IROTH)
				final |= TOREAD;
			else if (Mode & S_IWOTH)
				final |= TOWRITE;
			else if (Mode & S_IXOTH)
				final |= TOEXEC;

			snprintf(header->mode, sizeof(header->mode), "%07o", final);
		};

		FileHeader *hdr = new FileHeader{};
		SetMode(inode.Mode, hdr);
		strncpy(hdr->name, basename, sizeof(hdr->name));
		strncpy(hdr->signature, TMAGIC, TMAGLEN);
		strncpy(hdr->version, TVERSION, TVERSLEN);

		USTARInode *node = new USTARInode{.Node = inode,
										  .Header = hdr,
										  .Parent = Parent,
										  .Name{},
										  .Path{},
										  .Children{},
										  .Deleted = false,
										  .Checksum = INODE_CHECKSUM};

		node->Name.assign(basename, length);
		node->Path.assign(Name, strlen(Name));

		Files.insert(std::make_pair(NextInode, node));
		*Result = &Files.at(NextInode)->Node;
		if (Parent)
			Parent->Children.push_back(Files.at(NextInode));
		NextInode++;
		return 0;
	}

	ssize_t USTAR::Read(struct Inode *Node, void *Buffer, size_t Size, off_t Offset)
	{
		auto fileItr = Files.find(Node->Index);
		assert(fileItr != Files.end());

		if (fileItr->second->Deleted)
			return -ENOENT;

		USTARInode *node = fileItr->second;
		size_t fileSize = GetSize(node->Header->size);

		if (Size <= 0)
		{
			debug("Size is less than or equal to 0");
			Size = fileSize;
		}

		if ((size_t)Offset > fileSize)
		{
			debug("Offset %d is greater than file size %d", Offset, fileSize);
			return 0;
		}

		if ((fileSize - Offset) == 0)
		{
			debug("Offset %d is equal to file size %d", Offset, fileSize);
			return 0; /* EOF */
		}

		if ((size_t)Offset + Size > fileSize)
		{
			debug("Offset %d + Size %d is greater than file size %d",
				  Offset, Size, fileSize);
			Size = fileSize;
		}

		memcpy(Buffer, (uint8_t *)((uintptr_t)node->Header + sizeof(FileHeader) + Offset), Size);
		// debug("Read %d bytes from %d[%d]", Size, Node->Index, Offset);
		return Size;
	}

	__no_sanitize("alignment")
		ssize_t USTAR::ReadDir(struct Inode *_Node, struct kdirent *Buffer, size_t Size, off_t Offset, off_t Entries)
	{
		/* FIXME: FIX ALIGNMENT FOR DIRENT! */
		auto Node = (USTARInode *)_Node;

		off_t realOffset = Offset;

		size_t totalSize = 0;
		uint16_t reclen = 0;
		struct kdirent *ent = nullptr;

		if (Offset == 0)
		{
			reclen = (uint16_t)(offsetof(struct kdirent, d_name) + strlen(".") + 1);
			if (totalSize + reclen >= Size)
				return -EINVAL;

			ent = (struct kdirent *)((uintptr_t)Buffer + totalSize);
			ent->d_ino = Node->Node.Index;
			ent->d_off = Offset++;
			ent->d_reclen = reclen;
			ent->d_type = DT_DIR;
			strcpy(ent->d_name, ".");
			totalSize += reclen;
		}

		if (Offset <= 1)
		{
			reclen = (uint16_t)(offsetof(struct kdirent, d_name) + strlen("..") + 1);
			if (totalSize + reclen >= Size)
			{
				if (realOffset == 1)
					return -EINVAL;
				return totalSize;
			}

			ent = (struct kdirent *)((uintptr_t)Buffer + totalSize);

			if (Node->Parent)
				ent->d_ino = Node->Parent->Node.Index;
			else
			{
				warn("Parent is null for %s", Node->Name.c_str());
				ent->d_ino = Node->Node.Index;
			}
			ent->d_off = Offset++;
			ent->d_reclen = reclen;
			ent->d_type = DT_DIR;
			strcpy(ent->d_name, "..");
			totalSize += reclen;
		}

		// off_t entriesSkipped = 0;
		// auto fileItr = Files.begin();
		// while (fileItr != Files.end())
		// {
		// 	if (fileItr->second->Deleted)
		// 		continue;
		// 	reclen = (uint16_t)(offsetof(struct kdirent, d_name) + strlen(fileItr->second->Name.c_str()) + 1);
		// 	if (Offset > entriesSkipped)
		// 	{
		// 		entriesSkipped++;
		// 		continue;
		// 	}
		// 	if (totalSize + reclen >= Size)
		// 		break;
		// 	ent = (struct kdirent *)((uintptr_t)Buffer + totalSize);
		// 	ent->d_ino = fileItr->first;
		// 	ent->d_off = Offset++;
		// 	ent->d_reclen = reclen;
		// 	ent->d_type = IFTODT(StringToInt(fileItr->second->Header->mode));
		// 	strncpy(ent->d_name,
		// 			fileItr->second->Name.c_str(),
		// 			strlen(fileItr->second->Name.c_str()));
		// 	totalSize += reclen;
		// 	fileItr++;
		// }

		if (!S_ISDIR(Node->Node.Mode))
			return -ENOTDIR;

		if ((Offset >= 2 ? (Offset - 2) : Offset) > (off_t)Node->Children.size())
			return -EINVAL;

		off_t entries = 0;
		for (const auto &var : Node->Children)
		{
			if (var->Node.Offset < Offset)
				continue;

			if (entries >= Entries)
				break;

			if (var->Deleted)
				continue;

			reclen = (uint16_t)(offsetof(struct kdirent, d_name) + strlen(var->Name.c_str()) + 1);

			if (totalSize + reclen >= Size)
				break;

			ent = (struct kdirent *)((uintptr_t)Buffer + totalSize);
			ent->d_ino = var->Node.Index;
			ent->d_off = var->Node.Offset;
			ent->d_reclen = reclen;
			switch (var->Header->typeflag[0])
			{
			case AREGTYPE:
			case REGTYPE:
				ent->d_type = DT_REG;
				break;
			case LNKTYPE:
				fixme("Hard link not implemented for %s", var->Header->name);
				ent->d_type = DT_LNK;
				break;
			case SYMTYPE:
				ent->d_type = DT_LNK;
				break;
			case CHRTYPE:
				ent->d_type = DT_CHR;
				break;
			case BLKTYPE:
				ent->d_type = DT_BLK;
				break;
			case DIRTYPE:
				ent->d_type = DT_DIR;
				break;
			case FIFOTYPE:
				ent->d_type = DT_FIFO;
				break;
			case CONTTYPE:
			default:
				ent->d_type = 0;
				break;
			}
			strncpy(ent->d_name, var->Name.c_str(), strlen(var->Name.c_str()));

			totalSize += reclen;
			entries++;
		}

		if (totalSize + sizeof(struct kdirent) >= Size)
			return totalSize;

		ent = (struct kdirent *)((uintptr_t)Buffer + totalSize);
		ent->d_ino = 0;
		ent->d_off = 0;
		ent->d_reclen = 0;
		ent->d_type = DT_UNKNOWN;
		ent->d_name[0] = '\0';
		return totalSize;
	}

	int USTAR::SymLink(struct Inode *Node, const char *Name, const char *Target, struct Inode **Result)
	{
		int ret = this->Create(Node, Name, S_IFLNK, Result);
		if (ret < 0)
			return ret;

		USTARInode *node = (USTARInode *)*Result;
		FileHeader *hdr = node->Header;
		strncpy(hdr->link, Target, MIN(sizeof(hdr->link) - 1, strlen(Target)));
		return 0;
	}

	ssize_t USTAR::ReadLink(struct Inode *Node, char *Buffer, size_t Size)
	{
		auto fileItr = Files.find(Node->Index);
		assert(fileItr != Files.end());

		if (fileItr->second->Deleted)
			return -ENOENT;

		USTARInode *node = fileItr->second;

		if (strlen(node->Header->link) > Size)
			Size = strlen(node->Header->link);

		strncpy(Buffer, node->Header->link, Size);
		debug("Read %d bytes from %d", Size, Node->Index);
		return Size;
	}

	int USTAR::Stat(struct Inode *Node, struct kstat *Stat)
	{
		auto fileItr = Files.find(Node->Index);
		assert(fileItr != Files.end());
		if (fileItr->second->Deleted)
			return -ENOENT;

		USTARInode *node = fileItr->second;
		size_t fileSize = GetSize(node->Header->size);

		debug("Header: \"%.*s\"", sizeof(struct FileHeader), node->Header);
		Stat->Device = this->DeviceID;
		Stat->Index = Node->Index;
		Stat->HardLinks = 1;
		Stat->UserID = GetSize(node->Header->uid);
		Stat->GroupID = GetSize(node->Header->gid);
		Stat->RawDevice = Stat->MakeDevice(GetSize(node->Header->dev_maj), GetSize(node->Header->dev_min));
		Stat->Size = fileSize;
		Stat->AccessTime = GetSize(node->Header->mtime);
		Stat->ModifyTime = GetSize(node->Header->mtime);
		Stat->ChangeTime = GetSize(node->Header->mtime);
		Stat->BlockSize = 512;
		Stat->Blocks = (fileSize + 511) / 512;
		Stat->Attribute = 0;

		mode_t hdrMode = StringToInt(node->Header->mode);

		if (hdrMode & TSUID)
			Stat->Mode |= S_ISUID;
		else if (hdrMode & TSGID)
			Stat->Mode |= S_ISGID;
		else if (hdrMode & TSVTX)
			Stat->Mode |= S_ISVTX;
		else if (hdrMode & TUREAD)
			Stat->Mode |= S_IRUSR;
		else if (hdrMode & TUWRITE)
			Stat->Mode |= S_IWUSR;
		else if (hdrMode & TUEXEC)
			Stat->Mode |= S_IXUSR;
		else if (hdrMode & TGREAD)
			Stat->Mode |= S_IRGRP;
		else if (hdrMode & TGWRITE)
			Stat->Mode |= S_IWGRP;
		else if (hdrMode & TGEXEC)
			Stat->Mode |= S_IXGRP;
		else if (hdrMode & TOREAD)
			Stat->Mode |= S_IROTH;
		else if (hdrMode & TOWRITE)
			Stat->Mode |= S_IWOTH;
		else if (hdrMode & TOEXEC)
			Stat->Mode |= S_IXOTH;

		switch (node->Header->typeflag[0])
		{
		case AREGTYPE:
		case REGTYPE:
			Stat->Mode |= S_IFREG;
			break;
		case LNKTYPE:
			fixme("Hard link not implemented for %s", node->Header->name);
			Stat->Mode |= S_IFLNK;
			break;
		case SYMTYPE:
			Stat->Mode |= S_IFLNK;
			break;
		case CHRTYPE:
			Stat->Mode |= S_IFCHR;
			break;
		case BLKTYPE:
			Stat->Mode |= S_IFBLK;
			break;
		case DIRTYPE:
			Stat->Mode |= S_IFDIR;
			break;
		case FIFOTYPE:
			Stat->Mode |= S_IFIFO;
			break;
		case CONTTYPE:
			warn("Reserved type for %s", node->Header->name);
			__fallthrough;
		default:
			error("Unknown type: %d for %s", node->Header->typeflag[0], node->Header->name);
			break;
		}
		return 0;
	}

	bool USTAR::TestArchive(uintptr_t Address)
	{
		if (!Memory::Virtual().Check((void *)Address))
		{
			error("Address %#lx is not mapped!", Address);
			return false;
		}

		FileHeader *header = (FileHeader *)Address;
		if (strncmp(header->signature, TMAGIC, TMAGLEN) != 0)
		{
			error("Invalid signature!");
			return false;
		}
		return true;
	}

	void USTAR::ReadArchive(uintptr_t Address, size_t Size)
	{
		trace("Initializing USTAR with address %#lx and size %d", Address, Size);

		auto SetMode = [&](Inode &uNode, FileHeader *header)
		{
			mode_t hdrMode = StringToInt(header->mode);
			if (hdrMode & TSUID)
				uNode.Mode |= S_ISUID;
			else if (hdrMode & TSGID)
				uNode.Mode |= S_ISGID;
			else if (hdrMode & TSVTX)
				uNode.Mode |= S_ISVTX;
			else if (hdrMode & TUREAD)
				uNode.Mode |= S_IRUSR;
			else if (hdrMode & TUWRITE)
				uNode.Mode |= S_IWUSR;
			else if (hdrMode & TUEXEC)
				uNode.Mode |= S_IXUSR;
			else if (hdrMode & TGREAD)
				uNode.Mode |= S_IRGRP;
			else if (hdrMode & TGWRITE)
				uNode.Mode |= S_IWGRP;
			else if (hdrMode & TGEXEC)
				uNode.Mode |= S_IXGRP;
			else if (hdrMode & TOREAD)
				uNode.Mode |= S_IROTH;
			else if (hdrMode & TOWRITE)
				uNode.Mode |= S_IWOTH;
			else if (hdrMode & TOEXEC)
				uNode.Mode |= S_IXOTH;

			switch (header->typeflag[0])
			{
			case AREGTYPE:
			case REGTYPE:
				uNode.Mode |= S_IFREG;
				break;
			case LNKTYPE:
				uNode.Mode |= S_IFLNK;
				break;
			case SYMTYPE:
				uNode.Mode |= S_IFLNK;
				break;
			case CHRTYPE:
				uNode.Mode |= S_IFCHR;
				break;
			case BLKTYPE:
				uNode.Mode |= S_IFBLK;
				break;
			case DIRTYPE:
				uNode.Mode |= S_IFDIR;
				break;
			case FIFOTYPE:
				uNode.Mode |= S_IFIFO;
				break;
			case CONTTYPE:
				warn("Reserved type for %s", header->name);
				__fallthrough;
			default:
				error("Unknown type: %d for %s", header->typeflag[0], header->name);
				break;
			}
		};

		FileHeader *header = (FileHeader *)Address;

		debug("USTAR signature valid! Name:%s Signature:%s Mode:%d Size:%lu",
			  header->name, header->signature, StringToInt(header->mode), header->size);

		Memory::Virtual vmm;
		std::vector<USTARInode *> tmpNodes; /* FIXME: bug in unordered_map for iterators */
		for (size_t i = 0;; i++)
		{
			if (!vmm.Check((void *)header))
			{
				error("Address %#lx is not mapped!", header);
				return;
			}

			if (strncmp(header->signature, TMAGIC, TMAGLEN) != 0)
				break;
			// debug("\"%s\"", header->name);

			/* This removes the "." at the beginning of the file name
				"./foo/bar" > "/foo/bar" */
			if (header->name[0] == '.' && header->name[1] == '/')
				memmove(header->name, header->name + 1, strlen(header->name));

			if (isempty((char *)header->name))
				fixme("Ignoring empty file name \"%.*s\"", sizeof(struct FileHeader), header);

			struct Inode uNode;
			uNode.Device = this->DeviceID;
			uNode.RawDevice = 0;
			uNode.Index = NextInode;
			SetMode(uNode, header);
			uNode.Flags = I_FLAG_CACHE_KEEP;
			uNode.Offset = 0;
			uNode.PrivateData = this;

			const char *basename;
			size_t length;
			cwk_path_get_basename(header->name, &basename, &length);

			USTARInode *node = new USTARInode{.Node = uNode,
											  .Header = header,
											  .Parent = nullptr,
											  .Name{},
											  .Path{},
											  .Children{},
											  .Deleted = false,
											  .Checksum = INODE_CHECKSUM};

			if (basename)
				node->Name.assign(basename, length);
			else
				node->Name.assign((const char *)header->name, strlen(header->name));
			node->Path.assign((const char *)header->name, strlen(header->name));

			Files.insert(std::make_pair(NextInode, node));
			tmpNodes.push_back(node);

			size_t size = GetSize(header->size);
			Address += ((size / 512) + 1) * 512;
			if (size % 512)
				Address += 512;

			header = (FileHeader *)Address;
			NextInode++;
		}

		/* TODO: This code can be significantly optimized but good luck understanding it */
		USTARInode *parent = nullptr;
		std::vector<USTARInode *> parentStack;
		std::vector<std::string *> pathStack;
		for (auto &file : tmpNodes)
		{
			if (file->Path == "/") /* This is root / */
			{
				parentStack.push_back(file);
				pathStack.push_back(&file->Path);
				// debug("root / generated");
				continue;
			}

			/* pathStack is never empty */

			if (file->Path.back() == '/') /* This is a directory */
			{
				const char *path = file->Path.c_str();
				size_t length;
				/* This converts /one/two/path.txt to /one/two/ */
				cwk_path_get_dirname(path, &length);
				std::string dirName(path, length);

				/* Check if the directory is at the same level as the current directory */
				if (dirName == *pathStack.back())
				{
					parent = parentStack.back();
					parentStack.push_back(file);
					pathStack.push_back(&file->Path);
					parent->Children.push_back(file);
					file->Parent = parent;
					// debug("adding \"%s\" to \"%s\"", file->Path.c_str(), parent->Path.c_str());
					continue;
				}
				else
				{
					/* Check if the directory is at a higher level */
					for (size_t i = 0; i < parentStack.size(); i++)
					{
						if (dirName != *pathStack[i])
							continue;

						/* Adjust vectors */
						while (!parentStack.empty())
						{
							if (dirName == *pathStack.back())
								break;

							// debug("popping \"%s\"", pathStack.back()->c_str());
							parentStack.pop_back();
							pathStack.pop_back();
						}

						parent = parentStack.back();
						parentStack.push_back(file);
						pathStack.push_back(&file->Path);
						parent->Children.push_back(file);
						file->Parent = parent;
						// debug("adding \"%s\" to \"%s\"", file->Path.c_str(), parent->Path.c_str());
						goto foundEnd;
					}

					// This is a new directory level
					parentStack.pop_back();
					pathStack.pop_back();
					parent = parentStack.back();
					parentStack.push_back(file);
					pathStack.push_back(&file->Path);
					parent->Children.push_back(file);
					file->Parent = parent;
					// debug("adding \"%s\" to \"%s\"", file->Path.c_str(), parent->Path.c_str());
				foundEnd:
					continue;
				}
			}

			/* From here, it's a file */

			const char *path = file->Path.c_str();
			size_t length;
			/* This converts /one/two/path.txt to /one/two/ */
			cwk_path_get_dirname(path, &length);
			std::string dirName(path, length);

			/* Check if the file is at the same level as the current directory */
			if (dirName == *pathStack.back())
			{
				parent = parentStack.back();
				parent->Children.push_back(file);
				file->Parent = parent;
				// debug("adding \"%s\" to \"%s\"", file->Path.c_str(), parent->Path.c_str());
				continue;
			}

			/* Check if the file is at a higher level */
			for (size_t i = 0; i < parentStack.size(); i++)
			{
				if (dirName != *pathStack[i])
					continue;

				/* Adjust vectors */
				while (!parentStack.empty())
				{
					if (dirName == *pathStack.back())
						break;

					// debug("popping \"%s\"", pathStack.back()->c_str());
					parentStack.pop_back();
					pathStack.pop_back();
				}

				parent = parentStack.back();
				parent->Children.push_back(file);
				file->Parent = parent;
				// debug("adding \"%s\" to \"%s\"", file->Path.c_str(), parent->Path.c_str());
			}
		}

		std::function<void(vfs::USTAR::USTARInode *, int, const std::string &)> ustarTree = [&](vfs::USTAR::USTARInode *node, int level, const std::string &prefix)
		{
			debug("%*s\"%s\"%ld", level * 4, prefix.c_str(), node->Name.c_str(), node->Node.Offset);
			off_t offset = 2; /* 0 . | 1 .. */
			for (auto &child : node->Children)
			{
#ifdef DEBUG
				if (offset <= 2)
				{
					if (offset == 2)
						offset = 0;
					/* Pseudo directories . and .. */
					USTARInode pseudoDot{};
					pseudoDot.Node = node->Node;
					pseudoDot.Name = ".";
					pseudoDot.Node.Offset = offset++;
					USTARInode pseudoDDot{};
					pseudoDDot.Node = node->Parent ? node->Parent->Node : node->Node;
					pseudoDDot.Name = "..";
					pseudoDDot.Node.Offset = offset++;
					ustarTree(&pseudoDot, level + 1, "|-- ");
					ustarTree(&pseudoDDot, level + 1, "|-- ");
				}
#endif

				child->Node.Offset = offset++;
				ustarTree(child, level + 1, child == node->Children.back() ? "`-- " : "|-- ");
			}
		};

		ustarTree(tmpNodes[0], 0, "");
	}
}

O2 int __ustar_Lookup(struct Inode *Parent, const char *Name, struct Inode **Result)
{
	return ((vfs::USTAR *)Parent->PrivateData)->Lookup(Parent, Name, Result);
}

O2 int __ustar_Create(struct Inode *Parent, const char *Name, mode_t Mode, struct Inode **Result)
{
	return ((vfs::USTAR *)Parent->PrivateData)->Create(Parent, Name, Mode, Result);
}

O2 ssize_t __ustar_Read(struct Inode *Node, void *Buffer, size_t Size, off_t Offset)
{
	return ((vfs::USTAR *)Node->PrivateData)->Read(Node, Buffer, Size, Offset);
}

O2 ssize_t __ustar_Readdir(struct Inode *Node, struct kdirent *Buffer, size_t Size, off_t Offset, off_t Entries)
{
	return ((vfs::USTAR *)Node->PrivateData)->ReadDir(Node, Buffer, Size, Offset, Entries);
}

O2 int __ustar_SymLink(Inode *Parent, const char *Name, const char *Target, Inode **Result)
{
	return ((vfs::USTAR *)Parent->PrivateData)->SymLink(Parent, Name, Target, Result);
}

O2 ssize_t __ustar_ReadLink(Inode *Node, char *Buffer, size_t Size)
{
	return ((vfs::USTAR *)Node->PrivateData)->ReadLink(Node, Buffer, Size);
}

O2 int __ustar_Stat(struct Inode *Node, kstat *Stat)
{
	return ((vfs::USTAR *)Node->PrivateData)->Stat(Node, Stat);
}

int __ustar_DestroyInode(FileSystemInfo *Info, Inode *Node)
{
	((vfs::USTAR::USTARInode *)Node)->Deleted = true;
	return 0;
}

int __ustar_Destroy(FileSystemInfo *fsi)
{
	assert(fsi->PrivateData);
	delete (vfs::USTAR *)fsi->PrivateData;
	delete fsi;
	return 0;
}

bool TestAndInitializeUSTAR(uintptr_t Address, size_t Size)
{
	vfs::USTAR *ustar = new vfs::USTAR();
	if (!ustar->TestArchive(Address))
	{
		delete ustar;
		return false;
	}

	ustar->DeviceID = fs->EarlyReserveDevice();
	ustar->ReadArchive(Address, Size);

	Inode *rootfs = nullptr;
	ustar->Lookup(nullptr, "/", &rootfs);
	assert(rootfs != nullptr);

	FileSystemInfo *fsi = new FileSystemInfo;
	fsi->Name = "ustar";
	fsi->RootName = "/";
	fsi->Flags = I_FLAG_ROOT | I_FLAG_MOUNTPOINT | I_FLAG_CACHE_KEEP;
	fsi->SuperOps.DeleteInode = __ustar_DestroyInode;
	fsi->SuperOps.Destroy = __ustar_Destroy;
	fsi->Ops.Lookup = __ustar_Lookup;
	fsi->Ops.Create = __ustar_Create;
	fsi->Ops.Read = __ustar_Read;
	fsi->Ops.ReadDir = __ustar_Readdir;
	fsi->Ops.SymLink = __ustar_SymLink;
	fsi->Ops.ReadLink = __ustar_ReadLink;
	fsi->Ops.Stat = __ustar_Stat;
	fsi->PrivateData = ustar;
	fs->LateRegisterFileSystem(ustar->DeviceID, fsi, rootfs);

	fs->AddRoot(rootfs);
	return true;
}
