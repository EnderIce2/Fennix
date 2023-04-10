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
        DriverManager->IOCB(DriverUID, (void *)&callback);
        this->AvailablePorts = callback.DiskCallback.Fetch.Ports;
        this->BytesPerSector = callback.DiskCallback.Fetch.BytesPerSector;
        debug("AvailablePorts:%ld BytesPerSector:%ld", this->AvailablePorts, this->BytesPerSector);

        if (this->AvailablePorts <= 0)
            return;

        uint8_t *RWBuffer = (uint8_t *)KernelAllocator.RequestPages(TO_PAGES(this->BytesPerSector + 1));

        for (unsigned char ItrPort = 0; ItrPort < this->AvailablePorts; ItrPort++)
        {
            Drive *drive = new Drive;
            sprintf(drive->Name, "sd%ld-%d", DriverUID, this->AvailablePorts);
            debug("Drive Name: %s", drive->Name);
            // TODO: Implement disk type detection. Very useful in the future.
            drive->MechanicalDisk = true;

            memset(RWBuffer, 0, this->BytesPerSector);
            callback.Reason = ReceiveReason;
            callback.DiskCallback.RW = {
                .Sector = 0,
                .SectorCount = 2,
                .Port = ItrPort,
                .Buffer = RWBuffer,
                .Write = false,
            };
            DriverManager->IOCB(DriverUID, (void *)&callback);
            memcpy(&drive->Table, RWBuffer, sizeof(PartitionTable));

            /*
            TODO: Add to devfs the disk
            */

            if (drive->Table.GPT.Signature == GPT_MAGIC)
            {
                drive->Style = GPT;
                uint32_t Entries = 512 / drive->Table.GPT.EntrySize;
                uint32_t Sectors = drive->Table.GPT.PartCount / Entries;
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
                    DriverManager->IOCB(DriverUID, (void *)&callback);

                    for (uint32_t e = 0; e < Entries; e++)
                    {
                        GUIDPartitionTablePartition GPTPartition = reinterpret_cast<GUIDPartitionTablePartition *>(RWBuffer)[e];
                        if (GPTPartition.TypeLow || GPTPartition.TypeHigh)
                        {
                            Partition *partition = new Partition;
                            memcpy(partition->Label, GPTPartition.Label, sizeof(partition->Label));
                            partition->StartLBA = GPTPartition.StartLBA;
                            partition->EndLBA = GPTPartition.EndLBA;
                            partition->Sectors = partition->EndLBA - partition->StartLBA;
                            partition->Port = ItrPort;
                            partition->Flags = Present;
                            partition->Style = GPT;
                            if (GPTPartition.Attributes & 1)
                                partition->Flags |= EFISystemPartition;
                            partition->Index = drive->Partitions.size();
                            // why there is NUL (\0) between every char?????
                            char PartName[72];
                            memcpy(PartName, GPTPartition.Label, 72);
                            for (int i = 0; i < 72; i++)
                                if (PartName[i] == '\0')
                                    PartName[i] = ' ';
                            PartName[71] = '\0';
                            trace("GPT partition \"%s\" found with %lld sectors", PartName, partition->Sectors);
                            drive->Partitions.push_back(partition);

                            char *PartitionName = new char[64];
                            sprintf(PartitionName, "sd%ldp%ld", drives.size() - 1, partition->Index);

                            /*
                            TODO: Add to devfs the disk
                            */

                            delete[] PartitionName;
                        }
                    }
                }
                trace("%d GPT partitions found.", drive->Partitions.size());
            }
            else if (drive->Table.MBR.Signature[0] == MBR_MAGIC0 && drive->Table.MBR.Signature[1] == MBR_MAGIC1)
            {
                drive->Style = MBR;
                for (size_t p = 0; p < 4; p++)
                    if (drive->Table.MBR.Partitions[p].LBAFirst != 0)
                    {
                        Partition *partition = new Partition;
                        partition->StartLBA = drive->Table.MBR.Partitions[p].LBAFirst;
                        partition->EndLBA = drive->Table.MBR.Partitions[p].LBAFirst + drive->Table.MBR.Partitions[p].Sectors;
                        partition->Sectors = drive->Table.MBR.Partitions[p].Sectors;
                        partition->Port = ItrPort;
                        partition->Flags = Present;
                        partition->Style = MBR;
                        partition->Index = drive->Partitions.size();
                        trace("Partition \"%#llx\" found with %lld sectors.", drive->Table.MBR.UniqueID, partition->Sectors);
                        drive->Partitions.push_back(partition);

                        char *PartitionName = new char[64];
                        sprintf(PartitionName, "sd%ldp%ld", drives.size() - 1, partition->Index);

                        /*
                        TODO: Add to devfs the disk
                        */

                        delete[] PartitionName;
                    }
                trace("%d MBR partitions found.", drive->Partitions.size());
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
