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

#include "smbios.hpp"

#include <debug.h>

#include "../kernel.h"

namespace SMBIOS
{
    bool CheckSMBIOS()
    {
        if (bInfo.SMBIOSPtr != nullptr && bInfo.SMBIOSPtr < (void *)0xFFFFFFFFFFFF0000)
        {
            debug("SMBIOS is available (%#lx).", bInfo.SMBIOSPtr);
            return true;
        }
        debug("SMBIOS is not available. (%#lx)", bInfo.SMBIOSPtr);
        return false;
    }

    SMBIOSEntryPoint *GetSMBIOSEntryPoint() { return (SMBIOSEntryPoint *)bInfo.SMBIOSPtr; }

    __no_sanitize("alignment") static inline int SMBIOSTableLength(SMBIOSHeader *Hdr)
    {
        int i;
        const char *strtab = (char *)Hdr + Hdr->Length;
        for (i = 1; strtab[i - 1] != '\0' || strtab[i] != '\0'; i++)
            ;
        return Hdr->Length + i + 1;
    }

    __no_sanitize("alignment") void *GetSMBIOSHeader(SMBIOSType Type)
    {
        if (!CheckSMBIOS())
            return nullptr;

        SMBIOSEntryPoint *Header = (SMBIOSEntryPoint *)bInfo.SMBIOSPtr;
        debug("Getting SMBIOS header for type %d", Type);

        struct SMBIOSHeader *hdr = (SMBIOSHeader *)(uintptr_t)Header->TableAddress;
        for (int i = 0; i <= 11; i++)
        {
            if (hdr < (void *)(uintptr_t)(Header->TableAddress + Header->TableLength))
                if (hdr->Type == Type)
                {
                    debug("Found SMBIOS header for type %d at %#lx", Type, hdr);
                    return hdr;
                }
            hdr = (struct SMBIOSHeader *)((uintptr_t)hdr + SMBIOSTableLength(hdr));
        }
        return nullptr;
    }

    SMBIOSBIOSInformation *GetBIOSInformation() { return (SMBIOSBIOSInformation *)GetSMBIOSHeader(SMBIOSTypeBIOSInformation); }

    SMBIOSSystemInformation *GetSystemInformation() { return (SMBIOSSystemInformation *)GetSMBIOSHeader(SMBIOSTypeSystemInformation); }

    SMBIOSBaseBoardInformation *GetBaseBoardInformation() { return (SMBIOSBaseBoardInformation *)GetSMBIOSHeader(SMBIOSTypeBaseBoardInformation); }

    SMBIOSProcessorInformation *GetProcessorInformation() { return (SMBIOSProcessorInformation *)GetSMBIOSHeader(SMBIOSTypeProcessorInformation); }

    SMBIOSMemoryArray *GetMemoryArray() { return (SMBIOSMemoryArray *)GetSMBIOSHeader(SMBIOSTypePhysicalMemoryArray); }

    SMBIOSMemoryDevice *GetMemoryDevice() { return (SMBIOSMemoryDevice *)GetSMBIOSHeader(SMBIOSTypeMemoryDevice); }

    SMBIOSMemoryArrayMappedAddress *GetMemoryArrayMappedAddress() { return (SMBIOSMemoryArrayMappedAddress *)GetSMBIOSHeader(SMBIOSTypeMemoryArrayMappedAddress); }

    SMBIOSMemoryDeviceMappedAddress *GetMemoryDeviceMappedAddress() { return (SMBIOSMemoryDeviceMappedAddress *)GetSMBIOSHeader(SMBIOSTypeMemoryDeviceMappedAddress); }
}
