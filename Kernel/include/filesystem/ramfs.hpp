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

#pragma once

#include <filesystem.hpp>
#include <memory.hpp>

namespace vfs
{
	class RAMFS
	{
	public:
		class InodeBuffer
		{
		public:
			void *Data = nullptr;
			size_t DataSize = 0;

			void Allocate(size_t size, bool extend = false, bool atEnd = true)
			{
				if (extend == false)
				{
					if (Data)
						Free();
					Data = kmalloc(size);
					if (!Data)
						throw std::bad_alloc();
					DataSize = size;
				}
				else
				{
					if (Data == nullptr)
					{
						Data = kmalloc(size);
						if (!Data)
							throw std::bad_alloc();
						DataSize = size;
					}
					else
					{
						size_t newSize = DataSize + size;
						void *newData = kmalloc(newSize);
						if (!newData)
							throw std::bad_alloc();

						if (atEnd)
							memcpy(newData, Data, DataSize);
						else
							memcpy(static_cast<char *>(newData) + size, Data, DataSize);

						kfree(Data);
						Data = newData;
						DataSize = newSize;
					}
				}
			}

			void Free()
			{
				if (Data)
				{
					kfree(Data);
					Data = nullptr;
					DataSize = 0;
				}
			}

			bool IsAllocated() const
			{
				return Data != nullptr;
			}

			InodeBuffer() = default;
			~InodeBuffer() { Free(); }
		};

		class RAMFSInode
		{
		public:
			struct Inode Node;
			RAMFSInode *Parent = nullptr;
			std::string Name;
			kstat Stat{};
			mode_t Mode = 0;
			InodeBuffer Buffer;
			std::string SymLink;
			std::vector<RAMFSInode *> Children;

			void AddChild(RAMFSInode *child)
			{
				Children.push_back(child);
				child->Parent = this;
			}

			void RemoveChild(RAMFSInode *child)
			{
				auto it = std::find(Children.begin(), Children.end(), child);
				if (it != Children.end())
				{
					Children.erase(it);
					child->Parent = nullptr;
				}
			}

			RAMFSInode() = default;
			~RAMFSInode()
			{
				for (auto child : Children)
					delete child;
			}
		};

	private:
		std::unordered_map<ino_t, RAMFSInode *> Files;

	public:
		dev_t DeviceID = -1;
		ino_t NextInode = 0;
		std::string RootName;

		int Lookup(struct Inode *Parent, const char *Name, struct Inode **Result);
		int Create(struct Inode *Parent, const char *Name, mode_t Mode, struct Inode **Result);
		ssize_t Read(struct Inode *Node, void *Buffer, size_t Size, off_t Offset);
		ssize_t Write(struct Inode *Node, const void *Buffer, size_t Size, off_t Offset);
		ssize_t ReadDir(struct Inode *Node, struct kdirent *Buffer, size_t Size, off_t Offset, off_t Entries);
		int SymLink(struct Inode *Node, const char *Name, const char *Target, struct Inode **Result);
		ssize_t ReadLink(struct Inode *Node, char *Buffer, size_t Size);
		int Stat(struct Inode *Node, struct kstat *Stat);

		RAMFS() = default;
		~RAMFS() = default;
	};
}

bool MountRAMFS(FileNode *Parent, const char *Name, size_t Index);
