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

#ifndef __FENNIX_KERNEL_FILESYSTEM_USTAR_H__
#define __FENNIX_KERNEL_FILESYSTEM_USTAR_H__

#include <filesystem.hpp>

namespace vfs
{
	class USTAR
	{
	public:
		enum TypeFlag
		{
			AREGTYPE = '\0',
			REGTYPE = '0',
			LNKTYPE = '1',
			SYMTYPE = '2',
			CHRTYPE = '3',
			BLKTYPE = '4',
			DIRTYPE = '5',
			FIFOTYPE = '6',
			CONTTYPE = '7'
		};

		enum ModeFlag
		{
			TSUID = 04000,
			TSGID = 02000,
			TSVTX = 01000,
			TUREAD = 00400,
			TUWRITE = 00200,
			TUEXEC = 00100,
			TGREAD = 00040,
			TGWRITE = 00020,
			TGEXEC = 00010,
			TOREAD = 00004,
			TOWRITE = 00002,
			TOEXEC = 00001,
		};

		struct FileHeader
		{
			char name[100];
			char mode[8];
			char uid[8];
			char gid[8];
			char size[12];
			char mtime[12];
			char chksum[8];
			char typeflag[1];
			char link[100];
			char signature[6];
			char version[2];
			char owner[32];
			char group[32];
			char dev_maj[8];
			char dev_min[8];
			char prefix[155];
			char pad[12];
		};

		constexpr static int INODE_CHECKSUM = 0x7757A4;

		struct USTARInode
		{
			struct Inode Node;
			FileHeader *Header;
			USTARInode *Parent;
			std::string Name;
			std::string Path;
			std::vector<USTARInode *> Children;
			bool Deleted;
			int Checksum;
		};

	private:
		std::unordered_map<ino_t, USTARInode *> Files;

		inline uint32_t GetSize(const char *String)
		{
			uint32_t ret = 0;
			while (*String)
			{
				ret *= 8;
				ret += *String - '0';
				String++;
			}
			return ret;
		}

		inline int StringToInt(const char *String)
		{
			int ret = 0;
			for (int i = 0; String[i] != '\0'; ++i)
				ret = ret * 10 + String[i] - '0';
			return ret;
		}

	public:
		dev_t DeviceID = -1;
		ino_t NextInode = 0;

		int Lookup(struct Inode *Parent, const char *Name, struct Inode **Result);
		int Create(struct Inode *Parent, const char *Name, mode_t Mode, struct Inode **Result);
		ssize_t Read(struct Inode *Node, void *Buffer, size_t Size, off_t Offset);
		ssize_t ReadDir(struct Inode *Node, struct kdirent *Buffer, size_t Size, off_t Offset, off_t Entries);
		int SymLink(struct Inode *Node, const char *Name, const char *Target, struct Inode **Result);
		ssize_t ReadLink(struct Inode *Node, char *Buffer, size_t Size);
		int Stat(struct Inode *Node, struct kstat *Stat);

		bool TestArchive(uintptr_t Address);
		void ReadArchive(uintptr_t Address, size_t Size);

		USTAR() = default;
		~USTAR() = default;
	};
}

bool TestAndInitializeUSTAR(uintptr_t Address, size_t Size, size_t Index);

#endif // !__FENNIX_KERNEL_FILESYSTEM_USTAR_H__
