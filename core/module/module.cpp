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

#include <module.hpp>

#include <memory.hpp>
#include <ints.hpp>
#include <task.hpp>
#include <printf.h>
#include <cwalk.h>
#include <md5.h>

#include "../../modules/mod.hpp"
#include "../../kernel.h"
#include "../../mapi.hpp"
#include "../../Fex.hpp"
#include "api.hpp"

using vfs::RefNode;

namespace Module
{
	void Module::Panic()
	{
		debug("%ld modules loaded, [modUIDs: %ld]", Modules.size(), modUIDs - 1);

		foreach (auto Drv in Modules)
		{
			KernelCallback callback{};
			callback.Reason = StopReason;
			ModuleManager->IOCB(Drv.modUniqueID, &callback);

			for (size_t j = 0; j < sizeof(Drv.InterruptHook) / sizeof(Drv.InterruptHook[0]); j++)
			{
				if (!Drv.InterruptHook[j])
					continue;

				Drv.InterruptHook[j]->Disable();
				debug("Interrupt hook %#lx disabled", Drv.InterruptHook[j]);
			}
		}
	}

	void Module::UnloadAllModules()
	{
		debug("%ld modules loaded, [modUIDs: %ld]", Modules.size(), modUIDs - 1);

		foreach (auto Drv in Modules)
		{
			KernelCallback callback{};
			callback.Reason = StopReason;
			debug("Stopping & unloading module %ld [%#lx]", Drv.modUniqueID, Drv.Address);
			ModuleManager->IOCB(Drv.modUniqueID, &callback);

			for (size_t j = 0; j < sizeof(Drv.InterruptHook) / sizeof(Drv.InterruptHook[0]); j++)
			{
				if (!Drv.InterruptHook[j])
					continue;

				debug("Interrupt hook %#lx", Drv.InterruptHook[j]);
				delete Drv.InterruptHook[j], Drv.InterruptHook[j] = nullptr;
			}

			if (Drv.vma)
				delete Drv.vma, Drv.vma = nullptr;
		}
		Modules.clear();
	}

	bool Module::UnloadModule(unsigned long id)
	{
		debug("Searching for module %ld", id);

		forItr(Drv, Modules)
		{
			if (Drv->modUniqueID != id)
				continue;

			KernelCallback callback{};
			callback.Reason = StopReason;
			debug("Stopping & unloading module %ld [%#lx]", Drv->modUniqueID, Drv->Address);
			this->IOCB(Drv->modUniqueID, &callback);

			for (size_t j = 0; j < sizeof(Drv->InterruptHook) / sizeof(Drv->InterruptHook[0]); j++)
			{
				if (!Drv->InterruptHook[j])
					continue;

				debug("Interrupt hook %#lx", Drv->InterruptHook[j]);
				delete Drv->InterruptHook[j], Drv->InterruptHook[j] = nullptr;
			}

			if (Drv->vma)
				delete Drv->vma, Drv->vma = nullptr;

			Modules.erase(Drv);
			return true;
		}
		return false;
	}

	int Module::IOCB(unsigned long id, void *KCB)
	{
		foreach (auto Drv in Modules)
		{
			if (Drv.modUniqueID != id)
				continue;

			FexExtended *fexE = (FexExtended *)Drv.ExtendedHeaderAddress;
			return ((int (*)(void *))((uintptr_t)fexE->Module.Callback + (uintptr_t)Drv.Address))(KCB);
		}
		return -1;
	}

	ModuleCode Module::CallModuleEntryPoint(void *fex, bool BuiltIn)
	{
		ModuleCode ret{};
		KernelAPI modKAPI = KernelAPITemplate;

		modKAPI.Info.modUniqueID = modUIDs++;
		modKAPI.Info.KernelDebug = DebuggerIsAttached;

		debug("Calling module entry point ( %#lx %ld )", (unsigned long)fex, modKAPI.Info.modUniqueID);

		if (!BuiltIn)
		{
			modKAPI.Info.Offset = (unsigned long)fex;

			debug("MODULE: %s HAS MODULE ID %ld",
				  ((FexExtended *)((uintptr_t)fex + EXTENDED_SECTION_ADDRESS))->Module.Name,
				  modKAPI.Info.modUniqueID);
			ret = ((ModuleCode(*)(KernelAPI *))((uintptr_t)((Fex *)fex)->EntryPoint + (uintptr_t)fex))(((KernelAPI *)&modKAPI));
		}
		else
		{
			debug("MODULE: BUILTIN HAS MODULE ID %ld", modKAPI.Info.modUniqueID);
			ret = ((ModuleCode(*)(KernelAPI *))((uintptr_t)fex))(((KernelAPI *)&modKAPI));
		}

		if (ModuleCode::OK != ret)
		{
			modUIDs--;
			return ret;
		}
		return ModuleCode::OK;
	}

	ModuleCode Module::LoadModule(vfs::Node *fildes)
	{
		Fex DrvHdr;
		fildes->read((uint8_t *)&DrvHdr, sizeof(Fex), 0);

		if (DrvHdr.Magic[0] != 'F' || DrvHdr.Magic[1] != 'E' || DrvHdr.Magic[2] != 'X' || DrvHdr.Magic[3] != '\0')
			return ModuleCode::INVALID_FEX_HEADER;

		debug("Fex Magic: \"%s\"; Type: %d; OS: %d; EntryPoint: %#lx", DrvHdr.Magic, DrvHdr.Type, DrvHdr.OS, DrvHdr.EntryPoint);

		if (DrvHdr.Type != FexFormatType::FexFormatType_Module)
			return ModuleCode::NOT_MODULE;

		FexExtended fexE;
		fildes->read((uint8_t *)&fexE, sizeof(FexExtended), EXTENDED_SECTION_ADDRESS);

		debug("Name: \"%s\"; Type: %d; Callback: %#lx", fexE.Module.Name, fexE.Module.Type, fexE.Module.Callback);

		Memory::SmartHeap ModuleAddress(fildes->Size);
		fildes->read(ModuleAddress, fildes->Size, 0);

		switch (fexE.Module.Bind.Type)
		{
		case ModuleBindType::BIND_PCI:
			return this->ModuleLoadBindPCI(ModuleAddress, fildes->Size);
		case ModuleBindType::BIND_INTERRUPT:
			return this->ModuleLoadBindInterrupt(ModuleAddress, fildes->Size);
		case ModuleBindType::BIND_PROCESS:
			return this->ModuleLoadBindProcess(ModuleAddress, fildes->Size);
		case ModuleBindType::BIND_INPUT:
			return this->ModuleLoadBindInput(ModuleAddress, fildes->Size);
		default:
		{
			error("Unknown module bind type: %d", fexE.Module.Bind.Type);
			return ModuleCode::UNKNOWN_MODULE_BIND_TYPE;
		}
		}
	}

	void Module::LoadModules()
	{
		SmartCriticalSection(ModuleInitLock);

		const char *ModuleConfigFile = new char[256];
		assert(strlen(Config.ModuleDirectory) < 255 - 12);
		strcpy((char *)ModuleConfigFile, Config.ModuleDirectory);
		strcat((char *)ModuleConfigFile, "/config.ini");
		fixme("Loading module config file: %s", ModuleConfigFile);
		delete[] ModuleConfigFile;

		debug("Loading built-in modules");
		StartBuiltInModules();

		RefNode *ModuleDirectory = fs->Open(Config.ModuleDirectory);
		if (!ModuleDirectory)
		{
			KPrint("\eE85230Failed to open %s: %d)",
				   Config.ModuleDirectory, errno);
			return;
		}

		debug("Loading modules from %s", Config.ModuleDirectory);
		foreach (auto DrvFile in ModuleDirectory->node->Children)
		{
			if (DrvFile->Type != vfs::NodeType::FILE)
				continue;

			if (cwk_path_has_extension(DrvFile->Name))
			{
				const char *extension;
				size_t extension_length;
				cwk_path_get_extension(DrvFile->Name, &extension, &extension_length);
				debug("File: %s; Extension: %s", DrvFile->Name, extension);
				if (strcmp(extension, ".fex") == 0)
				{
					uintptr_t ret = this->LoadModule(DrvFile);
					char *RetString = new char[256];
					if (ret == ModuleCode::OK)
						strcpy(RetString, "\e058C19OK");
					else if (ret == ModuleCode::NOT_AVAILABLE)
						strcpy(RetString, "\eFF7900NOT AVAILABLE");
					else
						strcpy(RetString, "\eE85230FAILED");
					KPrint("%s %s %#lx", DrvFile->Name, RetString, ret);
					delete[] RetString;
				}
			}
		}
		delete ModuleDirectory;
	}

	Module::Module() {}

	Module::~Module()
	{
		debug("Destructor called");
		this->UnloadAllModules();
	}

#if defined(a64)
	SafeFunction void ModuleInterruptHook::OnInterruptReceived(CPU::x64::TrapFrame *Frame)
#elif defined(a32)
	SafeFunction void ModuleInterruptHook::OnInterruptReceived(CPU::x32::TrapFrame *Frame)
#elif defined(aa64)
	SafeFunction void ModuleInterruptHook::OnInterruptReceived(CPU::aarch64::TrapFrame *Frame)
#endif
	{
		SmartLock(DriverInterruptLock); /* Lock in case of multiple interrupts firing at the same time */
		if (!this->Enabled)
		{
			debug("Interrupt hook is not enabled (%#lx, IRQ%d)",
				  Frame->InterruptNumber,
				  Frame->InterruptNumber - 32);
			return;
		}

		if (!Handle.InterruptCallback)
		{
#if defined(a86)
			uint64_t IntNum = Frame->InterruptNumber - 32;
#elif defined(aa64)
			uint64_t IntNum = Frame->InterruptNumber;
#endif
			warn("Interrupt callback for %ld is not set for module %ld!",
				 IntNum, Handle.modUniqueID);
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
		regs.edi = Frame->edi;
		regs.esi = Frame->esi;
		regs.ebp = Frame->ebp;
		regs.esp = Frame->esp;
		regs.ebx = Frame->ebx;
		regs.edx = Frame->edx;
		regs.ecx = Frame->ecx;
		regs.eax = Frame->eax;

		regs.InterruptNumber = Frame->InterruptNumber;
		regs.ErrorCode = Frame->ErrorCode;
		regs.eip = Frame->eip;
		regs.cs = Frame->cs;
		regs.eflags = Frame->eflags.raw;
		regs.r3_esp = Frame->r3_esp;
		regs.r3_ss = Frame->r3_ss;
#elif defined(aa64)
#endif
		((int (*)(void *))(Handle.InterruptCallback))(&regs);
		UNUSED(Frame);
	}

	ModuleInterruptHook::ModuleInterruptHook(int Interrupt, ModuleFile Handle) : Interrupts::Handler(Interrupt)
	{
		this->Handle = Handle;
#if defined(a86)
		trace("Interrupt %d hooked to module %ld", Interrupt, Handle.modUniqueID);
#elif defined(aa64)
		trace("Interrupt %d hooked to module %ld", Interrupt, Handle.modUniqueID);
#endif
	}
}
