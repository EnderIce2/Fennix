#include <driver.hpp>

#include <interrupts.hpp>
#include <memory.hpp>
#include <task.hpp>
#include <lock.hpp>
#include <printf.h>
#include <cwalk.h>

#include "../kernel.h"
#include "../DAPI.hpp"
#include "../Fex.hpp"

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

    void *Drivermemcpy(void *Destination, void *Source, unsigned long Size)
    {
        SmartLock(DriverDisplayPrintLock);
        return memcpy(Destination, Source, Size);
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
        },
        .PCI = {
            .GetDeviceName = nullptr,
            .Write = nullptr,
        },
        .Util = {
            .DebugPrint = DriverDebugPrint,
            .DisplayPrint = DriverDisplayPrint,
            .memcpy = Drivermemcpy,
        },
        .Commmand = {
            .Network = {
                .SendPacket = nullptr,
                .ReceivePacket = nullptr,
            },
            .Disk = {
                .AHCI = {
                    .ReadSector = nullptr,
                    .WriteSector = nullptr,
                },
            },
        },
    };

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
                for (long unsigned Vidx = 0; Vidx < sizeof(DrvExtHdr->Driver.Bind.PCI.VendorID) / sizeof(DrvExtHdr->Driver.Bind.PCI.VendorID[0]); Vidx++)
                    for (long unsigned Didx = 0; Didx < sizeof(DrvExtHdr->Driver.Bind.PCI.DeviceID) / sizeof(DrvExtHdr->Driver.Bind.PCI.DeviceID[0]); Didx++)
                    {
                        if (Vidx >= sizeof(DrvExtHdr->Driver.Bind.PCI.VendorID) && Didx >= sizeof(DrvExtHdr->Driver.Bind.PCI.DeviceID))
                            break;

                        if (DrvExtHdr->Driver.Bind.PCI.VendorID[Vidx] == 0 || DrvExtHdr->Driver.Bind.PCI.DeviceID[Didx] == 0)
                            continue;

                        debug("VendorID: %#x; DeviceID: %#x", DrvExtHdr->Driver.Bind.PCI.VendorID[Vidx], DrvExtHdr->Driver.Bind.PCI.DeviceID[Didx]);

                        Vector<PCI::PCIDeviceHeader *> devices = PCIManager->FindPCIDevice(DrvExtHdr->Driver.Bind.PCI.VendorID[Vidx], DrvExtHdr->Driver.Bind.PCI.DeviceID[Didx]);
                        foreach (auto PCIDevice in devices)
                        {
                            Fex *fex = (Fex *)KernelAllocator.RequestPages(TO_PAGES(Size));
                            memcpy(fex, (void *)DriverAddress, Size);
                            FexExtended *fexExtended = (FexExtended *)((uint64_t)fex + EXTENDED_SECTION_ADDRESS);
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

                                memset(KCallback, 0, sizeof(KernelCallback));
                                KCallback->Reason = CallbackReason::InterruptReason;

                                DriverFile *drvfile = new DriverFile;
                                drvfile->DriverUID = KAPI.Info.DriverUID;
                                drvfile->Address = (void *)fex;
                                drvfile->InterruptHook = InterruptHook;
                                Drivers.push_back(drvfile);
                                break;
                            }
                            case FexDriverType::FexDriverType_Storage:
                            {
                                fixme("Storage driver: %s", fexExtended->Driver.Name);
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
            else
            {
                fixme("Driver bind type: %d", DrvExtHdr->Driver.Bind.Type);
            }
        }
        else
            return DriverCode::NOT_DRIVER;
        return DriverCode::OK;
    }

    Driver::Driver()
    {
        FileSystem::FILE *DriverDirectory = vfs->Open("/system/drivers");
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
        ((int (*)(void *))(Handle))(Data);
    }

    DriverInterruptHook::DriverInterruptHook(int Interrupt, void *Address, void *ParamData) : Interrupts::Handler(Interrupt)
    {
        trace("Interrupt %d Hooked", Interrupt - 32); // x86
        Handle = Address;
        Data = ParamData;
    }

    DriverInterruptHook::~DriverInterruptHook() {}
}
