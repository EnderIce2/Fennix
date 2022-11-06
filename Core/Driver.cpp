#include <driver.hpp>

#include <interrupts.hpp>
#include <memory.hpp>
#include <dumper.hpp>
#include <task.hpp>
#include <lock.hpp>
#include <printf.h>
#include <cwalk.h>
#include <md5.h>

#include "../kernel.h"
#include "../DAPI.hpp"
#include "../Fex.hpp"

NewLock(DriverInitLock);
NewLock(DriverDisplayPrintLock);

namespace Driver
{
    const char *DriverTypesName[] = {
        "Unknown",
        "Generic",
        "Display",
        "Network",
        "Storage",
        "FileSystem",
        "Input",
        "Audio"};

    void DriverDebugPrint(char *String, unsigned long DriverUID)
    {
        SmartLock(DriverDisplayPrintLock);
        trace("[%ld] %s", DriverUID, String);
    }

    void DriverDisplayPrint(char *String)
    {
        SmartLock(DriverDisplayPrintLock);
        for (unsigned long i = 0; i < strlen(String); i++)
            Display->Print(String[i], 0, true);
    }

    void *RequestPage(unsigned long Size)
    {
        SmartLock(DriverDisplayPrintLock);
        return KernelAllocator.RequestPages(Size);
    }

    void FreePage(void *Page, unsigned long Size)
    {
        SmartLock(DriverDisplayPrintLock);
        KernelAllocator.FreePages(Page, Size);
    }

    void MapMemory(void *VirtualAddress, void *PhysicalAddress, unsigned long Flags)
    {
        SmartLock(DriverDisplayPrintLock);
        Memory::Virtual().Map(VirtualAddress, PhysicalAddress, Flags);
    }

    void UnmapMemory(void *VirtualAddress)
    {
        SmartLock(DriverDisplayPrintLock);
        Memory::Virtual().Unmap(VirtualAddress);
    }

    void *Drivermemcpy(void *Destination, void *Source, unsigned long Size)
    {
        SmartLock(DriverDisplayPrintLock);
        return memcpy(Destination, Source, Size);
    }

    void *Drivermemset(void *Destination, int Value, unsigned long Size)
    {
        SmartLock(DriverDisplayPrintLock);
        return memset(Destination, Value, Size);
    }

    void DriverNetSend(unsigned int DriverID, unsigned char *Data, unsigned short Size)
    {
        DumpData("DriverNetSend", Data, Size);
    }

    void DriverNetReceive(unsigned int DriverID, unsigned char *Data, unsigned short Size)
    {
        DumpData("DriverNetReceive", Data, Size);
    }

    void DriverAHCIDiskRead(unsigned int DriverID, unsigned long Sector, unsigned char *Data, unsigned int SectorCount, unsigned char Port)
    {
        DumpData("DriverDiskRead", Data, SectorCount * 512);
    }

    void DriverAHCIDiskWrite(unsigned int DriverID, unsigned long Sector, unsigned char *Data, unsigned int SectorCount, unsigned char Port)
    {
        DumpData("DriverDiskWrite", Data, SectorCount * 512);
    }

    char *DriverPCIGetDeviceName(unsigned int VendorID, unsigned int DeviceID)
    {
        return (char *)"Unknown";
    }

    KernelAPI KAPI = {
        .Version = {
            .Major = 0,
            .Minor = 0,
            .Patch = 1},
        .Info = {
            .Offset = 0,
            .DriverUID = 0,
        },
        .Memory = {
            .PageSize = PAGE_SIZE,
            .RequestPage = RequestPage,
            .FreePage = FreePage,
            .Map = MapMemory,
            .Unmap = UnmapMemory,
        },
        .PCI = {
            .GetDeviceName = DriverPCIGetDeviceName,
        },
        .Util = {
            .DebugPrint = DriverDebugPrint,
            .DisplayPrint = DriverDisplayPrint,
            .memcpy = Drivermemcpy,
            .memset = Drivermemset,
        },
        .Commmand = {
            .Network = {
                .SendPacket = DriverNetSend,
                .ReceivePacket = DriverNetReceive,
            },
            .Disk = {
                .AHCI = {
                    .ReadSector = DriverAHCIDiskRead,
                    .WriteSector = DriverAHCIDiskWrite,
                },
            },
        },
    };

    int Driver::IOCB(unsigned long DUID, void *KCB)
    {
        foreach (auto var in Drivers)
            if (var->DriverUID == DUID)
            {
                FexExtended *DrvExtHdr = (FexExtended *)((uint64_t)var->Address + EXTENDED_SECTION_ADDRESS);
                return ((int (*)(void *))((uint64_t)DrvExtHdr->Driver.Callback + (uint64_t)var->Address))(KCB);
            }
        return -1;
    }

    DriverCode Driver::CallDriverEntryPoint(void *fex)
    {
        KernelAPI *API = (KernelAPI *)KernelAllocator.RequestPages(TO_PAGES(sizeof(KernelAPI)));
        memcpy(API, &KAPI, sizeof(KernelAPI));

        API->Info.Offset = (unsigned long)fex;
        API->Info.DriverUID = DriverUIDs++;

        int ret = ((int (*)(KernelAPI *))((uint64_t)((Fex *)fex)->Pointer + (uint64_t)fex))(API);

        if (DriverReturnCode::OK != ret)
            return DriverCode::DRIVER_RETURNED_ERROR;
        return DriverCode::OK;
    }

    DriverCode Driver::LoadDriver(uint64_t DriverAddress, uint64_t Size)
    {
        Fex *DrvHdr = (Fex *)DriverAddress;
        if (DrvHdr->Magic[0] != 'F' || DrvHdr->Magic[1] != 'E' || DrvHdr->Magic[2] != 'X' || DrvHdr->Magic[3] != '\0')
            return DriverCode::INVALID_FEX_HEADER;
        debug("Fex Magic: \"%s\"; Type: %d; OS: %d; Pointer: %#lx", DrvHdr->Magic, DrvHdr->Type, DrvHdr->OS, DrvHdr->Pointer);

        if (DrvHdr->Type == FexFormatType::FexFormatType_Driver)
        {
            FexExtended *DrvExtHdr = (FexExtended *)((uint64_t)DrvHdr + EXTENDED_SECTION_ADDRESS);
            debug("Name: \"%s\"; Type: %d; Callback: %#lx", DrvExtHdr->Driver.Name, DrvExtHdr->Driver.Type, DrvExtHdr->Driver.Callback);

            if (DrvExtHdr->Driver.Bind.Type == DriverBindType::BIND_PCI)
            {
                for (unsigned long Vidx = 0; Vidx < sizeof(DrvExtHdr->Driver.Bind.PCI.VendorID) / sizeof(DrvExtHdr->Driver.Bind.PCI.VendorID[0]); Vidx++)
                    for (unsigned long Didx = 0; Didx < sizeof(DrvExtHdr->Driver.Bind.PCI.DeviceID) / sizeof(DrvExtHdr->Driver.Bind.PCI.DeviceID[0]); Didx++)
                    {
                        if (Vidx >= sizeof(DrvExtHdr->Driver.Bind.PCI.VendorID) && Didx >= sizeof(DrvExtHdr->Driver.Bind.PCI.DeviceID))
                            break;

                        if (DrvExtHdr->Driver.Bind.PCI.VendorID[Vidx] == 0 || DrvExtHdr->Driver.Bind.PCI.DeviceID[Didx] == 0)
                            continue;

                        Vector<PCI::PCIDeviceHeader *> devices = PCIManager->FindPCIDevice(DrvExtHdr->Driver.Bind.PCI.VendorID[Vidx], DrvExtHdr->Driver.Bind.PCI.DeviceID[Didx]);
                        if (devices.size() == 0)
                            continue;
                        foreach (auto PCIDevice in devices)
                        {
                            debug("[%ld] VendorID: %#x; DeviceID: %#x", devices.size(), PCIDevice->VendorID, PCIDevice->DeviceID);
                            Fex *fex = (Fex *)KernelAllocator.RequestPages(TO_PAGES(Size));
                            memcpy(fex, (void *)DriverAddress, Size);
                            FexExtended *fexExtended = (FexExtended *)((uint64_t)fex + EXTENDED_SECTION_ADDRESS);
#ifdef DEBUG
                            uint8_t *result = md5File((uint8_t *)fex, Size);
                            debug("MD5: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                                  result[0], result[1], result[2], result[3], result[4], result[5], result[6], result[7],
                                  result[8], result[9], result[10], result[11], result[12], result[13], result[14], result[15]);
#endif
                            if (CallDriverEntryPoint(fex) != DriverCode::OK)
                            {
                                KernelAllocator.FreePages(fex, TO_PAGES(Size));
                                return DriverCode::DRIVER_RETURNED_ERROR;
                            }
                            debug("Starting driver %s", fexExtended->Driver.Name);

                            KernelCallback *KCallback = (KernelCallback *)KernelAllocator.RequestPages(TO_PAGES(sizeof(KernelCallback)));

                            switch (fexExtended->Driver.Type)
                            {
                            case FexDriverType::FexDriverType_Generic:
                            {
                                fixme("Generic driver: %s", fexExtended->Driver.Name);
                                break;
                            }
                            case FexDriverType::FexDriverType_Display:
                            {
                                fixme("Display driver: %s", fexExtended->Driver.Name);
                                break;
                            }
                            case FexDriverType::FexDriverType_Network:
                            {
                                DriverInterruptHook *InterruptHook = new DriverInterruptHook(((int)((PCI::PCIHeader0 *)devices[0])->InterruptLine) + 32, // x86
                                                                                             (void *)((uint64_t)fexExtended->Driver.Callback + (uint64_t)fex),
                                                                                             KCallback);

                                KCallback->RawPtr = PCIDevice;
                                KCallback->Reason = CallbackReason::ConfigurationReason;
                                int callbackret = ((int (*)(KernelCallback *))((uint64_t)fexExtended->Driver.Callback + (uint64_t)fex))(KCallback);
                                if (callbackret == DriverReturnCode::NOT_IMPLEMENTED)
                                {
                                    KernelAllocator.FreePages(fex, TO_PAGES(Size));
                                    KernelAllocator.FreePages(KCallback, TO_PAGES(sizeof(KernelCallback)));
                                    delete InterruptHook;
                                    error("Driver %s does not implement the configuration callback", fexExtended->Driver.Name);
                                    continue;
                                }
                                else if (callbackret == DriverReturnCode::OK)
                                    trace("Device found for driver: %s", fexExtended->Driver.Name);
                                else
                                {
                                    KernelAllocator.FreePages(fex, TO_PAGES(Size));
                                    KernelAllocator.FreePages(KCallback, TO_PAGES(sizeof(KernelCallback)));
                                    delete InterruptHook;
                                    error("Driver %s returned error %d", fexExtended->Driver.Name, callbackret);
                                    continue;
                                }

                                memset(KCallback, 0, sizeof(KernelCallback));
                                KCallback->Reason = CallbackReason::InterruptReason;

                                DriverFile *drvfile = new DriverFile;
                                drvfile->DriverUID = KAPI.Info.DriverUID;
                                drvfile->Address = (void *)fex;
                                drvfile->InterruptHook[0] = InterruptHook;
                                Drivers.push_back(drvfile);
                                break;
                            }
                            case FexDriverType::FexDriverType_Storage:
                            {
                                KCallback->RawPtr = PCIDevice;
                                KCallback->Reason = CallbackReason::ConfigurationReason;
                                int callbackret = ((int (*)(KernelCallback *))((uint64_t)fexExtended->Driver.Callback + (uint64_t)fex))(KCallback);
                                if (callbackret == DriverReturnCode::NOT_IMPLEMENTED)
                                {
                                    KernelAllocator.FreePages(fex, TO_PAGES(Size));
                                    KernelAllocator.FreePages(KCallback, TO_PAGES(sizeof(KernelCallback)));
                                    error("Driver %s does not implement the configuration callback", fexExtended->Driver.Name);
                                    continue;
                                }
                                else if (callbackret == DriverReturnCode::OK)
                                    trace("Device found for driver: %s", fexExtended->Driver.Name);
                                else
                                {
                                    KernelAllocator.FreePages(fex, TO_PAGES(Size));
                                    KernelAllocator.FreePages(KCallback, TO_PAGES(sizeof(KernelCallback)));
                                    error("Driver %s returned error %d", fexExtended->Driver.Name, callbackret);
                                    continue;
                                }

                                DriverFile *drvfile = new DriverFile;
                                drvfile->DriverUID = KAPI.Info.DriverUID;
                                drvfile->Address = (void *)fex;
                                drvfile->InterruptHook[0] = nullptr;
                                Drivers.push_back(drvfile);
                                break;
                            }
                            case FexDriverType::FexDriverType_FileSystem:
                            {
                                fixme("Filesystem driver: %s", fexExtended->Driver.Name);
                                break;
                            }
                            case FexDriverType::FexDriverType_Input:
                            {
                                fixme("Input driver: %s", fexExtended->Driver.Name);
                                break;
                            }
                            case FexDriverType::FexDriverType_Audio:
                            {
                                fixme("Audio driver: %s", fexExtended->Driver.Name);
                                break;
                            }
                            default:
                            {
                                warn("Unknown driver type: %d", fexExtended->Driver.Type);
                                break;
                            }
                            }
                        }
                    }
            }
            else if (DrvExtHdr->Driver.Bind.Type == DriverBindType::BIND_INTERRUPT)
            {
                Fex *fex = (Fex *)KernelAllocator.RequestPages(TO_PAGES(Size));
                memcpy(fex, (void *)DriverAddress, Size);
                FexExtended *fexExtended = (FexExtended *)((uint64_t)fex + EXTENDED_SECTION_ADDRESS);
#ifdef DEBUG
                            uint8_t *result = md5File((uint8_t *)fex, Size);
                            debug("MD5: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                                  result[0], result[1], result[2], result[3], result[4], result[5], result[6], result[7],
                                  result[8], result[9], result[10], result[11], result[12], result[13], result[14], result[15]);
#endif
                if (CallDriverEntryPoint(fex) != DriverCode::OK)
                {
                    KernelAllocator.FreePages(fex, TO_PAGES(Size));
                    return DriverCode::DRIVER_RETURNED_ERROR;
                }
                debug("Starting driver %s (offset: %#lx)", fexExtended->Driver.Name, fex);

                KernelCallback *KCallback = (KernelCallback *)KernelAllocator.RequestPages(TO_PAGES(sizeof(KernelCallback)));

                switch (fexExtended->Driver.Type)
                {
                case FexDriverType::FexDriverType_Generic:
                {
                    fixme("Generic driver: %s", fexExtended->Driver.Name);
                    break;
                }
                case FexDriverType::FexDriverType_Display:
                {
                    fixme("Display driver: %s", fexExtended->Driver.Name);
                    break;
                }
                case FexDriverType::FexDriverType_Network:
                {
                    fixme("Network driver: %s", fexExtended->Driver.Name);
                    break;
                }
                case FexDriverType::FexDriverType_Storage:
                {
                    for (unsigned long i = 0; i < sizeof(DrvExtHdr->Driver.Bind.Interrupt.Vector) / sizeof(DrvExtHdr->Driver.Bind.Interrupt.Vector[0]); i++)
                    {
                        if (DrvExtHdr->Driver.Bind.Interrupt.Vector[i] == 0)
                            break;

                        fixme("TODO: MULTIPLE BIND INTERRUPT VECTORS %d", DrvExtHdr->Driver.Bind.Interrupt.Vector[i]);
                    }

                    fixme("Not implemented");
                    KernelAllocator.FreePages(fex, TO_PAGES(Size));
                    KernelAllocator.FreePages(KCallback, TO_PAGES(sizeof(KernelCallback)));
                    break;

                    KCallback->RawPtr = nullptr;
                    KCallback->Reason = CallbackReason::ConfigurationReason;
                    int callbackret = ((int (*)(KernelCallback *))((uint64_t)fexExtended->Driver.Callback + (uint64_t)fex))(KCallback);
                    if (callbackret == DriverReturnCode::NOT_IMPLEMENTED)
                    {
                        KernelAllocator.FreePages(fex, TO_PAGES(Size));
                        KernelAllocator.FreePages(KCallback, TO_PAGES(sizeof(KernelCallback)));
                        error("Driver %s does not implement the configuration callback", fexExtended->Driver.Name);
                        break;
                    }
                    else if (callbackret != DriverReturnCode::OK)
                    {
                        KernelAllocator.FreePages(fex, TO_PAGES(Size));
                        KernelAllocator.FreePages(KCallback, TO_PAGES(sizeof(KernelCallback)));
                        error("Driver %s returned error %d", fexExtended->Driver.Name, callbackret);
                        break;
                    }

                    KernelAllocator.FreePages(fex, TO_PAGES(Size));
                    KernelAllocator.FreePages(KCallback, TO_PAGES(sizeof(KernelCallback)));

                    // DriverFile *drvfile = new DriverFile;
                    // Drivers.push_back(drvfile);
                    break;
                }
                case FexDriverType::FexDriverType_FileSystem:
                {
                    fixme("Filesystem driver: %s", fexExtended->Driver.Name);
                    break;
                }
                case FexDriverType::FexDriverType_Input:
                {
                    fixme("Input driver: %s", fexExtended->Driver.Name);
                    break;
                }
                case FexDriverType::FexDriverType_Audio:
                {
                    fixme("Audio driver: %s", fexExtended->Driver.Name);
                    break;
                }
                default:
                {
                    warn("Unknown driver type: %d", fexExtended->Driver.Type);
                    break;
                }
                }
            }
            else if (DrvExtHdr->Driver.Bind.Type == DriverBindType::BIND_PROCESS)
            {
                fixme("Process driver: %s", DrvExtHdr->Driver.Name);
            }
            else
            {
                error("Unknown driver bind type: %d", DrvExtHdr->Driver.Bind.Type);
            }
        }
        else
            return DriverCode::NOT_DRIVER;
        return DriverCode::OK;
    }

    Driver::Driver()
    {
        SmartCriticalSection(DriverInitLock);
        FileSystem::FILE *DriverDirectory = vfs->Open(Config.DriverDirectory);
        if (DriverDirectory->Status == FileSystem::FileStatus::OK)
            foreach (auto driver in DriverDirectory->Node->Children)
                if (driver->Flags == FileSystem::NodeFlags::FS_FILE)
                    if (cwk_path_has_extension(driver->Name))
                    {
                        const char *extension;
                        cwk_path_get_extension(driver->Name, &extension, nullptr);
                        if (!strcmp(extension, ".fex"))
                        {
                            uint64_t ret = this->LoadDriver(driver->Address, driver->Length);
                            char retstring[64];
                            if (ret == DriverCode::OK)
                                strcpy(retstring, "\e058C19OK");
                            else
                                sprintf_(retstring, "\eE85230FAILED (%#lx)", ret);
                            KPrint("%s %s", driver->Name, retstring);
                        }
                    }
        vfs->Close(DriverDirectory);
    }

    Driver::~Driver()
    {
    }

#if defined(__amd64__)
    void DriverInterruptHook::OnInterruptReceived(CPU::x64::TrapFrame *Frame)
#elif defined(__i386__)
    void DriverInterruptHook::OnInterruptReceived(void *Frame)
#elif defined(__aarch64__)
    void DriverInterruptHook::OnInterruptReceived(void *Frame)
#endif
    {
        CriticalSection cs; // or SmartCriticalSection(DriverInitLock); ?
        ((int (*)(void *))(Handle))(Data);
    }

    DriverInterruptHook::DriverInterruptHook(int Interrupt, void *Address, void *ParamData) : Interrupts::Handler(Interrupt)
    {
        trace("Interrupt %d Hooked", Interrupt - 32); // x86
        Handle = Address;
        Data = ParamData;
    }
}
