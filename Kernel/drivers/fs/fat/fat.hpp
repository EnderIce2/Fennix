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

#include <types.h>

/* Source: https://wiki.osdev.org/FAT */

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

struct ExtendedBootRecord_FAT12_16
{
	/** Drive number. The value here should be identical to the value
	 * returned by BIOS interrupt 0x13, or passed in the DL register;
	 * i.e. 0x00 for a floppy disk and 0x80 for hard disks. This number
	 * is useless because the media is likely to be moved to another
	 * machine and inserted in a drive with a different drive number. */
	uint8_t DriveNumber;

	/** Flags in Windows NT. Reserved otherwise. */
	uint8_t Flags;

	/** Signature (must be 0x28 or 0x29). */
	uint8_t Signature;

	/** VolumeID 'Serial' number. Used for tracking volumes between
	 * computers. You can ignore this if you want. */
	uint32_t VolumeID;

	/** Volume label string. This field is padded with spaces. */
	uint8_t VolumeLabel[11];

	/** System identifier string. This field is a string representation
	 * of the FAT file system type. It is padded with spaces. The spec
	 * says never to trust the contents of this string for any use. */
	uint8_t SystemIdentifier[8];

	/** Boot code. */
	uint8_t BootCode[448];

	/** Bootable partition signature 0xAA55. */
	uint16_t BootSignature;
} __packed;

struct ExtendedBootRecord_FAT32
{
	/** Sectors per FAT. The size of the FAT in sectors. */
	uint32_t SectorsPerFAT;

	/** Flags. */
	uint16_t Flags;

	/** FAT version number. The high byte is the major version and the
	 * low byte is the minor version. FAT drivers should respect this
	 * field. */
	uint16_t FATVersion;

	/** The cluster number of the root directory. Often this field is
	 * set to 2. */
	uint32_t RootDirectoryCluster;

	/** The sector number of the FSInfo structure. */
	uint16_t FSInfoSector;

	/** The sector number of the backup boot sector. */
	uint16_t BackupBootSector;

	/** Reserved. When the volume is formated these bytes should be zero. */
	uint8_t Reserved[12];

	/** Drive number. The values here are identical to the values returned
	 * by the BIOS interrupt 0x13. 0x00 for a floppy disk and 0x80 for
	 * hard disks. */
	uint8_t DriveNumber;

	/** Flags in Windows NT. Reserved otherwise. */
	uint8_t Flags2;

	/** Signature (must be 0x28 or 0x29). */
	uint8_t Signature;

	/** Volume ID 'Serial' number. Used for tracking volumes between
	 * computers. You can ignore this if you want. */
	uint32_t VolumeID;

	/** Volume label string. This field is padded with spaces. */
	uint8_t VolumeLabel[11];

	/** System identifier string. Always "FAT32   ". The spec says never
	 * to trust the contents of this string for any use. */
	uint8_t SystemIdentifier[8];

	/** Boot code. */
	uint8_t BootCode[420];

	/** Bootable partition signature 0xAA55. */
	uint16_t BootSignature;
} __packed;

struct FSInfo
{
	/** Lead signature (must be 0x41615252 to indicate a valid FSInfo
	 * structure). */
	uint32_t LeadSignature;

	/** Reserved, these bytes should never be used. */
	uint8_t Reserved1[480];

	/** Another signature (must be 0x61417272). */
	uint32_t AnotherSignature;

	/** Contains the last known free cluster count on the volume. If the
	 * value is 0xFFFFFFFF, then the free count is unknown and must be
	 * computed. However, this value might be incorrect and should at
	 * least be range checked (<= volume cluster count). */
	uint32_t FreeClusterCount;

	/** Indicates the cluster number at which the filesystem driver should
	 * start looking for available clusters. If the value is 0xFFFFFFFF,
	 * then there is no hint and the driver should start searching at 2.
	 * Typically this value is set to the last allocated cluster number.
	 * As the previous field, this value should be range checked. */
	uint32_t NextFreeCluster;

	/** Reserved. */
	uint8_t Reserved2[12];

	/** Trail signature (0xAA550000). */
	uint32_t TrailSignature;
} __packed;

struct exFATBootRecord
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

	/** OEM identifier. This contains the string "EXFAT ". Not to be
	 * used for filesystem determination, but it's a nice hint. */
	uint8_t OEM[8];

	/** Set to zero. This makes sure any FAT driver will not be able to
	 * load it. */
	uint8_t Reserved1[53];

	/** Partition offset. No idea why the partition itself would have
	 * this, but it's here. Might be wrong. Probably best to just ignore. */
	uint64_t PartitionOffset;

	/** Volume length. */
	uint64_t VolumeLength;

	/** FAT offset (in sectors) from start of partition. */
	uint32_t FATOffset;

	/** FAT length (in sectors). */
	uint32_t FATLength;

	/** Cluster heap offset (in sectors). */
	uint32_t ClusterHeapOffset;

	/** Cluster count. */
	uint32_t ClusterCount;

	/** Root directory cluster. Typically 4 (but just read this value). */
	uint32_t RootDirectoryCluster;

	/** Serial number of partition. */
	uint32_t SerialNumber;

	/** Filesystem revision. */
	uint16_t FilesystemRevision;

	/** Flags. */
	uint16_t Flags;

	/** Sector shift. */
	uint8_t SectorShift;

	/** Cluster shift. */
	uint8_t ClusterShift;

	/** Number of FATs. */
	uint8_t NumberOfFATs;

	/** Drive select. */
	uint8_t DriveSelect;

	/** Percentage in use. */
	uint8_t PercentageInUse;

	/** Reserved (set to 0). */
	uint8_t Reserved2[7];
} __packed;
