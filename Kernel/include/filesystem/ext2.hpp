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

#ifndef __FENNIX_KERNEL_FILESYSTEM_EXT2_H__
#define __FENNIX_KERNEL_FILESYSTEM_EXT2_H__

#include <types.h>

#include <filesystem.hpp>

namespace vfs
{
	class EXT2
	{
	public:
		struct SuperBlock
		{
			uint32_t Inodes;
			uint32_t Blocks;
			uint32_t ReservedBlocks;
			uint32_t FreeBlock;
			uint32_t FreeInodes;
			uint32_t FirstDataBlock;
			uint32_t LogBlockSize;
			uint32_t LogFragSize;
			uint32_t BlocksPerGroup;
			uint32_t FragsPerGroup;
			uint32_t InodesPerGroup;
			uint32_t LastMountTime;
			uint32_t LastWrittenTime;
			uint16_t MountedTimes;
			uint16_t MaximumMountedTimes;
			uint16_t Magic;
			uint16_t State;
			uint16_t Errors;
			uint16_t MinorRevLevel;
			uint32_t LastCheck;
			uint32_t CheckInternval;
			uint32_t SystemID;
			uint32_t RevLevel;
			uint16_t ReservedBlocksUserID;
			uint16_t ReservedBlocksGroupID;

			uint32_t FirstInode;
			uint16_t InodeSize;
			uint16_t BlockGroups;
			uint32_t FeatureCompatibility;
			uint32_t FeatureIncompatibility;
			uint32_t FeatureRoCompatibility;
			uint8_t UUID[16];
			char VolumeName[16];
			char LastMounted[64];
			uint32_t BitmapAlogrithm;

			uint8_t PreallocatedBlocks;
			uint8_t PreallocatedDirectoryBlocks;

			uint16_t Padding;
			uint8_t JournalUUID[16];
			uint32_t JournalInum;
			uint32_t JournalDev;
			uint32_t LastOrphan;
			uint32_t HashSeed[4];
			uint8_t DefHashVersion;
			uint8_t ReservedCharPad;
			uint16_t ReservedWordPad;
			uint32_t DefaultMountOptions;
			uint32_t FirstMetaBg;
			uint32_t Reserved[190];
		};

		EXT2(void *partition);
		~EXT2();
	};
}

#endif // !__FENNIX_KERNEL_FILESYSTEM_EXT2_H__
