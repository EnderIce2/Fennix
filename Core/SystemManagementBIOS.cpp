#include "smbios.hpp"

#include <debug.h>

#include "../kernel.h"

/* https://www.dmtf.org/sites/default/files/standards/documents/DSP0134_3.2.0.pdf */

namespace SMBIOS
{
    bool CheckSMBIOS()
    {
        if (bInfo->SMBIOSPtr != nullptr && bInfo->SMBIOSPtr < (void *)0xffffffffffff0000)
        {
            debug("SMBIOS is available (%#lx).", bInfo->SMBIOSPtr);
            return true;
        }
        debug("SMBIOS is not available. (%#lx)", bInfo->SMBIOSPtr);
        return false;
    }

    SMBIOSEntryPoint *GetSMBIOSEntryPoint() { return (SMBIOSEntryPoint *)bInfo->SMBIOSPtr; }

    static inline int SMBIOSTableLength(SMBIOSHeader *Hdr)
    {
        int i;
        const char *strtab = (char *)Hdr + Hdr->Length;
        for (i = 1; strtab[i - 1] != '\0' || strtab[i] != '\0'; i++)
            ;
        return Hdr->Length + i + 1;
    }

    void *GetSMBIOSHeader(SMBIOSType Type)
    {
        if (!CheckSMBIOS())
            return nullptr;

        SMBIOSEntryPoint *Header = (SMBIOSEntryPoint *)bInfo->SMBIOSPtr;
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
