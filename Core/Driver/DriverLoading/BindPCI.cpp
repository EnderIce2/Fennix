#include "../api.hpp"

#include <ints.hpp>
#include <memory.hpp>
#include <task.hpp>
#include <lock.hpp>
#include <printf.h>
#include <cwalk.h>
#include <md5.h>

#include "../../../kernel.h"
#include "../../../DAPI.hpp"
#include "../../../Fex.hpp"

namespace Driver
{
    void Driver::MapPCIAddresses(PCI::PCIDeviceHeader *PCIDevice)
    {
        Memory::Virtual vma = Memory::Virtual(nullptr);

        debug("Header Type: %d", PCIDevice->HeaderType);
        switch (PCIDevice->HeaderType)
        {
        case 0: // PCI Header 0
        {
            uint32_t BAR[6] = {0};
            size_t BARsSize[6] = {0};

            BAR[0] = ((PCI::PCIHeader0 *)PCIDevice)->BAR0;
            BAR[1] = ((PCI::PCIHeader0 *)PCIDevice)->BAR1;
            BAR[2] = ((PCI::PCIHeader0 *)PCIDevice)->BAR2;
            BAR[3] = ((PCI::PCIHeader0 *)PCIDevice)->BAR3;
            BAR[4] = ((PCI::PCIHeader0 *)PCIDevice)->BAR4;
            BAR[5] = ((PCI::PCIHeader0 *)PCIDevice)->BAR5;

            uintptr_t BAR_Type = BAR[0] & 1;
            uintptr_t BAR_IOBase = BAR[1] & (~3);
            uintptr_t BAR_MemoryBase = BAR[0] & (~15);

            debug("Type: %d; IOBase: %#lx; MemoryBase: %#lx", BAR_Type, BAR_IOBase, BAR_MemoryBase);

            for (size_t i = 0; i < 6; i++)
            {
                if (BAR[i] == 0)
                    continue;
                debug("BAR%d: %#lx", i, BAR[i]);
            }

            /* BARs Size */
            for (size_t i = 0; i < 6; i++)
            {
                if (BAR[i] == 0)
                    continue;

                if ((BAR[i] & 1) == 0) // Memory Base
                {
                    ((PCI::PCIHeader0 *)PCIDevice)->BAR0 = 0xFFFFFFFF;
                    size_t size = ((PCI::PCIHeader0 *)PCIDevice)->BAR0;
                    ((PCI::PCIHeader0 *)PCIDevice)->BAR0 = BAR[i];
                    BARsSize[i] = size & (~15);
                    BARsSize[i] = ~BARsSize[i] + 1;
                    BARsSize[i] = BARsSize[i] & 0xFFFFFFFF;
                    debug("BAR%dSize: %#lx", i, BARsSize[i]);
                }
                else if ((BAR[i] & 1) == 1) // I/O Base
                {
                    ((PCI::PCIHeader0 *)PCIDevice)->BAR1 = 0xFFFFFFFF;
                    size_t size = ((PCI::PCIHeader0 *)PCIDevice)->BAR1;
                    ((PCI::PCIHeader0 *)PCIDevice)->BAR1 = BAR[i];
                    BARsSize[i] = size & (~3);
                    BARsSize[i] = ~BARsSize[i] + 1;
                    BARsSize[i] = BARsSize[i] & 0xFFFF;
                    debug("BAR%dSize: %#lx", i, BARsSize[i]);
                }
            }

            /* Mapping the BARs */
            for (size_t i = 0; i < 6; i++)
            {
                if (BAR[i] == 0)
                    continue;

                if ((BAR[i] & 1) == 0) // Memory Base
                {
                    uintptr_t BARBase = BAR[i] & (~15);
                    size_t BARSize = BARsSize[i];

                    debug("Mapping BAR%d from %#lx to %#lx", i, BARBase, BARBase + BARSize);
                    for (uintptr_t j = BARBase;
                         j < (BARBase + BARSize);
                         j += PAGE_SIZE)
                    {
                        vma.Map((void *)j, (void *)j, Memory::PTFlag::RW | Memory::PTFlag::PWT);
                    }
                }
                else if ((BAR[i] & 1) == 1) // I/O Base
                {
                    uintptr_t BARBase = BAR[i] & (~3);
                    uintptr_t BARSize = BARsSize[i];

                    debug("Mapping BAR%d from %#x to %#x", i, BARBase, BARBase + BARSize);
                    for (uintptr_t j = BARBase;
                         j < (BARBase + BARSize);
                         j += PAGE_SIZE)
                    {
                        vma.Map((void *)j, (void *)j, Memory::PTFlag::RW | Memory::PTFlag::PWT);
                    }
                }
            }
            break;
        }
        case 1: // PCI Header 1 (PCI-to-PCI Bridge)
        {
            fixme("PCI Header 1 (PCI-to-PCI Bridge) not implemented yet");
            break;
        }
        case 2: // PCI Header 2 (PCI-to-CardBus Bridge)
        {
            fixme("PCI Header 2 (PCI-to-CardBus Bridge) not implemented yet");
            break;
        }
        default:
        {
            error("Unknown header type %d", PCIDevice->HeaderType);
            return;
        }
        }
    }

    DriverCode Driver::BindPCIGeneric(Memory::MemMgr *mem, void *fex, PCI::PCIDeviceHeader *PCIDevice)
    {
        FexExtended *fexExtended = (FexExtended *)((uintptr_t)fex + EXTENDED_SECTION_ADDRESS);

        if (fexExtended->Driver.OverrideOnConflict)
        {
            std::vector<int> DriversToRemove = std::vector<int>();
            foreach (auto Drv in Drivers)
            {
                FexExtended *fe = ((FexExtended *)((uintptr_t)Drv->Address + EXTENDED_SECTION_ADDRESS));

                if (fe->Driver.OverrideOnConflict)
                {
                    debug("Driver %s is conflicting with %s", fe->Driver.Name, fexExtended->Driver.Name);
                    return DriverCode::DRIVER_CONFLICT;
                }

                DriversToRemove.push_back(Drv->DriverUID);
            }

            foreach (auto DrvID in DriversToRemove)
            {
                if (!this->UnloadDriver(DrvID))
                {
                    error("Failed to unload conflicting driver %d", DrvID);
                    return DriverCode::DRIVER_CONFLICT;
                }
            }
        }
        else
        {
            foreach (auto Drv in Drivers)
            {
                FexExtended *fe = ((FexExtended *)((uintptr_t)Drv->Address + EXTENDED_SECTION_ADDRESS));

                if (fe->Driver.OverrideOnConflict)
                {
                    debug("Driver %s is conflicting with %s", fe->Driver.Name, fexExtended->Driver.Name);
                    return DriverCode::DRIVER_CONFLICT;
                }
            }
        }

        fixme("Generic driver: %s", fexExtended->Driver.Name);
        delete mem, mem = nullptr;
        return DriverCode::NOT_IMPLEMENTED;
    }

    DriverCode Driver::BindPCIDisplay(Memory::MemMgr *mem, void *fex, PCI::PCIDeviceHeader *PCIDevice)
    {
        FexExtended *fexExtended = (FexExtended *)((uintptr_t)fex + EXTENDED_SECTION_ADDRESS);

        if (fexExtended->Driver.OverrideOnConflict)
        {
            std::vector<int> DriversToRemove = std::vector<int>();
            foreach (auto Drv in Drivers)
            {
                FexExtended *fe = ((FexExtended *)((uintptr_t)Drv->Address + EXTENDED_SECTION_ADDRESS));

                if (fe->Driver.OverrideOnConflict)
                {
                    debug("Driver %s is conflicting with %s", fe->Driver.Name, fexExtended->Driver.Name);
                    return DriverCode::DRIVER_CONFLICT;
                }

                DriversToRemove.push_back(Drv->DriverUID);
            }

            foreach (auto DrvID in DriversToRemove)
            {
                if (!this->UnloadDriver(DrvID))
                {
                    error("Failed to unload conflicting driver %d", DrvID);
                    return DriverCode::DRIVER_CONFLICT;
                }
            }
        }
        else
        {
            foreach (auto Drv in Drivers)
            {
                FexExtended *fe = ((FexExtended *)((uintptr_t)Drv->Address + EXTENDED_SECTION_ADDRESS));

                if (fe->Driver.OverrideOnConflict)
                {
                    debug("Driver %s is conflicting with %s", fe->Driver.Name, fexExtended->Driver.Name);
                    return DriverCode::DRIVER_CONFLICT;
                }
            }
        }

        fixme("Display driver: %s", fexExtended->Driver.Name);
        delete mem, mem = nullptr;
        return DriverCode::NOT_IMPLEMENTED;
    }

    DriverCode Driver::BindPCINetwork(Memory::MemMgr *mem, void *fex, PCI::PCIDeviceHeader *PCIDevice)
    {
        FexExtended *fexExtended = (FexExtended *)((uintptr_t)fex + EXTENDED_SECTION_ADDRESS);

        if (fexExtended->Driver.OverrideOnConflict)
        {
            std::vector<int> DriversToRemove = std::vector<int>();
            foreach (auto Drv in Drivers)
            {
                FexExtended *fe = ((FexExtended *)((uintptr_t)Drv->Address + EXTENDED_SECTION_ADDRESS));

                if (fe->Driver.OverrideOnConflict)
                {
                    debug("Driver %s is conflicting with %s", fe->Driver.Name, fexExtended->Driver.Name);
                    return DriverCode::DRIVER_CONFLICT;
                }

                DriversToRemove.push_back(Drv->DriverUID);
            }

            foreach (auto DrvID in DriversToRemove)
            {
                if (!this->UnloadDriver(DrvID))
                {
                    error("Failed to unload conflicting driver %d", DrvID);
                    return DriverCode::DRIVER_CONFLICT;
                }
            }
        }
        else
        {
            foreach (auto Drv in Drivers)
            {
                FexExtended *fe = ((FexExtended *)((uintptr_t)Drv->Address + EXTENDED_SECTION_ADDRESS));

                if (fe->Driver.OverrideOnConflict)
                {
                    debug("Driver %s is conflicting with %s", fe->Driver.Name, fexExtended->Driver.Name);
                    return DriverCode::DRIVER_CONFLICT;
                }
            }
        }

        DriverFile *DrvFile = new DriverFile;
        DrvFile->Enabled = true;
        DrvFile->DriverUID = this->DriverUIDs - 1;
        DrvFile->Address = (void *)fex;
        DrvFile->InterruptCallback = (void *)((uintptr_t)fex + (uintptr_t)fexExtended->Driver.InterruptCallback);
        DrvFile->MemTrk = mem;
        if (fexExtended->Driver.InterruptCallback)
            DrvFile->InterruptHook[0] = new DriverInterruptHook(((int)((PCI::PCIHeader0 *)PCIDevice)->InterruptLine), DrvFile);

        KernelCallback KCallback = {.raw = 0};
        KCallback.RawPtr = PCIDevice;
        KCallback.Reason = CallbackReason::ConfigurationReason;
        int CallbackRet = ((int (*)(KernelCallback *))((uintptr_t)fexExtended->Driver.Callback + (uintptr_t)fex))(&KCallback);

        if (CallbackRet == DriverReturnCode::NOT_IMPLEMENTED)
        {
            error("Driver %s is not implemented", fexExtended->Driver.Name);
            delete DrvFile, DrvFile = nullptr;
            delete mem, mem = nullptr;
            return DriverCode::NOT_IMPLEMENTED;
        }
        else if (CallbackRet == DriverReturnCode::OK)
            trace("Device found for driver: %s", fexExtended->Driver.Name);
        else
        {
            error("Driver %s returned error %d", fexExtended->Driver.Name, CallbackRet);
            delete DrvFile, DrvFile = nullptr;
            delete mem, mem = nullptr;
            return DriverCode::DRIVER_RETURNED_ERROR;
        }

        Drivers.push_back(DrvFile);
        return DriverCode::OK;
    }

    DriverCode Driver::BindPCIStorage(Memory::MemMgr *mem, void *fex, PCI::PCIDeviceHeader *PCIDevice)
    {
        FexExtended *fexExtended = (FexExtended *)((uintptr_t)fex + EXTENDED_SECTION_ADDRESS);

        if (fexExtended->Driver.OverrideOnConflict)
        {
            std::vector<int> DriversToRemove = std::vector<int>();
            foreach (auto Drv in Drivers)
            {
                FexExtended *fe = ((FexExtended *)((uintptr_t)Drv->Address + EXTENDED_SECTION_ADDRESS));

                if (fe->Driver.OverrideOnConflict)
                {
                    debug("Driver %s is conflicting with %s", fe->Driver.Name, fexExtended->Driver.Name);
                    return DriverCode::DRIVER_CONFLICT;
                }

                DriversToRemove.push_back(Drv->DriverUID);
            }

            foreach (auto DrvID in DriversToRemove)
            {
                if (!this->UnloadDriver(DrvID))
                {
                    error("Failed to unload conflicting driver %d", DrvID);
                    return DriverCode::DRIVER_CONFLICT;
                }
            }
        }
        else
        {
            foreach (auto Drv in Drivers)
            {
                FexExtended *fe = ((FexExtended *)((uintptr_t)Drv->Address + EXTENDED_SECTION_ADDRESS));

                if (fe->Driver.OverrideOnConflict)
                {
                    debug("Driver %s is conflicting with %s", fe->Driver.Name, fexExtended->Driver.Name);
                    return DriverCode::DRIVER_CONFLICT;
                }
            }
        }

        DriverFile *DrvFile = new DriverFile;
        DrvFile->Enabled = true;
        DrvFile->DriverUID = this->DriverUIDs - 1;
        DrvFile->Address = (void *)fex;
        DrvFile->InterruptCallback = (void *)((uintptr_t)fex + (uintptr_t)fexExtended->Driver.InterruptCallback);
        DrvFile->MemTrk = mem;
        if (fexExtended->Driver.InterruptCallback)
            DrvFile->InterruptHook[0] = new DriverInterruptHook(((int)((PCI::PCIHeader0 *)PCIDevice)->InterruptLine), DrvFile);

        KernelCallback KCallback = {.raw = 0};
        KCallback.RawPtr = PCIDevice;
        KCallback.Reason = CallbackReason::ConfigurationReason;
        int CallbackRet = ((int (*)(KernelCallback *))((uintptr_t)fexExtended->Driver.Callback + (uintptr_t)fex))(&KCallback);

        if (CallbackRet == DriverReturnCode::NOT_IMPLEMENTED)
        {
            error("Driver %s is not implemented", fexExtended->Driver.Name);
            delete DrvFile, DrvFile = nullptr;
            delete mem, mem = nullptr;
            return DriverCode::NOT_IMPLEMENTED;
        }
        else if (CallbackRet == DriverReturnCode::OK)
            trace("Device found for driver: %s", fexExtended->Driver.Name);
        else
        {
            error("Driver %s returned error %d", fexExtended->Driver.Name, CallbackRet);
            delete DrvFile, DrvFile = nullptr;
            delete mem, mem = nullptr;
            return DriverCode::DRIVER_RETURNED_ERROR;
        }

        Drivers.push_back(DrvFile);
        return DriverCode::OK;
    }

    DriverCode Driver::BindPCIFileSystem(Memory::MemMgr *mem, void *fex, PCI::PCIDeviceHeader *PCIDevice)
    {
        FexExtended *fexExtended = (FexExtended *)((uintptr_t)fex + EXTENDED_SECTION_ADDRESS);

        if (fexExtended->Driver.OverrideOnConflict)
        {
            std::vector<int> DriversToRemove = std::vector<int>();
            foreach (auto Drv in Drivers)
            {
                FexExtended *fe = ((FexExtended *)((uintptr_t)Drv->Address + EXTENDED_SECTION_ADDRESS));

                if (fe->Driver.OverrideOnConflict)
                {
                    debug("Driver %s is conflicting with %s", fe->Driver.Name, fexExtended->Driver.Name);
                    return DriverCode::DRIVER_CONFLICT;
                }

                DriversToRemove.push_back(Drv->DriverUID);
            }

            foreach (auto DrvID in DriversToRemove)
            {
                if (!this->UnloadDriver(DrvID))
                {
                    error("Failed to unload conflicting driver %d", DrvID);
                    return DriverCode::DRIVER_CONFLICT;
                }
            }
        }
        else
        {
            foreach (auto Drv in Drivers)
            {
                FexExtended *fe = ((FexExtended *)((uintptr_t)Drv->Address + EXTENDED_SECTION_ADDRESS));

                if (fe->Driver.OverrideOnConflict)
                {
                    debug("Driver %s is conflicting with %s", fe->Driver.Name, fexExtended->Driver.Name);
                    return DriverCode::DRIVER_CONFLICT;
                }
            }
        }

        fixme("Filesystem driver: %s", fexExtended->Driver.Name);
        delete mem, mem = nullptr;
        return DriverCode::NOT_IMPLEMENTED;
    }

    DriverCode Driver::BindPCIInput(Memory::MemMgr *mem, void *fex, PCI::PCIDeviceHeader *PCIDevice)
    {
        FexExtended *fexExtended = (FexExtended *)((uintptr_t)fex + EXTENDED_SECTION_ADDRESS);

        if (fexExtended->Driver.OverrideOnConflict)
        {
            std::vector<int> DriversToRemove = std::vector<int>();
            foreach (auto Drv in Drivers)
            {
                FexExtended *fe = ((FexExtended *)((uintptr_t)Drv->Address + EXTENDED_SECTION_ADDRESS));

                if (fe->Driver.OverrideOnConflict)
                {
                    debug("Driver %s is conflicting with %s", fe->Driver.Name, fexExtended->Driver.Name);
                    return DriverCode::DRIVER_CONFLICT;
                }

                DriversToRemove.push_back(Drv->DriverUID);
            }

            foreach (auto DrvID in DriversToRemove)
            {
                if (!this->UnloadDriver(DrvID))
                {
                    error("Failed to unload conflicting driver %d", DrvID);
                    return DriverCode::DRIVER_CONFLICT;
                }
            }
        }
        else
        {
            foreach (auto Drv in Drivers)
            {
                FexExtended *fe = ((FexExtended *)((uintptr_t)Drv->Address + EXTENDED_SECTION_ADDRESS));

                if (fe->Driver.OverrideOnConflict)
                {
                    debug("Driver %s is conflicting with %s", fe->Driver.Name, fexExtended->Driver.Name);
                    return DriverCode::DRIVER_CONFLICT;
                }
            }
        }

        fixme("Input driver: %s", fexExtended->Driver.Name);
        delete mem, mem = nullptr;
        return DriverCode::NOT_IMPLEMENTED;
    }

    DriverCode Driver::BindPCIAudio(Memory::MemMgr *mem, void *fex, PCI::PCIDeviceHeader *PCIDevice)
    {
        FexExtended *fexExtended = (FexExtended *)((uintptr_t)fex + EXTENDED_SECTION_ADDRESS);

        if (fexExtended->Driver.OverrideOnConflict)
        {
            std::vector<int> DriversToRemove = std::vector<int>();
            foreach (auto Drv in Drivers)
            {
                FexExtended *fe = ((FexExtended *)((uintptr_t)Drv->Address + EXTENDED_SECTION_ADDRESS));

                if (fe->Driver.OverrideOnConflict)
                {
                    debug("Driver %s is conflicting with %s", fe->Driver.Name, fexExtended->Driver.Name);
                    return DriverCode::DRIVER_CONFLICT;
                }

                DriversToRemove.push_back(Drv->DriverUID);
            }

            foreach (auto DrvID in DriversToRemove)
            {
                if (!this->UnloadDriver(DrvID))
                {
                    error("Failed to unload conflicting driver %d", DrvID);
                    return DriverCode::DRIVER_CONFLICT;
                }
            }
        }
        else
        {
            foreach (auto Drv in Drivers)
            {
                FexExtended *fe = ((FexExtended *)((uintptr_t)Drv->Address + EXTENDED_SECTION_ADDRESS));

                if (fe->Driver.OverrideOnConflict)
                {
                    debug("Driver %s is conflicting with %s", fe->Driver.Name, fexExtended->Driver.Name);
                    return DriverCode::DRIVER_CONFLICT;
                }
            }
        }

        DriverFile *DrvFile = new DriverFile;
        DrvFile->Enabled = true;
        DrvFile->DriverUID = this->DriverUIDs - 1;
        DrvFile->Address = (void *)fex;
        DrvFile->InterruptCallback = (void *)((uintptr_t)fex + (uintptr_t)fexExtended->Driver.InterruptCallback);
        DrvFile->MemTrk = mem;
        if (fexExtended->Driver.InterruptCallback)
            DrvFile->InterruptHook[0] = new DriverInterruptHook(((int)((PCI::PCIHeader0 *)PCIDevice)->InterruptLine), DrvFile);

        KernelCallback KCallback = {.raw = 0};
        KCallback.RawPtr = PCIDevice;
        KCallback.Reason = CallbackReason::ConfigurationReason;
        int CallbackRet = ((int (*)(KernelCallback *))((uintptr_t)fexExtended->Driver.Callback + (uintptr_t)fex))(&KCallback);

        if (CallbackRet == DriverReturnCode::NOT_IMPLEMENTED)
        {
            error("Driver %s is not implemented", fexExtended->Driver.Name);
            delete DrvFile, DrvFile = nullptr;
            delete mem, mem = nullptr;
            return DriverCode::NOT_IMPLEMENTED;
        }
        else if (CallbackRet == DriverReturnCode::OK)
            trace("Device found for driver: %s", fexExtended->Driver.Name);
        else
        {
            error("Driver %s returned error %d", fexExtended->Driver.Name, CallbackRet);
            delete DrvFile, DrvFile = nullptr;
            delete mem, mem = nullptr;
            return DriverCode::DRIVER_RETURNED_ERROR;
        }

        Drivers.push_back(DrvFile);
        return DriverCode::OK;
    }

    DriverCode Driver::DriverLoadBindPCI(void *DrvExtHdr, uintptr_t DriverAddress, size_t Size, bool IsElf)
    {
        UNUSED(IsElf);
        for (unsigned long Vidx = 0; Vidx < sizeof(((FexExtended *)DrvExtHdr)->Driver.Bind.PCI.VendorID) / sizeof(((FexExtended *)DrvExtHdr)->Driver.Bind.PCI.VendorID[0]); Vidx++)
        {
            for (unsigned long Didx = 0; Didx < sizeof(((FexExtended *)DrvExtHdr)->Driver.Bind.PCI.DeviceID) / sizeof(((FexExtended *)DrvExtHdr)->Driver.Bind.PCI.DeviceID[0]); Didx++)
            {
                if (Vidx >= sizeof(((FexExtended *)DrvExtHdr)->Driver.Bind.PCI.VendorID) && Didx >= sizeof(((FexExtended *)DrvExtHdr)->Driver.Bind.PCI.DeviceID))
                    break;

                if (((FexExtended *)DrvExtHdr)->Driver.Bind.PCI.VendorID[Vidx] == 0 || ((FexExtended *)DrvExtHdr)->Driver.Bind.PCI.DeviceID[Didx] == 0)
                    continue;

                std::vector<PCI::PCIDeviceHeader *> devices = PCIManager->FindPCIDevice(((FexExtended *)DrvExtHdr)->Driver.Bind.PCI.VendorID[Vidx], ((FexExtended *)DrvExtHdr)->Driver.Bind.PCI.DeviceID[Didx]);
                if (devices.size() == 0)
                    continue;

                foreach (auto PCIDevice in devices)
                {
                    debug("[%ld] VendorID: %#x; DeviceID: %#x", devices.size(), PCIDevice->VendorID, PCIDevice->DeviceID);
                    Memory::MemMgr *mem = new Memory::MemMgr(nullptr, TaskManager->GetCurrentProcess()->memDirectory);
                    Fex *fex = (Fex *)mem->RequestPages(TO_PAGES(Size));
                    memcpy(fex, (void *)DriverAddress, Size);
                    FexExtended *fexExtended = (FexExtended *)((uintptr_t)fex + EXTENDED_SECTION_ADDRESS);
                    debug("Driver allocated at %#lx-%#lx", fex, (uintptr_t)fex + Size);
#ifdef DEBUG
                    uint8_t *result = md5File((uint8_t *)fex, Size);
                    debug("MD5: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                          result[0], result[1], result[2], result[3], result[4], result[5], result[6], result[7],
                          result[8], result[9], result[10], result[11], result[12], result[13], result[14], result[15]);
                    kfree(result);
#endif
                    KernelAPI *KAPI = (KernelAPI *)mem->RequestPages(TO_PAGES(sizeof(KernelAPI)));

                    if (CallDriverEntryPoint(fex, KAPI) != DriverCode::OK)
                    {
                        delete mem, mem = nullptr;
                        return DriverCode::DRIVER_RETURNED_ERROR;
                    }
                    debug("Starting driver %s", fexExtended->Driver.Name);

                    MapPCIAddresses(PCIDevice);

                    switch (fexExtended->Driver.Type)
                    {
                    case FexDriverType::FexDriverType_Generic:
                        return BindPCIGeneric(mem, fex, PCIDevice);
                    case FexDriverType::FexDriverType_Display:
                        return BindPCIDisplay(mem, fex, PCIDevice);
                    case FexDriverType::FexDriverType_Network:
                        return BindPCINetwork(mem, fex, PCIDevice);
                    case FexDriverType::FexDriverType_Storage:
                        return BindPCIStorage(mem, fex, PCIDevice);
                    case FexDriverType::FexDriverType_FileSystem:
                        return BindPCIFileSystem(mem, fex, PCIDevice);
                    case FexDriverType::FexDriverType_Input:
                        return BindPCIInput(mem, fex, PCIDevice);
                    case FexDriverType::FexDriverType_Audio:
                        return BindPCIAudio(mem, fex, PCIDevice);
                    default:
                    {
                        warn("Unknown driver type: %d", fexExtended->Driver.Type);
                        delete mem, mem = nullptr;
                        return DriverCode::UNKNOWN_DRIVER_TYPE;
                    }
                    }
                }
            }
        }
        return DriverCode::PCI_DEVICE_NOT_FOUND;
    }
}
