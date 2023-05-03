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

#include <memory.hpp>
#include <debug.h>

#include "../../kernel.h"

namespace Memory
{
    ReadFSFunction(MEM_Read)
    {
        if (!Size)
            Size = node->Length;
        if (Offset > node->Length)
            return 0;
        if (Offset + Size > node->Length)
            Size = node->Length - Offset;
        memcpy(Buffer, (uint8_t *)(node->Address + Offset), Size);
        return Size;
    }

    WriteFSFunction(MEM_Write)
    {
        if (!Size)
            Size = node->Length;
        if (Offset > node->Length)
            return 0;
        if (Offset + Size > node->Length)
            Size = node->Length - Offset;
        memcpy((uint8_t *)(node->Address + Offset), Buffer, Size);
        return Size;
    }

    VirtualFileSystem::FileSystemOperations mem_op = {
        .Name = "mem",
        .Read = MEM_Read,
        .Write = MEM_Write,
    };

    uint64_t MemMgr::GetAllocatedMemorySize()
    {
        uint64_t Size = 0;
        foreach (auto ap in AllocatedPagesList)
            Size += ap.PageCount;
        return FROM_PAGES(Size);
    }

    bool MemMgr::Add(void *Address, size_t Count)
    {
        if (Address == nullptr)
        {
            error("Address is null!");
            return false;
        }

        if (Count == 0)
        {
            error("Count is 0!");
            return false;
        }

        for (size_t i = 0; i < AllocatedPagesList.size(); i++)
        {
            if (AllocatedPagesList[i].Address == Address)
            {
                error("Address already exists!");
                return false;
            }
            else if ((uintptr_t)Address < (uintptr_t)AllocatedPagesList[i].Address)
            {
                if ((uintptr_t)Address + (Count * PAGE_SIZE) > (uintptr_t)AllocatedPagesList[i].Address)
                {
                    error("Address intersects with an allocated page!");
                    return false;
                }
            }
            else
            {
                if ((uintptr_t)AllocatedPagesList[i].Address + (AllocatedPagesList[i].PageCount * PAGE_SIZE) > (uintptr_t)Address)
                {
                    error("Address intersects with an allocated page!");
                    return false;
                }
            }
        }

        if (this->Directory)
        {
            char FileName[64];
            sprintf(FileName, "%lx-%ld", (uintptr_t)Address, Count);
            VirtualFileSystem::Node *n = vfs->Create(FileName, VirtualFileSystem::NodeFlags::FILE, this->Directory);
            if (n)
            {
                n->Address = (uintptr_t)Address;
                n->Length = Count * PAGE_SIZE;
                n->Operator = &mem_op;
            }
        }

        AllocatedPagesList.push_back({Address, Count});
        return true;
    }

    void *MemMgr::RequestPages(size_t Count, bool User)
    {
        void *Address = KernelAllocator.RequestPages(Count);
        for (size_t i = 0; i < Count; i++)
        {
            int Flags = Memory::PTFlag::RW;
            if (User)
                Flags |= Memory::PTFlag::US;

            Memory::Virtual(this->Table).Remap((void *)((uintptr_t)Address + (i * PAGE_SIZE)), (void *)((uint64_t)Address + (i * PAGE_SIZE)), Flags);
        }

        if (this->Directory)
        {
            char FileName[64];
            sprintf(FileName, "%lx-%ld", (uintptr_t)Address, Count);
            VirtualFileSystem::Node *n = vfs->Create(FileName, VirtualFileSystem::NodeFlags::FILE, this->Directory);
            if (n) // If null, error or file already exists
            {
                n->Address = (uintptr_t)Address;
                n->Length = Count * PAGE_SIZE;
                n->Operator = &mem_op;
            }
        }

        AllocatedPagesList.push_back({Address, Count});

        /* For security reasons, we clear the allocated page
           if it's a user page. */
        if (User)
            memset(Address, 0, Count * PAGE_SIZE);

        return Address;
    }

    void MemMgr::FreePages(void *Address, size_t Count)
    {
        for (size_t i = 0; i < AllocatedPagesList.size(); i++)
        {
            if (AllocatedPagesList[i].Address == Address)
            {
                /** TODO: Advanced checks. Allow if the page count is less than the requested one.
                 * This will allow the user to free only a part of the allocated pages.
                 *
                 * But this will be in a separate function because we need to specify if we
                 * want to free from the start or from the end and return the new address.
                 */
                if (AllocatedPagesList[i].PageCount != Count)
                {
                    error("Page count mismatch! (Allocated: %lld, Requested: %lld)", AllocatedPagesList[i].PageCount, Count);
                    return;
                }

                KernelAllocator.FreePages(Address, Count);

                for (size_t i = 0; i < Count; i++)
                {
                    Memory::Virtual(this->Table).Remap((void *)((uintptr_t)Address + (i * PAGE_SIZE)), (void *)((uint64_t)Address + (i * PAGE_SIZE)), Memory::PTFlag::RW);
                    // Memory::Virtual(this->Table).Unmap((void *)((uintptr_t)Address + (i * PAGE_SIZE)));
                }

                if (this->Directory)
                {
                    char FileName[64];
                    sprintf(FileName, "%lx-%ld", (uintptr_t)Address, Count);
                    VirtualFileSystem::FileStatus s = vfs->Delete(FileName, false, this->Directory);
                    if (s != VirtualFileSystem::FileStatus::OK)
                        error("Failed to delete file %s", FileName);
                }

                AllocatedPagesList.remove(i);
                return;
            }
        }
    }

    void MemMgr::DetachAddress(void *Address)
    {
        for (size_t i = 0; i < AllocatedPagesList.size(); i++)
        {
            if (AllocatedPagesList[i].Address == Address)
            {
                if (this->Directory)
                {
                    char FileName[64];
                    sprintf(FileName, "%lx-%ld", (uintptr_t)Address, AllocatedPagesList[i].PageCount);
                    VirtualFileSystem::FileStatus s = vfs->Delete(FileName, false, this->Directory);
                    if (s != VirtualFileSystem::FileStatus::OK)
                        error("Failed to delete file %s", FileName);
                }

                AllocatedPagesList.remove(i);
                return;
            }
        }
    }

    MemMgr::MemMgr(PageTable *Table, VirtualFileSystem::Node *Directory)
    {
        if (Table)
            this->Table = Table;
        else
        {
#if defined(a64)
            this->Table = (PageTable *)CPU::x64::readcr3().raw;
#elif defined(a32)
            this->Table = (PageTable *)CPU::x32::readcr3().raw;
#endif
        }

        this->Directory = Directory;
        debug("+ %#lx", this);
    }

    MemMgr::~MemMgr()
    {
        foreach (auto ap in AllocatedPagesList)
        {
            KernelAllocator.FreePages(ap.Address, ap.PageCount);
            for (size_t i = 0; i < ap.PageCount; i++)
                Memory::Virtual(this->Table).Remap((void *)((uintptr_t)ap.Address + (i * PAGE_SIZE)), (void *)((uintptr_t)ap.Address + (i * PAGE_SIZE)), Memory::PTFlag::RW);
        }

        if (this->Directory)
        {
            foreach (auto Child in this->Directory->Children)
                vfs->Delete(Child, true);
        }

        debug("- %#lx", this);
    }
}
