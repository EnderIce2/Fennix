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
#include <printf.h>
#include <cwalk.h>
#include <md5.h>

#include "../../Modules/drv.hpp"
#include "../../kernel.h"
#include "../../DAPI.hpp"
#include "../../Fex.hpp"
#include "api.hpp"

namespace Driver
{
	void Driver::Panic()
	{
		debug("%ld drivers loaded, [DriverUIDs: %ld]", Drivers.size(), DriverUIDs - 1);

		foreach (auto Drv in Drivers)
		{
			KernelCallback callback{};
			callback.Reason = StopReason;
			DriverManager->IOCB(Drv.DriverUID, &callback);

			for (size_t j = 0; j < sizeof(Drv.InterruptHook) / sizeof(Drv.InterruptHook[0]); j++)
			{
				if (!Drv.InterruptHook[j])
					continue;

				Drv.InterruptHook[j]->Disable();
				debug("Interrupt hook %#lx disabled", Drv.InterruptHook[j]);
			}
		}
	}

	void Driver::UnloadAllDrivers()
	{
		debug("%ld drivers loaded, [DriverUIDs: %ld]", Drivers.size(), DriverUIDs - 1);

		foreach (auto Drv in Drivers)
		{
			KernelCallback callback{};
			callback.Reason = StopReason;
			debug("Stopping & unloading driver %ld [%#lx]", Drv.DriverUID, Drv.Address);
			DriverManager->IOCB(Drv.DriverUID, &callback);

			for (size_t j = 0; j < sizeof(Drv.InterruptHook) / sizeof(Drv.InterruptHook[0]); j++)
			{
				if (!Drv.InterruptHook[j])
					continue;

				debug("Interrupt hook %#lx", Drv.InterruptHook[j]);
				delete Drv.InterruptHook[j], Drv.InterruptHook[j] = nullptr;
			}

			if (Drv.MemTrk)
				delete Drv.MemTrk, Drv.MemTrk = nullptr;
		}
		Drivers.clear();
	}

	bool Driver::UnloadDriver(unsigned long DUID)
	{
		debug("Searching for driver %ld", DUID);

		forItr(Drv, Drivers)
		{
			if (Drv->DriverUID != DUID)
				continue;

			KernelCallback callback{};
			callback.Reason = StopReason;
			debug("Stopping & unloading driver %ld [%#lx]", Drv->DriverUID, Drv->Address);
			this->IOCB(Drv->DriverUID, &callback);

			for (size_t j = 0; j < sizeof(Drv->InterruptHook) / sizeof(Drv->InterruptHook[0]); j++)
			{
				if (!Drv->InterruptHook[j])
					continue;

				debug("Interrupt hook %#lx", Drv->InterruptHook[j]);
				delete Drv->InterruptHook[j], Drv->InterruptHook[j] = nullptr;
			}

			if (Drv->MemTrk)
				delete Drv->MemTrk, Drv->MemTrk = nullptr;

			Drivers.erase(Drv);
			return true;
		}
		return false;
	}

	int Driver::IOCB(unsigned long DUID, void *KCB)
	{
		foreach (auto Drv in Drivers)
		{
			if (Drv.DriverUID != DUID)
				continue;

			FexExtended *fexE = (FexExtended *)Drv.ExtendedHeaderAddress;
			return ((int (*)(void *))((uintptr_t)fexE->Driver.Callback + (uintptr_t)Drv.Address))(KCB);
		}
		return -1;
	}

	DriverCode Driver::CallDriverEntryPoint(void *fex, bool BuiltIn)
	{
		DriverCode ret{};
		KernelAPI DriverKAPI = KernelAPITemplate;

		DriverKAPI.Info.DriverUID = DriverUIDs++;
		DriverKAPI.Info.KernelDebug = DebuggerIsAttached;

		debug("Calling driver entry point ( %#lx %ld )", (unsigned long)fex, DriverKAPI.Info.DriverUID);

		if (!BuiltIn)
		{
			DriverKAPI.Info.Offset = (unsigned long)fex;

			debug("DRIVER: %s HAS DRIVER ID %ld",
				  ((FexExtended *)((uintptr_t)fex + EXTENDED_SECTION_ADDRESS))->Driver.Name,
				  DriverKAPI.Info.DriverUID);
			ret = ((DriverCode(*)(KernelAPI *))((uintptr_t)((Fex *)fex)->EntryPoint + (uintptr_t)fex))(((KernelAPI *)&DriverKAPI));
		}
		else
		{
			debug("DRIVER: BUILTIN HAS DRIVER ID %ld", DriverKAPI.Info.DriverUID);
			ret = ((DriverCode(*)(KernelAPI *))((uintptr_t)fex))(((KernelAPI *)&DriverKAPI));
		}

		if (DriverCode::OK != ret)
		{
			DriverUIDs--;
			return ret;
		}
		return DriverCode::OK;
	}

	DriverCode Driver::LoadDriver(uintptr_t DriverAddress, size_t Size)
	{
		Fex *DrvHdr = (Fex *)DriverAddress;
		if (DrvHdr->Magic[0] != 'F' || DrvHdr->Magic[1] != 'E' || DrvHdr->Magic[2] != 'X' || DrvHdr->Magic[3] != '\0')
			return DriverCode::INVALID_FEX_HEADER;

		debug("Fex Magic: \"%s\"; Type: %d; OS: %d; EntryPoint: %#lx", DrvHdr->Magic, DrvHdr->Type, DrvHdr->OS, DrvHdr->EntryPoint);

		if (DrvHdr->Type != FexFormatType::FexFormatType_Driver)
			return DriverCode::NOT_DRIVER;

		FexExtended *fexE = (FexExtended *)((uintptr_t)DrvHdr + EXTENDED_SECTION_ADDRESS);
		debug("Name: \"%s\"; Type: %d; Callback: %#lx", fexE->Driver.Name, fexE->Driver.Type, fexE->Driver.Callback);

		switch (fexE->Driver.Bind.Type)
		{
		case DriverBindType::BIND_PCI:
			return this->DriverLoadBindPCI(DriverAddress, Size);
		case DriverBindType::BIND_INTERRUPT:
			return this->DriverLoadBindInterrupt(DriverAddress, Size);
		case DriverBindType::BIND_PROCESS:
			return this->DriverLoadBindProcess(DriverAddress, Size);
		case DriverBindType::BIND_INPUT:
			return this->DriverLoadBindInput(DriverAddress, Size);
		default:
		{
			error("Unknown driver bind type: %d", fexE->Driver.Bind.Type);
			return DriverCode::UNKNOWN_DRIVER_BIND_TYPE;
		}
		}
	}

	void Driver::LoadDrivers()
	{
		SmartCriticalSection(DriverInitLock);

		std::string DriverConfigFile = Config.DriverDirectory;
		DriverConfigFile << "/config.ini";
		fixme("Loading driver config file: %s", DriverConfigFile.c_str());

		debug("Loading built-in drivers");
		StartAHCI();
		StartVMwareMouse();
		StartPS2Mouse();
		StartPS2Keyboard();
		StartATA();
		StartAC97();
		StartRTL8139();
		StartPCNET();
		StartGigabit();

		RefNode *DriverDirectory = vfs->Open(Config.DriverDirectory);
		if (!DriverDirectory)
		{
			KPrint("\eE85230Failed to open %s: %d)",
				   Config.DriverDirectory, errno);
			return;
		}

		debug("Loading drivers from %s", Config.DriverDirectory);
		foreach (auto DrvFile in DriverDirectory->GetNode()->Children)
		{
			if (DrvFile->Flags != VirtualFileSystem::NodeFlags::FILE)
				continue;

			if (cwk_path_has_extension(DrvFile->Name))
			{
				const char *extension;
				size_t extension_length;
				cwk_path_get_extension(DrvFile->Name, &extension, &extension_length);
				debug("File: %s; Extension: %s", DrvFile->Name, extension);
				if (strcmp(extension, ".fex") == 0)
				{
					uintptr_t ret = this->LoadDriver(DrvFile->Address, DrvFile->Length);
					std::string RetString;
					if (ret == DriverCode::OK)
						RetString << "\e058C19OK";
					else if (ret == DriverCode::NOT_AVAILABLE)
						RetString << "\eFF7900NOT AVAILABLE";
					else
						RetString << "\eE85230FAILED";
					KPrint("%s %s %#lx", DrvFile->Name, RetString.c_str(), ret);
				}
			}
		}
		delete DriverDirectory;
	}

	Driver::Driver() {}

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
		if (!this->Enabled)
		{
			debug("Interrupt hook is not enabled");
			return;
		}

		if (!Handle.InterruptCallback)
		{
#if defined(a86)
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
#if defined(a86)
		trace("Interrupt %d hooked to driver %ld", Interrupt, Handle.DriverUID);
#elif defined(aa64)
		trace("Interrupt %d hooked to driver %ld", Interrupt, Handle.DriverUID);
#endif
	}
}
