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

#include <fs/fdt.hpp>
#include <errno.h>
#include <cwalk.h>
#include <atomic>
#include <string>
#include <list>

/* sanity checks */
static_assert(DTTOIF(DT_FIFO) == S_IFIFO);
static_assert(IFTODT(S_IFCHR) == DT_CHR);

namespace vfs
{
	class Virtual
	{
	private:
		std::unordered_map<dev_t, Node> Roots;
		std::unordered_map<dev_t, FileSystemInfo *> FileSystems;

	public:
#pragma region Utilities

		inline bool PathIsRelative(const char *Path) { return cwk_path_is_relative(Path); }
		inline bool PathIsAbsolute(const char *Path) { return !PathIsRelative(Path); }
		inline bool PathIsRelative(std::string &Path) { return cwk_path_is_relative(Path.c_str()); }
		inline bool PathIsAbsolute(std::string &Path) { return !PathIsRelative(Path.c_str()); }

		eNode Convert(Node node) { return {node, 0}; }

		/**
		 * @brief Converts an Inode to a Node
		 *
		 * Result of this function will be empty.
		 * You are responsible to populate the Node.
		 *
		 * @param inode Inode to convert
		 * @return eNode{Node, errno}
		 */
		eNode Convert(Inode *inode);

		/**
		 * @brief Converts an Inode to a Node
		 *
		 * Result of this function will be populated using the Parent Node.
		 * This function will automatically assign the FSI, parents and children.
		 *
		 * @note Name and Path won't be set.
		 *
		 * @param Parent Parent Node
		 * @param inode Inode to convert
		 * @return eNode{Node, errno}
		 */
		eNode Convert(Node &Parent, Inode *inode);

		/**
		 * @brief Normalizes a path
		 *
		 * If the path is relative, it will be normalized using the Parent Node.
		 * Example if Join is true:
		 * 		Parent = /home/user/Desktop
		 * 		Path   = ../Documents
		 * 		Result = /home/user/Documents
		 *
		 * If Join is false:
		 * 		Path   = /var/foo/bar/../../
		 * 		Result = /var
		 *
		 * @param Parent Parent Node
		 * @param Path Path to normalize
		 * @param Join If true, the path will be joined with the Parent Node
		 * @return Normalized Path
		 */
		std::string NormalizePath(Node &Parent, std::string Path, bool Join = false);

#pragma endregion Utilities

#pragma region Roots

		bool RootExists(dev_t Index);
		eNode GetRoot(dev_t Index);
		ssize_t GetRoot(Node Index);
		int AddRoot(dev_t Index, Node Root, bool Replace = false);
		int AddRoot(dev_t Index, eNode Root, bool Replace = false) { return AddRoot(Index, Root.Value, Replace); }

#pragma endregion Roots

#pragma region Registrations

		dev_t RegisterFileSystem(FileSystemInfo *fsi);
		int UnregisterFileSystem(dev_t Device);

#pragma endregion Registrations

#pragma region Node Operations

		eNode Lookup(Node &Parent, std::string Path);

		eNode Create(Node &Parent, std::string Name, mode_t Mode, bool ErrorIfExists = true);

		int Remove(Node &Parent, std::string Name);
		int Remove(Node &node);

		int Rename(Node &node, std::string NewName);

		ssize_t Read(Node &Target, void *Buffer, size_t Size, off_t Offset);
		ssize_t Write(Node &Target, const void *Buffer, size_t Size, off_t Offset);

		int Truncate(Node &Target, off_t Size);

		/**
		 * @brief Read directory entries
		 *
		 * @note This function includes "." and ".."
		 *
		 * @param Target
		 * @param Buffer
		 * @param Size
		 * @param Offset
		 * @param Entries
		 * @return
		 */
		ssize_t ReadDirectory(Node &Target, kdirent *Buffer, size_t Size, off_t Offset, off_t Entries);

		/**
		 * @brief Read directory entries
		 *
		 * @note This function does NOT include "." and ".."
		 *
		 * @param Target
		 * @return
		 */
		std::list<Node> ReadDirectory(Node &Target);

		eNode CreateLink(Node &Parent, std::string Name, std::string Target);
		eNode CreateLink(Node &Parent, std::string Name, Node &Target) { return this->CreateLink(Parent, Name, Target->Path); }

		int Stat(Node &Target, kstat *Stat);
		int ReadLink(Node &Target, char *Buffer, size_t Size) { return Target->__ReadLink(Buffer, Size); }
		off_t Seek(Node &Target, off_t Offset);

		int Open(Node &Target, int Flags, mode_t Mode);
		int Close(Node &Target);
		int Ioctl(Node &Target, unsigned long Request, void *Argp) { return Target->__Ioctl(Request, Argp); }

#pragma endregion Node Operations

#pragma region Mounting

		FileSystemInfo *Probe(FileSystemDevice *Device);

		FileSystemInfo *Probe(Node &node)
		{
			FileSystemDevice dev;
			dev.inode.node = node->inode;
			dev.inode.ops = &node->fsi->Ops;
			dev.Block = nullptr;
			return this->Probe(&dev);
		}

		FileSystemInfo *Probe(BlockDevice *Device)
		{
			FileSystemDevice dev;
			dev.inode.node = nullptr;
			dev.inode.ops = nullptr;
			dev.Block = Device;
			return this->Probe(&dev);
		}

		/**
		 * @brief Mount a filesystem
		 *
		 *
		 *
		 * @param Parent Parent Node
		 * @param inode Inode from the filesystem as root of the mount point
		 * @param Name Name of the mount point
		 * @param fsi FileSystemInfo structure
		 * @return eNode{Node, errno}
		 */
		eNode Mount(Node &Parent, Inode *inode, std::string Name, FileSystemInfo *fsi);

		/**
		 * @brief Mount a filesystem
		 *
		 *
		 *
		 * @param Parent Parent Node
		 * @param Name Name of the mount point
		 * @param fsi FileSystemInfo structure
		 * @param Device Device to mount
		 * @return eNode{Node, errno}
		 */
		eNode Mount(Node &Parent, std::string Name, FileSystemInfo *fsi, FileSystemDevice *Device);

		/**
		 * Wrapper for Mount(Node &Parent, std::string Name, FileSystemInfo *fsi, FileSystemDevice *Device)
		 */
		eNode Mount(Node &Parent, std::string Name, FileSystemInfo *fsi, Node Device)
		{
			FileSystemDevice dev;
			dev.inode.node = Device->inode;
			dev.inode.ops = &Device->fsi->Ops;
			dev.Block = nullptr;
			return this->Mount(Parent, Name, fsi, &dev);
		}

		/**
		 * Wrapper for Mount(Node &Parent, std::string Name, FileSystemInfo *fsi, FileSystemDevice *Device)
		 */
		eNode Mount(Node &Parent, std::string Name, FileSystemInfo *fsi, BlockDevice *Device)
		{
			FileSystemDevice dev;
			dev.inode.node = nullptr;
			dev.inode.ops = nullptr;
			dev.Block = Device;
			return this->Mount(Parent, Name, fsi, &dev);
		}

		int Umount(Node &node);
		int Umount(Node &Parent, std::string Name);

#pragma endregion Mounting

		void Initialize();
		Virtual();
		~Virtual();
	};
}
