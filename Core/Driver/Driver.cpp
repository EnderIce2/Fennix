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

#include <driver.hpp>

#include <memory.hpp>
#include <ints.hpp>
#include <task.hpp>
#include <lock.hpp>
#include <printf.h>
#include <cwalk.h>
#include <md5.h>
// #include <ini.h>

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

    void Driver::Panic()
    {
        size_t DriversNum = Drivers.size();
        debug("%ld drivers loaded, [DUIDs: %ld]", DriversNum, DriverUIDs);
        debug("driver size %ld", DriversNum);
        for (size_t i = 0; i < DriversNum; i++)
        {
            DriverFile drv = Drivers[i];
            KernelCallback callback;
            callback.Reason = StopReason;
            debug("Removing interrupt hook for %ld [%#lx]", drv.DriverUID, drv.Address);
            DriverManager->IOCB(drv.DriverUID, (void *)&callback);

            for (size_t j = 0; j < sizeof(drv.InterruptHook) / sizeof(drv.InterruptHook[0]); j++)
            {
                if (!drv.InterruptHook[j])
                    continue;
                delete drv.InterruptHook[j], drv.InterruptHook[j] = nullptr;
            }
        }
    }

    void Driver::UnloadAllDrivers()
    {
        size_t DriversNum = Drivers.size();
        debug("%ld drivers loaded, [DUIDs: %ld]", DriversNum, DriverUIDs);
        debug("driver size %ld", DriversNum);
        for (size_t i = 0; i < DriversNum; i++)
        {
            DriverFile drv = Drivers[i];
            KernelCallback callback;
            callback.Reason = StopReason;
            debug("Stopping & unloading driver %ld [%#lx]", drv.DriverUID, drv.Address);
            DriverManager->IOCB(drv.DriverUID, (void *)&callback);

            for (size_t j = 0; j < sizeof(drv.InterruptHook) / sizeof(drv.InterruptHook[0]); j++)
            {
                if (!drv.InterruptHook[j])
                    break;
                delete drv.InterruptHook[j], drv.InterruptHook[j] = nullptr;
            }
            if (drv.MemTrk)
                delete drv.MemTrk, drv.MemTrk = nullptr;
            Drivers.remove(i);
        }
    }

    bool Driver::UnloadDriver(unsigned long DUID)
    {
        debug("Searching for driver %ld", DUID);
        for (size_t i = 0; i < Drivers.size(); i++)
        {
            DriverFile drv = Drivers[i];
            if (drv.DriverUID == DUID)
            {
                KernelCallback callback;
                callback.Reason = StopReason;
                debug("Stopping and unloading driver %ld [%#lx]", drv.DriverUID, drv.Address);
                this->IOCB(drv.DriverUID, (void *)&callback);

                for (size_t i = 0; i < sizeof(drv.InterruptHook) / sizeof(drv.InterruptHook[0]); i++)
                {
                    if (!drv.InterruptHook[i])
                        break;
                    delete drv.InterruptHook[i], drv.InterruptHook[i] = nullptr;
                }
                delete drv.MemTrk, drv.MemTrk = nullptr;
                Drivers.remove(i);
                return true;
            }
        }
        return false;
    }

    int Driver::IOCB(unsigned long DUID, void *KCB)
    {
        foreach (auto Drv in Drivers)
        {
            if (Drv.DriverUID == DUID)
            {
                FexExtended *DrvExtHdr = (FexExtended *)((uintptr_t)Drv.Address + EXTENDED_SECTION_ADDRESS);
                int ret = ((int (*)(void *))((uintptr_t)DrvExtHdr->Driver.Callback + (uintptr_t)Drv.Address))(KCB);
                __sync;
                return ret;
            }
        }
        return -1;
    }

    DriverCode Driver::CallDriverEntryPoint(void *fex, void *KAPIAddress)
    {
        memcpy(KAPIAddress, &KernelAPITemplate, sizeof(KernelAPI));

        ((KernelAPI *)KAPIAddress)->Info.Offset = (unsigned long)fex;
        ((KernelAPI *)KAPIAddress)->Info.DriverUID = DriverUIDs++;

#ifdef DEBUG
        FexExtended *fexExtended = (FexExtended *)((uintptr_t)fex + EXTENDED_SECTION_ADDRESS);
        debug("DRIVER: %s HAS DRIVER ID %ld", fexExtended->Driver.Name, ((KernelAPI *)KAPIAddress)->Info.DriverUID);
#endif

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

        std::string DriverConfigFile = Config.DriverDirectory;
        DriverConfigFile << "/config.ini";
        fixme("Loading driver config file: %s", DriverConfigFile.c_str());

        std::shared_ptr<VirtualFileSystem::File> DriverDirectory = vfs->Open(Config.DriverDirectory);
        if (DriverDirectory->Status == VirtualFileSystem::FileStatus::OK)
        {
            foreach (auto driver in DriverDirectory->node->Children)
                if (driver->Flags == VirtualFileSystem::NodeFlags::FILE)
                    if (cwk_path_has_extension(driver->Name))
                    {
                        const char *extension;
                        size_t extension_length;
                        cwk_path_get_extension(driver->Name, &extension, &extension_length);
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
            KPrint("\eE85230Failed to open driver directory: %s! (Status: %#lx)", Config.DriverDirectory, DriverDirectory->Status);
            CPU::Stop();
        }
        vfs->Close(DriverDirectory);
    }

    Driver::~Driver()
    {
        debug("Destructor called");
        this->UnloadAllDrivers();
    }

#if defined(a64)
    SafeFunction void DriverInterruptHook::OnInterruptReceived(CPU::x64::TrapFrame *Frame)
#elif defined(a32)
    SafeFunction void DriverInterruptHook::OnInterruptReceived(CPU::x32::TrapFrame *Frame)
#elif defined(aa64)
    SafeFunction void DriverInterruptHook::OnInterruptReceived(CPU::aarch64::TrapFrame *Frame)
#endif
    {
        SmartLock(DriverInterruptLock); /* Lock in case of multiple interrupts firing at the same time */
        if (!Handle.InterruptCallback)
        {
#if defined(a64) || defined(a32)
            uint64_t IntNum = Frame->InterruptNumber - 32;
#elif defined(aa64)
            uint64_t IntNum = Frame->InterruptNumber;
#endif
            warn("Interrupt callback for %ld is not set for driver %ld!", IntNum, Handle.DriverUID);
            return;
        }
        CPURegisters regs;
#if defined(a64)
        regs.r15 = Frame->r15;
        regs.r14 = Frame->r14;
        regs.r13 = Frame->r13;
        regs.r12 = Frame->r12;
        regs.r11 = Frame->r11;
        regs.r10 = Frame->r10;
        regs.r9 = Frame->r9;
        regs.r8 = Frame->r8;

        regs.rbp = Frame->rbp;
        regs.rdi = Frame->rdi;
        regs.rsi = Frame->rsi;
        regs.rdx = Frame->rdx;
        regs.rcx = Frame->rcx;
        regs.rbx = Frame->rbx;
        regs.rax = Frame->rax;

        regs.InterruptNumber = Frame->InterruptNumber;
        regs.ErrorCode = Frame->ErrorCode;
        regs.rip = Frame->rip;
        regs.cs = Frame->cs;
        regs.rflags = Frame->rflags.raw;
        regs.rsp = Frame->rsp;
        regs.ss = Frame->ss;
#elif defined(a32)
        regs.ebp = Frame->ebp;
        regs.edi = Frame->edi;
        regs.esi = Frame->esi;
        regs.edx = Frame->edx;
        regs.ecx = Frame->ecx;
        regs.ebx = Frame->ebx;
        regs.eax = Frame->eax;

        regs.InterruptNumber = Frame->InterruptNumber;
        regs.ErrorCode = Frame->ErrorCode;
        regs.eip = Frame->eip;
        regs.cs = Frame->cs;
        regs.eflags = Frame->eflags.raw;
        regs.esp = Frame->esp;
        regs.ss = Frame->ss;
#elif defined(aa64)
#endif
        ((int (*)(void *))(Handle.InterruptCallback))(&regs);
        UNUSED(Frame);
    }

    DriverInterruptHook::DriverInterruptHook(int Interrupt, DriverFile Handle) : Interrupts::Handler(Interrupt)
    {
        this->Handle = Handle;
#if defined(a64) || defined(a32)
        trace("Interrupt %d hooked to driver %ld", Interrupt, Handle.DriverUID);
#elif defined(aa64)
        trace("Interrupt %d hooked to driver %ld", Interrupt, Handle.DriverUID);
#endif
    }
}
