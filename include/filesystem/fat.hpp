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

#ifndef __FENNIX_KERNEL_FILESYSTEM_FAT_H__
#define __FENNIX_KERNEL_FILESYSTEM_FAT_H__

#include <types.h>

#include <filesystem.hpp>

namespace vfs
{
	class FAT
	{
	public:
		enum FatType
		{
			Unknown,
			FAT12,
			FAT16,
			FAT32
		};

		/* https://wiki.osdev.org/FAT */
		struct BIOSParameterBlock
		{
			/** The first three bytes EB 3C 90 disassemble to JMP SHORT 3C NOP.
			 * (The 3C value may be different.) The reason for this is to jump
			 * over the disk format information (the BPB and EBPB). Since the
			 * first sector of the disk is loaded into ram at location
			 * 0x0000:0x7c00 and executed, without this jump, the processor
			 * would attempt to execute data that isn't code. Even for
			 * non-bootable volumes, code matching this pattern (or using the
			 * E9 jump opcode) is required to be present by both Windows and
			 * OS X. To fulfil this requirement, an infinite loop can be placed
			 * here with the bytes EB FE 90. */
			uint8_t JumpBoot[3];

			/** OEM identifier. The first 8 Bytes (3 - 10) is the version of DOS
			 * being used. The next eight Bytes 29 3A 63 7E 2D 49 48 and 43 read
			 * out the name of the version. The official FAT Specification from
			 * Microsoft says that this field is really meaningless and is ignored
			 * by MS FAT Modules, however it does recommend the value "MSWIN4.1"
			 * as some 3rd party drivers supposedly check it and expect it to
			 * have that value. Older versions of dos also report MSDOS5.1,
			 * linux-formatted floppy will likely to carry "mkdosfs" here, and
			 * FreeDOS formatted disks have been observed to have "FRDOS5.1" here.
			 * If the string is less than 8 bytes, it is padded with spaces. */
			uint8_t OEM[8];

			/** The number of Bytes per sector (remember, all numbers are in the
			 * little-endian format). */
			uint16_t BytesPerSector;

			/** Number of sectors per cluster. */
			uint8_t SectorsPerCluster;

			/** Number of reserved sectors. The boot record sectors are included
			 * in this value. */
			uint16_t ReservedSectors;

			/** Number of File Allocation Tables (FAT's) on the storage media.
			 * Often this value is 2. */
			uint8_t NumberOfFATs;

			/** Number of root directory entries (must be set so that the root
			 * directory occupies entire sectors). */
			uint16_t RootDirectoryEntries;

			/** The total sectors in the logical volume. If this value is 0, it
			 * means there are more than 65535 sectors in the volume, and the
			 * actual count is stored in the Large Sector Count entry at 0x20. */
			uint16_t Sectors16;

			/** This Byte indicates the media descriptor type. */
			uint8_t Media;

			/** Number of sectors per FAT. FAT12/FAT16 only. */
			uint16_t SectorsPerFAT;

			/** Number of sectors per track. */
			uint16_t SectorsPerTrack;

			/** Number of heads or sides on the storage media. */
			uint16_t NumberOfHeads;

			/** Number of hidden sectors. (i.e. the LBA of the beginning of
			 * the partition). */
			uint32_t HiddenSectors;

			/** Large sector count. This field is set if there are more than
			 * 65535 sectors in the volume, resulting in a value which does not
			 * fit in the Number of Sectors entry at 0x13. */
			uint32_t Sectors32;
		} __packed;

		FatType GetFATType(BIOSParameterBlock *bpb);
		FAT(void *partition);
		~FAT();
	};
}

#endif // !__FENNIX_KERNEL_FILESYSTEM_FAT_H__
