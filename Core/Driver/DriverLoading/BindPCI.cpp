#include "../api.hpp"

#include <interrupts.hpp>
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
    DriverCode Driver::DriverLoadBindPCI(void *DrvExtHdr, uintptr_t DriverAddress, size_t Size, bool IsElf)
    {
        for (unsigned long Vidx = 0; Vidx < sizeof(((FexExtended *)DrvExtHdr)->Driver.Bind.PCI.VendorID) / sizeof(((FexExtended *)DrvExtHdr)->Driver.Bind.PCI.VendorID[0]); Vidx++)
            for (unsigned long Didx = 0; Didx < sizeof(((FexExtended *)DrvExtHdr)->Driver.Bind.PCI.DeviceID) / sizeof(((FexExtended *)DrvExtHdr)->Driver.Bind.PCI.DeviceID[0]); Didx++)
            {
                if (Vidx >= sizeof(((FexExtended *)DrvExtHdr)->Driver.Bind.PCI.VendorID) && Didx >= sizeof(((FexExtended *)DrvExtHdr)->Driver.Bind.PCI.DeviceID))
                    break;

                if (((FexExtended *)DrvExtHdr)->Driver.Bind.PCI.VendorID[Vidx] == 0 || ((FexExtended *)DrvExtHdr)->Driver.Bind.PCI.DeviceID[Didx] == 0)
                    continue;

                Vector<PCI::PCIDeviceHeader *> devices = PCIManager->FindPCIDevice(((FexExtended *)DrvExtHdr)->Driver.Bind.PCI.VendorID[Vidx], ((FexExtended *)DrvExtHdr)->Driver.Bind.PCI.DeviceID[Didx]);
                if (devices.size() == 0)
                    continue;
                foreach (auto PCIDevice in devices)
                {
                    debug("[%ld] VendorID: %#x; DeviceID: %#x", devices.size(), PCIDevice->VendorID, PCIDevice->DeviceID);
                    Memory::Tracker *Tracker = new Memory::Tracker();
                    Fex *fex = (Fex *)Tracker->RequestPages(TO_PAGES(Size));
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
                    KernelAPI *KAPI = (KernelAPI *)Tracker->RequestPages(TO_PAGES(sizeof(KernelAPI)));

                    if (CallDriverEntryPoint(fex, KAPI) != DriverCode::OK)
                    {
                        delete Tracker;
                        return DriverCode::DRIVER_RETURNED_ERROR;
                    }
                    debug("Starting driver %s", fexExtended->Driver.Name);

                    KernelCallback *KCallback = (KernelCallback *)Tracker->RequestPages(TO_PAGES(sizeof(KernelCallback)));

                    debug("Type: %d; IOBase: %#x; MemoryBase: %#x",
                          ((PCI::PCIHeader0 *)PCIDevice)->BAR0 & 1,
                          ((PCI::PCIHeader0 *)PCIDevice)->BAR1 & (~3),
                          ((PCI::PCIHeader0 *)PCIDevice)->BAR0 & (~15));

                    if ((((PCI::PCIHeader0 *)PCIDevice)->BAR0 & 1) != 0)
                        if (!Memory::Virtual().Check((void *)(uintptr_t)(((PCI::PCIHeader0 *)PCIDevice)->BAR1 & (~3))))
                        {
                            debug("IO base (BAR1 & ~3) is not mapped");
                            Memory::Virtual().Map((void *)(uintptr_t)(((PCI::PCIHeader0 *)PCIDevice)->BAR1 & (~3)), (void *)(uintptr_t)(((PCI::PCIHeader0 *)PCIDevice)->BAR1 & (~3)), Memory::PTFlag::RW);
                        }

                    if ((((PCI::PCIHeader0 *)PCIDevice)->BAR0 & 1) == 0)
                        if (!Memory::Virtual().Check((void *)(uintptr_t)(((PCI::PCIHeader0 *)PCIDevice)->BAR0 & (~15))))
                        {
                            debug("Memory base (BAR0 & ~15) is not mapped");
                            Memory::Virtual().Map((void *)(uintptr_t)(((PCI::PCIHeader0 *)PCIDevice)->BAR0 & (~15)), (void *)(uintptr_t)(((PCI::PCIHeader0 *)PCIDevice)->BAR0 & (~15)), Memory::PTFlag::RW);

                            uintptr_t original = ((PCI::PCIHeader0 *)PCIDevice)->BAR0;
                            ((PCI::PCIHeader0 *)PCIDevice)->BAR0 = 0xFFFFFFFF;
                            uintptr_t size = ((PCI::PCIHeader0 *)PCIDevice)->BAR0 & 0xFFFFFFF0;
                            ((PCI::PCIHeader0 *)PCIDevice)->BAR0 = original;
                            debug("Size: %#lx (%ld pages)", size, TO_PAGES(size));
                            fixme("TODO: [BUG] Mapping is broken!!!!!!");
                        }

                    switch (fexExtended->Driver.Type)
                    {
                    case FexDriverType::FexDriverType_Generic:
                    {
                        fixme("Generic driver: %s", fexExtended->Driver.Name);
                        delete Tracker;
                        break;
                    }
                    case FexDriverType::FexDriverType_Display:
                    {
                        fixme("Display driver: %s", fexExtended->Driver.Name);
                        delete Tracker;
                        break;
                    }
                    case FexDriverType::FexDriverType_Network:
                    {
                        DriverInterruptHook *InterruptHook = new DriverInterruptHook(((int)((PCI::PCIHeader0 *)devices[0])->InterruptLine) + 32, // x86
                                                                                     (void *)((uintptr_t)fexExtended->Driver.Callback + (uintptr_t)fex),
                                                                                     KCallback);

                        KCallback->RawPtr = PCIDevice;
                        KCallback->Reason = CallbackReason::ConfigurationReason;
                        int CallbackRet = ((int (*)(KernelCallback *))((uintptr_t)fexExtended->Driver.Callback + (uintptr_t)fex))(KCallback);
                        if (CallbackRet == DriverReturnCode::NOT_IMPLEMENTED)
                        {
                            error("Driver %s does not implement the configuration callback", fexExtended->Driver.Name);
                            delete Tracker;
                            delete InterruptHook;
                            continue;
                        }
                        else if (CallbackRet == DriverReturnCode::OK)
                            trace("Device found for driver: %s", fexExtended->Driver.Name);
                        else
                        {
                            error("Driver %s returned error %d", fexExtended->Driver.Name, CallbackRet);
                            delete Tracker;
                            delete InterruptHook;
                            continue;
                        }

                        memset(KCallback, 0, sizeof(KernelCallback));
                        KCallback->Reason = CallbackReason::InterruptReason;

                        DriverFile *DrvFile = new DriverFile;
                        DrvFile->DriverUID = this->DriverUIDs - 1;
                        DrvFile->Address = (void *)fex;
                        DrvFile->InterruptHook[0] = InterruptHook;
                        Drivers.push_back(DrvFile);
                        break;
                    }
                    case FexDriverType::FexDriverType_Storage:
                    {
                        KCallback->RawPtr = PCIDevice;
                        KCallback->Reason = CallbackReason::ConfigurationReason;
                        int CallbackRet = ((int (*)(KernelCallback *))((uintptr_t)fexExtended->Driver.Callback + (uintptr_t)fex))(KCallback);
                        if (CallbackRet == DriverReturnCode::NOT_IMPLEMENTED)
                        {
                            error("Driver %s does not implement the configuration callback", fexExtended->Driver.Name);
                            delete Tracker;
                            continue;
                        }
                        else if (CallbackRet == DriverReturnCode::OK)
                            trace("Device found for driver: %s", fexExtended->Driver.Name);
                        else
                        {
                            error("Driver %s returned error %d", fexExtended->Driver.Name, CallbackRet);
                            delete Tracker;
                            continue;
                        }

                        DriverFile *DrvFile = new DriverFile;
                        DrvFile->DriverUID = this->DriverUIDs - 1;
                        DrvFile->Address = (void *)fex;
                        DrvFile->InterruptHook[0] = nullptr;
                        Drivers.push_back(DrvFile);
                        break;
                    }
                    case FexDriverType::FexDriverType_FileSystem:
                    {
                        fixme("Filesystem driver: %s", fexExtended->Driver.Name);
                        delete Tracker;
                        break;
                    }
                    case FexDriverType::FexDriverType_Input:
                    {
                        fixme("Input driver: %s", fexExtended->Driver.Name);
                        delete Tracker;
                        break;
                    }
                    case FexDriverType::FexDriverType_Audio:
                    {
                        fixme("Audio driver: %s", fexExtended->Driver.Name);
                        delete Tracker;
                        break;
                    }
                    default:
                    {
                        warn("Unknown driver type: %d", fexExtended->Driver.Type);
                        delete Tracker;
                        break;
                    }
                    }
                }
            }

        return DriverCode::OK;
    }
}
