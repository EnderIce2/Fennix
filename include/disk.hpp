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

#ifndef __FENNIX_KERNEL_DISK_H__
#define __FENNIX_KERNEL_DISK_H__

#include <types.h>
#include <vector>

namespace Disk
{
#define MBR_MAGIC0 0x55
#define MBR_MAGIC1 0xAA

// "EFI PART"
#define GPT_MAGIC 0x5452415020494645ULL

    enum PartitionStyle
    {
        Unknown,
        MBR,
        GPT
    };

    enum PartitionFlags
    {
        Present,
        Bootable,
        EFISystemPartition
    };

    struct MasterBootRecordPartition
    {
        uint8_t Flags;
        uint8_t CHSFirst[3];
        uint8_t Type;
        uint8_t CHSLast[3];
        uint32_t LBAFirst;
        uint32_t Sectors;
    } __packed;

    struct MasterBootRecord
    {
        uint8_t Bootstrap[440];
        uint32_t UniqueID;
        uint16_t Reserved;
        MasterBootRecordPartition Partitions[4];
        uint8_t Signature[2];
    } __packed;

    struct GUIDPartitionTableEntry
    {
        uint8_t PartitionType[16];
        uint8_t UniquePartitionGUID[16];
        uint64_t FirstLBA;
        uint64_t LastLBA;
        uint64_t Attributes;
        uint16_t PartitionName[36];
    } __packed;

    struct GUIDPartitionTable
    {
        uint64_t Signature;
        uint32_t Revision;
        uint32_t HdrSize;
        uint32_t HdrCRC32;
        uint32_t Reserved;
        uint64_t LBA;
        uint64_t ALTLBA;
        uint64_t FirstBlock;
        uint64_t LastBlock;
        uint64_t GUIDLow;
        uint64_t GUIDHigh;
        uint64_t PartLBA;
        uint32_t PartCount;
        uint32_t EntrySize;
        uint32_t PartCRC32;
    } __packed;

    struct PartitionTable
    {
        MasterBootRecord MBR;
        GUIDPartitionTable GPT;
    };

    class Partition
    {
    public:
        char Label[72] = "Unidentified Partition";
        size_t StartLBA = 0xdeadbeef;
        size_t EndLBA = 0xdeadbeef;
        size_t Sectors = 0xdeadbeef;
        size_t Flags = 0xdeadbeef;
        unsigned char Port = 0;
        PartitionStyle Style = PartitionStyle::Unknown;
        size_t Index = 0;

        size_t Read(size_t Offset, size_t Count, uint8_t *Buffer)
        {
            return 0;
            UNUSED(Offset);
            UNUSED(Count);
            UNUSED(Buffer);
        }

        size_t Write(size_t Offset, size_t Count, uint8_t *Buffer)
        {
            return 0;
            UNUSED(Offset);
            UNUSED(Count);
            UNUSED(Buffer);
        }

        Partition() {}
        ~Partition() {}
    };

    class Drive
    {
    public:
        char Name[64] = "Unidentified Drive";
        uint8_t *Buffer = nullptr;
        PartitionTable Table;
        PartitionStyle Style = PartitionStyle::Unknown;
        std::vector<Partition> Partitions;
        bool MechanicalDisk = false;
        size_t UniqueIdentifier = 0xdeadbeef;

        size_t Read(size_t Offset, size_t Count, uint8_t *Buffer)
        {
            return 0;
            UNUSED(Offset);
            UNUSED(Count);
            UNUSED(Buffer);
        }

        size_t Write(size_t Offset, size_t Count, uint8_t *Buffer)
        {
            return 0;
            UNUSED(Offset);
            UNUSED(Count);
            UNUSED(Buffer);
        }

        Drive()
        { // TODO: Allocate buffer
        }
        ~Drive() {}
    };

    class Manager
    {
    private:
        unsigned char AvailablePorts = 0;
        int BytesPerSector = 0;

        std::vector<Drive> drives;

    public:
        void FetchDisks(unsigned long DriverUID);
        Manager();
        ~Manager();
    };
}

#endif // !__FENNIX_KERNEL_DISK_H__
