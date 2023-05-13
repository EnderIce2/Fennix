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

#include <disk.hpp>

#include <memory.hpp>
#include <printf.h>

#include "../kernel.h"
#include "../DAPI.hpp"
#include "../Fex.hpp"

namespace Disk
{
    void Manager::FetchDisks(unsigned long DriverUID)
    {
        KernelCallback callback{};
        callback.Reason = FetchReason;
        DriverManager->IOCB(DriverUID, &callback);
        this->AvailablePorts = callback.DiskCallback.Fetch.Ports;
        this->BytesPerSector = callback.DiskCallback.Fetch.BytesPerSector;
        debug("AvailablePorts:%ld BytesPerSector:%ld", this->AvailablePorts, this->BytesPerSector);

        if (this->AvailablePorts <= 0)
            return;

        uint8_t *RWBuffer = (uint8_t *)KernelAllocator.RequestPages(TO_PAGES(this->BytesPerSector + 1));

        for (unsigned char ItrPort = 0; ItrPort < this->AvailablePorts; ItrPort++)
        {
            Drive drive{};
            sprintf(drive.Name, "sd%ld", DriverUID);
            debug("Drive Name: %s", drive.Name);
            // TODO: Implement disk type detection. Very useful in the future.
            drive.MechanicalDisk = true;

            memset(RWBuffer, 0, this->BytesPerSector);
            callback.Reason = ReceiveReason;
            callback.DiskCallback.RW = {
                .Sector = 0,
                .SectorCount = 2,
                .Port = ItrPort,
                .Buffer = RWBuffer,
                .Write = false,
            };
            DriverManager->IOCB(DriverUID, &callback);
            memcpy(&drive.Table, RWBuffer, sizeof(PartitionTable));

            /*
            TODO: Add to devfs the disk
            */

            if (drive.Table.GPT.Signature == GPT_MAGIC)
            {
                drive.Style = GPT;
                uint32_t Entries = 512 / drive.Table.GPT.EntrySize;
                uint32_t Sectors = drive.Table.GPT.PartCount / Entries;
                for (uint32_t Block = 0; Block < Sectors; Block++)
                {
                    memset(RWBuffer, 0, this->BytesPerSector);
                    callback.Reason = ReceiveReason;
                    callback.DiskCallback.RW = {
                        .Sector = 2 + Block,
                        .SectorCount = 1,
                        .Port = ItrPort,
                        .Buffer = RWBuffer,
                        .Write = false,
                    };
                    DriverManager->IOCB(DriverUID, &callback);

                    for (uint32_t e = 0; e < Entries; e++)
                    {
                        GUIDPartitionTableEntry GPTPartition = reinterpret_cast<GUIDPartitionTableEntry *>(RWBuffer)[e];
                        if (memcmp(GPTPartition.PartitionType, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", sizeof(GPTPartition.PartitionType)) != 0)
                        {
                            Partition partition{};
                            memset(partition.Label, '\0', sizeof(partition.Label));
                            // Convert utf16 to utf8
                            for (int i = 0; i < 36; i++)
                            {
                                uint16_t utf16 = GPTPartition.PartitionName[i];
                                if (utf16 == 0)
                                    break;
                                if (utf16 < 0x80)
                                    partition.Label[i] = (char)utf16;
                                else if (utf16 < 0x800)
                                {
                                    partition.Label[i] = (char)(0xC0 | (utf16 >> 6));
                                    partition.Label[i + 1] = (char)(0x80 | (utf16 & 0x3F));
                                    i++;
                                }
                                else
                                {
                                    partition.Label[i] = (char)(0xE0 | (utf16 >> 12));
                                    partition.Label[i + 1] = (char)(0x80 | ((utf16 >> 6) & 0x3F));
                                    partition.Label[i + 2] = (char)(0x80 | (utf16 & 0x3F));
                                    i += 2;
                                }
                            }
                            partition.StartLBA = GPTPartition.FirstLBA;
                            partition.EndLBA = GPTPartition.LastLBA;
                            partition.Sectors = partition.EndLBA - partition.StartLBA;
                            partition.Port = ItrPort;
                            partition.Flags = Present;
                            partition.Style = GPT;
                            if (GPTPartition.Attributes & 1)
                                partition.Flags |= EFISystemPartition;
                            partition.Index = drive.Partitions.size();
                            trace("GPT partition \"%s\" found with %lld sectors", partition.Label, partition.Sectors);
                            drive.Partitions.push_back(partition);

                            char PartitionName[64];
                            sprintf(PartitionName, "sd%ldp%ld", drives.size(), partition.Index);
                            fixme("PartitionName: %s", PartitionName);

                            /*
                            TODO: Add to devfs the disk
                            */
                        }
                    }
                }
                trace("%d GPT partitions found.", drive.Partitions.size());
            }
            else if (drive.Table.MBR.Signature[0] == MBR_MAGIC0 && drive.Table.MBR.Signature[1] == MBR_MAGIC1)
            {
                drive.Style = MBR;
                for (size_t p = 0; p < 4; p++)
                    if (drive.Table.MBR.Partitions[p].LBAFirst != 0)
                    {
                        Partition partition{};
                        partition.StartLBA = drive.Table.MBR.Partitions[p].LBAFirst;
                        partition.EndLBA = drive.Table.MBR.Partitions[p].LBAFirst + drive.Table.MBR.Partitions[p].Sectors;
                        partition.Sectors = drive.Table.MBR.Partitions[p].Sectors;
                        partition.Port = ItrPort;
                        partition.Flags = Present;
                        partition.Style = MBR;
                        partition.Index = drive.Partitions.size();
                        trace("Partition \"%#llx\" found with %lld sectors.", drive.Table.MBR.UniqueID, partition.Sectors);
                        drive.Partitions.push_back(partition);

                        char PartitionName[64];
                        sprintf(PartitionName, "sd%ldp%ld", drives.size(), partition.Index);
                        fixme("PartitionName: %s", PartitionName);

                        /*
                        TODO: Add to devfs the disk
                        */
                    }
                trace("%d MBR partitions found.", drive.Partitions.size());
            }
            else
                warn("No partition table found on port %d!", ItrPort);

            drives.push_back(drive);
        }

        KernelAllocator.FreePages(RWBuffer, TO_PAGES(this->BytesPerSector + 1));
    }

    Manager::Manager()
    {
    }

    Manager::~Manager()
    {
        debug("Destructor called");
    }
}
