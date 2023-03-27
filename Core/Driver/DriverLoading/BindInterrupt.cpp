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
    DriverCode Driver::BindInterruptGeneric(Memory::MemMgr *mem, void *fex)
    {
        FexExtended *fexExtended = (FexExtended *)((uintptr_t)fex + EXTENDED_SECTION_ADDRESS);

        if (fexExtended->Driver.OverrideOnConflict)
        {
            std::vector<uint64_t> DriversToRemove = std::vector<uint64_t>();
            foreach (auto Drv in Drivers)
            {
                FexExtended *fe = ((FexExtended *)((uintptr_t)Drv.Address + EXTENDED_SECTION_ADDRESS));

                if (fe->Driver.OverrideOnConflict)
                {
                    debug("Driver %s is conflicting with %s", fe->Driver.Name, fexExtended->Driver.Name);
                    return DriverCode::DRIVER_CONFLICT;
                }

                DriversToRemove.push_back(Drv.DriverUID);
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
                FexExtended *fe = ((FexExtended *)((uintptr_t)Drv.Address + EXTENDED_SECTION_ADDRESS));

                if (fe->Driver.OverrideOnConflict)
                {
                    debug("Driver %s is conflicting with %s", fe->Driver.Name, fexExtended->Driver.Name);
                    return DriverCode::DRIVER_CONFLICT;
                }
            }
        }

        fixme("Generic driver: %s", fexExtended->Driver.Name);
        DriverFile DrvFile = {
            .Enabled = true,
            .DriverUID = this->DriverUIDs - 1,
            .Address = (void *)fex,
            .InterruptCallback = (void *)((uintptr_t)fex + (uintptr_t)fexExtended->Driver.InterruptCallback),
            .MemTrk = mem,
        };
        Drivers.push_back(DrvFile);
        return DriverCode::OK;
    }

    DriverCode Driver::BindInterruptDisplay(Memory::MemMgr *mem, void *fex)
    {
        FexExtended *fexExtended = (FexExtended *)((uintptr_t)fex + EXTENDED_SECTION_ADDRESS);

        if (fexExtended->Driver.OverrideOnConflict)
        {
            std::vector<uint64_t> DriversToRemove = std::vector<uint64_t>();
            foreach (auto Drv in Drivers)
            {
                FexExtended *fe = ((FexExtended *)((uintptr_t)Drv.Address + EXTENDED_SECTION_ADDRESS));

                if (fe->Driver.OverrideOnConflict)
                {
                    debug("Driver %s is conflicting with %s", fe->Driver.Name, fexExtended->Driver.Name);
                    return DriverCode::DRIVER_CONFLICT;
                }

                DriversToRemove.push_back(Drv.DriverUID);
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
                FexExtended *fe = ((FexExtended *)((uintptr_t)Drv.Address + EXTENDED_SECTION_ADDRESS));

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

    DriverCode Driver::BindInterruptNetwork(Memory::MemMgr *mem, void *fex)
    {
        FexExtended *fexExtended = (FexExtended *)((uintptr_t)fex + EXTENDED_SECTION_ADDRESS);

        if (fexExtended->Driver.OverrideOnConflict)
        {
            std::vector<uint64_t> DriversToRemove = std::vector<uint64_t>();
            foreach (auto Drv in Drivers)
            {
                FexExtended *fe = ((FexExtended *)((uintptr_t)Drv.Address + EXTENDED_SECTION_ADDRESS));

                if (fe->Driver.OverrideOnConflict)
                {
                    debug("Driver %s is conflicting with %s", fe->Driver.Name, fexExtended->Driver.Name);
                    return DriverCode::DRIVER_CONFLICT;
                }

                DriversToRemove.push_back(Drv.DriverUID);
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
                FexExtended *fe = ((FexExtended *)((uintptr_t)Drv.Address + EXTENDED_SECTION_ADDRESS));

                if (fe->Driver.OverrideOnConflict)
                {
                    debug("Driver %s is conflicting with %s", fe->Driver.Name, fexExtended->Driver.Name);
                    return DriverCode::DRIVER_CONFLICT;
                }
            }
        }

        fixme("Network driver: %s", fexExtended->Driver.Name);
        delete mem, mem = nullptr;
        return DriverCode::NOT_IMPLEMENTED;
    }

    DriverCode Driver::BindInterruptStorage(Memory::MemMgr *mem, void *fex)
    {
        return DriverCode::NOT_IMPLEMENTED; // FIXME
        FexExtended *fexExtended = (FexExtended *)((uintptr_t)fex + EXTENDED_SECTION_ADDRESS);

        if (fexExtended->Driver.OverrideOnConflict)
        {
            std::vector<uint64_t> DriversToRemove = std::vector<uint64_t>();
            foreach (auto Drv in Drivers)
            {
                FexExtended *fe = ((FexExtended *)((uintptr_t)Drv.Address + EXTENDED_SECTION_ADDRESS));

                if (fe->Driver.OverrideOnConflict)
                {
                    debug("Driver %s is conflicting with %s", fe->Driver.Name, fexExtended->Driver.Name);
                    return DriverCode::DRIVER_CONFLICT;
                }

                DriversToRemove.push_back(Drv.DriverUID);
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
                FexExtended *fe = ((FexExtended *)((uintptr_t)Drv.Address + EXTENDED_SECTION_ADDRESS));

                if (fe->Driver.OverrideOnConflict)
                {
                    debug("Driver %s is conflicting with %s", fe->Driver.Name, fexExtended->Driver.Name);
                    return DriverCode::DRIVER_CONFLICT;
                }
            }
        }
        DriverFile DrvFile = {
            .Enabled = true,
            .DriverUID = this->DriverUIDs - 1,
            .Address = (void *)fex,
            .InterruptCallback = (void *)((uintptr_t)fex + (uintptr_t)fexExtended->Driver.InterruptCallback),
            .MemTrk = mem,
        };
        if (fexExtended->Driver.InterruptCallback)
        {
            for (unsigned long i = 0; i < sizeof(fexExtended->Driver.Bind.Interrupt.Vector) / sizeof(fexExtended->Driver.Bind.Interrupt.Vector[0]); i++)
            {
                if (fexExtended->Driver.Bind.Interrupt.Vector[i] == 0)
                    break;
                DrvFile.InterruptHook[i] = new DriverInterruptHook(fexExtended->Driver.Bind.Interrupt.Vector[i], DrvFile);
            }
        }

        KernelCallback KCallback = {.raw = 0};
        KCallback.RawPtr = nullptr;
        KCallback.Reason = CallbackReason::ConfigurationReason;
        int CallbackRet = ((int (*)(KernelCallback *))((uintptr_t)fexExtended->Driver.Callback + (uintptr_t)fex))(&KCallback);

        if (CallbackRet == DriverReturnCode::NOT_IMPLEMENTED)
        {
            error("Driver %s is not implemented", fexExtended->Driver.Name);
            delete mem, mem = nullptr;
            return DriverCode::NOT_IMPLEMENTED;
        }
        else if (CallbackRet != DriverReturnCode::OK)
        {
            error("Driver %s returned error %d", fexExtended->Driver.Name, CallbackRet);
            delete mem, mem = nullptr;
            return DriverCode::DRIVER_RETURNED_ERROR;
        }

        Drivers.push_back(DrvFile);
        return DriverCode::OK;
    }

    DriverCode Driver::BindInterruptFileSystem(Memory::MemMgr *mem, void *fex)
    {
        FexExtended *fexExtended = (FexExtended *)((uintptr_t)fex + EXTENDED_SECTION_ADDRESS);

        if (fexExtended->Driver.OverrideOnConflict)
        {
            std::vector<uint64_t> DriversToRemove = std::vector<uint64_t>();
            foreach (auto Drv in Drivers)
            {
                FexExtended *fe = ((FexExtended *)((uintptr_t)Drv.Address + EXTENDED_SECTION_ADDRESS));

                if (fe->Driver.OverrideOnConflict)
                {
                    debug("Driver %s is conflicting with %s", fe->Driver.Name, fexExtended->Driver.Name);
                    return DriverCode::DRIVER_CONFLICT;
                }

                DriversToRemove.push_back(Drv.DriverUID);
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
                FexExtended *fe = ((FexExtended *)((uintptr_t)Drv.Address + EXTENDED_SECTION_ADDRESS));

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

    DriverCode Driver::BindInterruptInput(Memory::MemMgr *mem, void *fex)
    {
        FexExtended *fexExtended = (FexExtended *)((uintptr_t)fex + EXTENDED_SECTION_ADDRESS);

        debug("Searching for conflicting drivers...");
        if (fexExtended->Driver.OverrideOnConflict)
        {
            std::vector<uint64_t> DriversToRemove = std::vector<uint64_t>();
            foreach (auto Drv in Drivers)
            {
                FexExtended *fe = ((FexExtended *)((uintptr_t)Drv.Address + EXTENDED_SECTION_ADDRESS));

                if ((fe->Driver.TypeFlags & FexDriverInputTypes_Mouse &&
                     fexExtended->Driver.TypeFlags & FexDriverInputTypes_Mouse) ||
                    (fe->Driver.TypeFlags & FexDriverInputTypes_Keyboard &&
                     fexExtended->Driver.TypeFlags & FexDriverInputTypes_Keyboard))
                {
                    debug("Driver %s is conflicting with %s", fe->Driver.Name, fexExtended->Driver.Name);
                    if (fe->Driver.OverrideOnConflict)
                        return DriverCode::DRIVER_CONFLICT;

                    DriversToRemove.push_back(Drv.DriverUID);
                }
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
                FexExtended *fe = ((FexExtended *)((uintptr_t)Drv.Address + EXTENDED_SECTION_ADDRESS));

                if ((fe->Driver.TypeFlags & FexDriverInputTypes_Mouse &&
                     fexExtended->Driver.TypeFlags & FexDriverInputTypes_Mouse) ||
                    (fe->Driver.TypeFlags & FexDriverInputTypes_Keyboard &&
                     fexExtended->Driver.TypeFlags & FexDriverInputTypes_Keyboard))
                {
                    debug("Driver %s is conflicting with %s", fe->Driver.Name, fexExtended->Driver.Name);
                    if (fe->Driver.OverrideOnConflict)
                        return DriverCode::DRIVER_CONFLICT;
                }
            }
        }

        DriverFile DrvFile = {
            .Enabled = true,
            .DriverUID = this->DriverUIDs - 1,
            .Address = (void *)fex,
            .InterruptCallback = (void *)((uintptr_t)fex + (uintptr_t)fexExtended->Driver.InterruptCallback),
            .MemTrk = mem,
        };
        if (fexExtended->Driver.InterruptCallback)
        {
            for (unsigned long i = 0; i < sizeof(fexExtended->Driver.Bind.Interrupt.Vector) / sizeof(fexExtended->Driver.Bind.Interrupt.Vector[0]); i++)
            {
                if (fexExtended->Driver.Bind.Interrupt.Vector[i] == 0)
                    break;
                DrvFile.InterruptHook[i] = new DriverInterruptHook(fexExtended->Driver.Bind.Interrupt.Vector[i], DrvFile);
            }
        }

        KernelCallback KCallback = {.raw = 0};
        KCallback.RawPtr = nullptr;
        KCallback.Reason = CallbackReason::ConfigurationReason;
        int CallbackRet = ((int (*)(KernelCallback *))((uintptr_t)fexExtended->Driver.Callback + (uintptr_t)fex))(&KCallback);

        if (CallbackRet == DriverReturnCode::NOT_IMPLEMENTED)
        {
            error("Driver %s is not implemented", fexExtended->Driver.Name);
            delete mem, mem = nullptr;
            return DriverCode::NOT_IMPLEMENTED;
        }
        else if (CallbackRet != DriverReturnCode::OK)
        {
            error("Driver %s returned error %d", fexExtended->Driver.Name, CallbackRet);
            delete mem, mem = nullptr;
            return DriverCode::DRIVER_RETURNED_ERROR;
        }

        Drivers.push_back(DrvFile);
        return DriverCode::OK;
    }

    DriverCode Driver::BindInterruptAudio(Memory::MemMgr *mem, void *fex)
    {
        FexExtended *fexExtended = (FexExtended *)((uintptr_t)fex + EXTENDED_SECTION_ADDRESS);

        if (fexExtended->Driver.OverrideOnConflict)
        {
            std::vector<uint64_t> DriversToRemove = std::vector<uint64_t>();
            foreach (auto Drv in Drivers)
            {
                FexExtended *fe = ((FexExtended *)((uintptr_t)Drv.Address + EXTENDED_SECTION_ADDRESS));

                if (fe->Driver.OverrideOnConflict)
                {
                    debug("Driver %s is conflicting with %s", fe->Driver.Name, fexExtended->Driver.Name);
                    return DriverCode::DRIVER_CONFLICT;
                }

                DriversToRemove.push_back(Drv.DriverUID);
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
                FexExtended *fe = ((FexExtended *)((uintptr_t)Drv.Address + EXTENDED_SECTION_ADDRESS));

                if (fe->Driver.OverrideOnConflict)
                {
                    debug("Driver %s is conflicting with %s", fe->Driver.Name, fexExtended->Driver.Name);
                    return DriverCode::DRIVER_CONFLICT;
                }
            }
        }

        fixme("Audio driver: %s", fexExtended->Driver.Name);
        delete mem, mem = nullptr;
        return DriverCode::NOT_IMPLEMENTED;
    }

    DriverCode Driver::DriverLoadBindInterrupt(void *DrvExtHdr, uintptr_t DriverAddress, size_t Size, bool IsElf)
    {
        UNUSED(IsElf);
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
        debug("Starting driver %s (offset: %#lx)", fexExtended->Driver.Name, fex);

        switch (fexExtended->Driver.Type)
        {
        case FexDriverType::FexDriverType_Generic:
            return BindInterruptGeneric(mem, fex);
        case FexDriverType::FexDriverType_Display:
            return BindInterruptDisplay(mem, fex);
        case FexDriverType::FexDriverType_Network:
            return BindInterruptNetwork(mem, fex);
        case FexDriverType::FexDriverType_Storage:
            return BindInterruptStorage(mem, fex);
        case FexDriverType::FexDriverType_FileSystem:
            return BindInterruptFileSystem(mem, fex);
        case FexDriverType::FexDriverType_Input:
            return BindInterruptInput(mem, fex);
        case FexDriverType::FexDriverType_Audio:
            return BindInterruptAudio(mem, fex);
        default:
        {
            warn("Unknown driver type: %d", fexExtended->Driver.Type);
            delete mem, mem = nullptr;
            return DriverCode::UNKNOWN_DRIVER_TYPE;
        }
        }

        return DriverCode::OK;
    }
}
