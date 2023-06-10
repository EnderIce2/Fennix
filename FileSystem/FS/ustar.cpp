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

namespace VirtualFileSystem
{
    ReadFSFunction(USTAR_Read)
    {
        if (!Size)
            Size = node->Length;

        if ((size_t)node->Offset > node->Length)
            return 0;

        if (node->Offset + Size > node->Length)
            Size = node->Length - node->Offset;

        memcpy(Buffer, (uint8_t *)(node->Address + node->Offset), Size);
        return Size;
    }

    FileSystemOperations ustar_op = {
        .Name = "ustar",
        .Read = USTAR_Read,
    };

    bool USTAR::TestArchive(uintptr_t Address)
    {
        if (!Memory::Virtual().Check((void *)Address))
        {
            error("Address %#lx is not mapped!", Address);
            return false;
        }

        if (memcmp(((FileHeader *)(uintptr_t)Address)->signature, "ustar", 5) != 0)
        {
            error("ustar signature invalid!");
            return false;
        }
        return true;
    }

    void USTAR::ReadArchive(uintptr_t Address, Virtual *vfs_ctx)
    {
        trace("Initializing USTAR with address %#llx", Address);

        if (!this->TestArchive(Address))
            return; /* Check whether the archive is deflated */

        debug("USTAR signature valid! Name:%s Signature:%s Mode:%c Size:%lu",
              ((FileHeader *)Address)->name,
              ((FileHeader *)Address)->signature,
              string2int(((FileHeader *)Address)->mode),
              ((FileHeader *)Address)->size);

        vfs_ctx->CreateRoot("/", &ustar_op);

        for (size_t i = 0;; i++)
        {
            FileHeader *header = (FileHeader *)Address;
            if (memcmp(((FileHeader *)Address)->signature, "ustar", 5) != 0)
                break;
            memmove(header->name, header->name + 1, strlen(header->name));
            if (header->name[strlen(header->name) - 1] == '/')
                header->name[strlen(header->name) - 1] = 0;
            size_t size = getsize(header->size);
            Node *node = nullptr;

            // if (!isempty((char *)header->name))
            //     KPrint("Adding file \e88AACC%s\eCCCCCC (\e88AACC%lu \eCCCCCCbytes)", header->name, size);
            // else
            //     goto NextFileAddress;

            if (isempty((char *)header->name))
                goto NextFileAddress;

            node = vfs_ctx->Create(header->name, NodeFlags::NODE_FLAG_ERROR);
            debug("Added node: %s", node->Name);
            if (node == nullptr)
            {
                static int ErrorsAllowed = 20;

                if (ErrorsAllowed > 0)
                {
                    ErrorsAllowed--;
                    goto NextFileAddress;
                }
                else
                {
                    error("Adding USTAR files failed because too many files were corrupted or invalid.");
                    break;
                }
            }
            else
            {
                trace("%s %dKB Type:%c", header->name, TO_KB(size), header->typeflag[0]);
                node->Mode = string2int(header->mode);
                node->Address = (Address + 512);
                node->Length = size;
                node->GroupIdentifier = getsize(header->gid);
                node->UserIdentifier = getsize(header->uid);
                node->IndexNode = i;

                switch (header->typeflag[0])
                {
                case REGULAR_FILE:
                    node->Flags = NodeFlags::FILE;
                    break;
                case SYMLINK:
                    node->Flags = NodeFlags::SYMLINK;
                    break;
                case DIRECTORY:
                    node->Flags = NodeFlags::DIRECTORY;
                    break;
                case CHARDEV:
                    node->Flags = NodeFlags::CHARDEVICE;
                    break;
                case BLOCKDEV:
                    node->Flags = NodeFlags::BLOCKDEVICE;
                    break;
                default:
                    warn("Unknown type: %d", header->typeflag[0]);
                    break;
                }
            NextFileAddress:
                Address += ((size / 512) + 1) * 512;
                if (size % 512)
                    Address += 512;
            }
        }
    }

    USTAR::USTAR() {}

    USTAR::~USTAR() {}
}
