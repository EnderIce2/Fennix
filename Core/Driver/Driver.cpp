#include <driver.hpp>

#include <interrupts.hpp>
#include <memory.hpp>
#include <task.hpp>
#include <lock.hpp>
#include <printf.h>
#include <cwalk.h>
#include <md5.h>

#include "../../kernel.h"
#include "../../DAPI.hpp"
#include "../../Fex.hpp"
#include "api.hpp"

NewLock(DriverInitLock);
NewLock(DriverInterruptLock);

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

    void Driver::UnloadAllDrivers()
    {
        KernelCallback callback;
        debug("%ld drivers loaded, [DUIDs: %ld]", Drivers.size(), DriverUIDs);
        debug("driver size %ld", Drivers.size());
        for (size_t i = 0; i < Drivers.size(); i++)
        {
            DriverFile *drv = Drivers[i];
            memset(&callback, 0, sizeof(KernelCallback));
            callback.Reason = StopReason;
            debug("Stopping & unloading driver %ld [%#lx]", drv->DriverUID, drv->Address);
            DriverManager->IOCB(drv->DriverUID, (void *)&callback);

            delete drv->MemTrk;
            for (size_t i = 0; i < sizeof(drv->InterruptHook) / sizeof(drv->InterruptHook[0]); i++)
            {
                if (!drv->InterruptHook[i])
                    continue;
                delete drv->InterruptHook[i];
            }
            Drivers.remove(i);
        }
    }

    bool Driver::UnloadDriver(unsigned long DUID)
    {
        for (size_t i = 0; i < Drivers.size(); i++)
        {
            DriverFile *drv = Drivers[i];
            if (drv->DriverUID == DUID)
            {
                KernelCallback callback;
                memset(&callback, 0, sizeof(KernelCallback));
                callback.Reason = StopReason;
                debug("Stopping & unloading driver %ld [%#lx]", drv->DriverUID, drv->Address);
                DriverManager->IOCB(drv->DriverUID, (void *)&callback);

                delete drv->MemTrk;
                for (size_t i = 0; i < sizeof(drv->InterruptHook) / sizeof(drv->InterruptHook[0]); i++)
                {
                    if (!drv->InterruptHook[i])
                        continue;
                    delete drv->InterruptHook[i];
                }
                Drivers.remove(i);
                return true;
            }
        }
        return false;
    }

    int Driver::IOCB(unsigned long DUID, void *KCB)
    {
        foreach (auto var in Drivers)
            if (var->DriverUID == DUID)
            {
                FexExtended *DrvExtHdr = (FexExtended *)((uintptr_t)var->Address + EXTENDED_SECTION_ADDRESS);
                return ((int (*)(void *))((uintptr_t)DrvExtHdr->Driver.Callback + (uintptr_t)var->Address))(KCB);
            }
        return -1;
    }

    DriverCode Driver::CallDriverEntryPoint(void *fex, void *KAPIAddress)
    {
        memcpy(KAPIAddress, &KernelAPITemplate, sizeof(KernelAPI));

        ((KernelAPI *)KAPIAddress)->Info.Offset = (unsigned long)fex;
        ((KernelAPI *)KAPIAddress)->Info.DriverUID = DriverUIDs++;

        debug("Calling driver entry point ( %#lx %ld )", (unsigned long)fex, ((KernelAPI *)KAPIAddress)->Info.DriverUID);
        int ret = ((int (*)(KernelAPI *))((uintptr_t)((Fex *)fex)->EntryPoint + (uintptr_t)fex))(((KernelAPI *)KAPIAddress));

        if (DriverReturnCode::OK != ret)
            return DriverCode::DRIVER_RETURNED_ERROR;
        return DriverCode::OK;
    }

    DriverCode Driver::LoadDriver(uintptr_t DriverAddress, uintptr_t Size)
    {
        Fex *DrvHdr = (Fex *)DriverAddress;
        if (DrvHdr->Magic[0] != 'F' || DrvHdr->Magic[1] != 'E' || DrvHdr->Magic[2] != 'X' || DrvHdr->Magic[3] != '\0')
        {
            if (Size > 0x1000)
            {
                Fex *ElfDrvHdr = (Fex *)(DriverAddress + 0x1000);
                if (ElfDrvHdr->Magic[0] != 'F' || ElfDrvHdr->Magic[1] != 'E' || ElfDrvHdr->Magic[2] != 'X' || ElfDrvHdr->Magic[3] != '\0')
                    return DriverCode::INVALID_FEX_HEADER;
                else
                {
                    debug("Fex Magic: \"%s\"; Type: %d; OS: %d; EntryPoint: %#lx", ElfDrvHdr->Magic, ElfDrvHdr->Type, ElfDrvHdr->OS, ElfDrvHdr->EntryPoint);

                    if (ElfDrvHdr->Type == FexFormatType::FexFormatType_Driver)
                    {
                        FexExtended *ElfDrvExtHdr = (FexExtended *)((uintptr_t)ElfDrvHdr + EXTENDED_SECTION_ADDRESS);
                        debug("Name: \"%s\"; Type: %d; Callback: %#lx", ElfDrvExtHdr->Driver.Name, ElfDrvExtHdr->Driver.Type, ElfDrvExtHdr->Driver.Callback);

                        if (ElfDrvExtHdr->Driver.Bind.Type == DriverBindType::BIND_PCI)
                            return this->DriverLoadBindPCI(ElfDrvExtHdr, DriverAddress, Size, true);
                        else if (ElfDrvExtHdr->Driver.Bind.Type == DriverBindType::BIND_INTERRUPT)
                            return this->DriverLoadBindInterrupt(ElfDrvExtHdr, DriverAddress, Size, true);
                        else if (ElfDrvExtHdr->Driver.Bind.Type == DriverBindType::BIND_PROCESS)
                            return this->DriverLoadBindProcess(ElfDrvExtHdr, DriverAddress, Size, true);
                        else if (ElfDrvExtHdr->Driver.Bind.Type == DriverBindType::BIND_INPUT)
                            return this->DriverLoadBindInput(ElfDrvExtHdr, DriverAddress, Size, true);
                        else
                            error("Unknown driver bind type: %d", ElfDrvExtHdr->Driver.Bind.Type);
                    }
                    else
                        return DriverCode::NOT_DRIVER;
                }
            }
            else
                return DriverCode::INVALID_FEX_HEADER;
        }
        debug("Fex Magic: \"%s\"; Type: %d; OS: %d; EntryPoint: %#lx", DrvHdr->Magic, DrvHdr->Type, DrvHdr->OS, DrvHdr->EntryPoint);

        if (DrvHdr->Type == FexFormatType::FexFormatType_Driver)
        {
            FexExtended *DrvExtHdr = (FexExtended *)((uintptr_t)DrvHdr + EXTENDED_SECTION_ADDRESS);
            debug("Name: \"%s\"; Type: %d; Callback: %#lx", DrvExtHdr->Driver.Name, DrvExtHdr->Driver.Type, DrvExtHdr->Driver.Callback);

            if (DrvExtHdr->Driver.Bind.Type == DriverBindType::BIND_PCI)
                return this->DriverLoadBindPCI(DrvExtHdr, DriverAddress, Size);
            else if (DrvExtHdr->Driver.Bind.Type == DriverBindType::BIND_INTERRUPT)
                return this->DriverLoadBindInterrupt(DrvExtHdr, DriverAddress, Size);
            else if (DrvExtHdr->Driver.Bind.Type == DriverBindType::BIND_PROCESS)
                return this->DriverLoadBindProcess(DrvExtHdr, DriverAddress, Size);
            else if (DrvExtHdr->Driver.Bind.Type == DriverBindType::BIND_INPUT)
                return this->DriverLoadBindInput(DrvExtHdr, DriverAddress, Size);
            else
                error("Unknown driver bind type: %d", DrvExtHdr->Driver.Bind.Type);
        }
        else
            return DriverCode::NOT_DRIVER;
        return DriverCode::ERROR;
    }

    Driver::Driver()
    {
        SmartCriticalSection(DriverInitLock);
        shared_ptr<VirtualFileSystem::File> DriverDirectory = vfs->Open(Config.DriverDirectory);
        if (DriverDirectory->Status == VirtualFileSystem::FileStatus::OK)
        {
            foreach (auto driver in DriverDirectory->node->Children)
                if (driver->Flags == VirtualFileSystem::NodeFlags::FILE)
                    if (cwk_path_has_extension(driver->Name))
                    {
                        const char *extension;
                        cwk_path_get_extension(driver->Name, &extension, nullptr);
                        debug("Driver: %s; Extension: %s", driver->Name, extension);
                        if (strcmp(extension, ".fex") == 0 || strcmp(extension, ".elf") == 0)
                        {
                            uintptr_t ret = this->LoadDriver(driver->Address, driver->Length);
                            char RetString[128];
                            if (ret == DriverCode::OK)
                                strncpy(RetString, "\e058C19OK", 10);
                            else if (ret == DriverCode::NOT_AVAILABLE)
                                strncpy(RetString, "\eFF7900NOT AVAILABLE", 21);
                            else
                                sprintf(RetString, "\eE85230FAILED (%#lx)", ret);
                            KPrint("%s %s", driver->Name, RetString);
                        }
                    }
        }
        else
        {
            KPrint("\eE85230Failed to open driver directory: %s", Config.DriverDirectory);
            CPU::Stop();
        }
        vfs->Close(DriverDirectory);
    }

    Driver::~Driver()
    {
        debug("Destructor called");
        this->UnloadAllDrivers();
    }

#if defined(__amd64__)
    SafeFunction void DriverInterruptHook::OnInterruptReceived(CPU::x64::TrapFrame *Frame)
#elif defined(__i386__)
    SafeFunction void DriverInterruptHook::OnInterruptReceived(void *Frame)
#elif defined(__aarch64__)
    SafeFunction void DriverInterruptHook::OnInterruptReceived(void *Frame)
#endif
    {
        SmartCriticalSection(DriverInterruptLock);
        ((int (*)(void *))(Handle))(Data);
        UNUSED(Frame);
    }

    DriverInterruptHook::DriverInterruptHook(int Interrupt, void *Address, void *ParamData) : Interrupts::Handler(Interrupt)
    {
        trace("Interrupt %d Hooked", Interrupt - 32); // x86
        Handle = Address;
        Data = ParamData;
    }
}
