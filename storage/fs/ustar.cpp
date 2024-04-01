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
#include <debug.h>

#include "../../kernel.h"

namespace vfs
{
	size_t USTARNode::read(uint8_t *Buffer,
						   size_t Size,
						   off_t Offset)
	{
		if (Size <= 0)
			Size = this->Size;

		if (Offset > this->Size)
			return 0;

		if ((this->Size - Offset) == 0)
			return 0; /* EOF */

		if (Offset + (off_t)Size > this->Size)
			Size = this->Size;

		memcpy(Buffer, (uint8_t *)(this->Address + Offset), Size);
		return Size;
	}

	USTARNode::USTARNode(uintptr_t Address,
						 const char *Name,
						 NodeType Type,
						 Virtual *vfs_ctx)
		: Node(nullptr,
			   Name,
			   Type,
			   true,
			   vfs_ctx,
			   nullptr),
		  Address(Address)
	{
	}

	USTARNode::~USTARNode() {}

	bool USTAR::TestArchive(uintptr_t Address)
	{
		if (!Memory::Virtual().Check((void *)Address))
		{
			error("Address %#lx is not mapped!", Address);
			return false;
		}

		FileHeader *header = (FileHeader *)Address;
		if (memcmp(header->signature, "ustar", 5) != 0)
		{
			error("ustar signature invalid!");
			return false;
		}
		return true;
	}

	void USTAR::ReadArchive(uintptr_t Address, Virtual *vfs_ctx)
	{
		trace("Initializing USTAR with address %#lx", Address);

		if (!this->TestArchive(Address))
			return; /* Check whether the archive is deflated */

		FileHeader *header = (FileHeader *)Address;

		debug("USTAR signature valid! Name:%s Signature:%s Mode:%d Size:%lu",
			  header->name, header->signature,
			  string2int(header->mode), header->size);

		for (size_t i = 0;; i++)
		{
			if (memcmp(header->signature, "ustar", 5) != 0)
				break;

			memmove(header->name,
					header->name + 1,
					strlen(header->name));

			if (header->name[strlen(header->name) - 1] == '/')
			{
				debug("Removing trailing slash from %s", header->name);
				header->name[strlen(header->name) - 1] = 0;
			}

			// if (!isempty((char *)header->name))
			//     KPrint("Adding file \e88AACC%s\eCCCCCC (\e88AACC%lu \eCCCCCCbytes)", header->name, size);
			// else
			//     goto NextFileAddress;

			size_t size = getsize(header->size);
			Node *node;
			NodeType type = NODE_TYPE_NONE;
			if (isempty((char *)header->name))
				goto NextFileAddress;

			switch (header->typeflag[0])
			{
			case REGULAR_FILE:
				type = NodeType::FILE;
				break;
			case SYMLINK:
				type = NodeType::SYMLINK;
				break;
			case DIRECTORY:
				type = NodeType::DIRECTORY;
				break;
			case CHARDEV:
				type = NodeType::CHARDEVICE;
				break;
			case BLOCKDEV:
				type = NodeType::BLOCKDEVICE;
				break;
			default:
				warn("Unknown type: %d", header->typeflag[0]);
				break;
			}

			node = new USTARNode((Address + 512), header->name,
								 type, vfs_ctx);

			// debug("%s %d KiB, Type:%c", header->name,
			// 	  TO_KiB(size), header->typeflag[0]);
			node->Mode = string2int(header->mode);
			node->Size = size;
			node->GroupIdentifier = getsize(header->gid);
			node->UserIdentifier = getsize(header->uid);
			node->DeviceMajor = getsize(header->dev_maj);
			node->DeviceMinor = getsize(header->dev_min);

			node->AccessTime = getsize(header->mtime);
			node->ModifyTime = getsize(header->mtime);
			node->ChangeTime = getsize(header->mtime);
			node->IndexNode = i;

			if (type == NodeType::SYMLINK)
			{
				node->Symlink = new char[strlen(header->link) + 1];
				strncpy((char *)node->Symlink,
						header->link,
						strlen(header->link));
			}

		NextFileAddress:
			Address += ((size / 512) + 1) * 512;
			if (size % 512)
				Address += 512;

			header = (FileHeader *)Address;
		}
	}

	USTAR::USTAR() {}

	USTAR::~USTAR() {}
}
