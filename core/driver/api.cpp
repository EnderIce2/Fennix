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
#include <interface/driver.h>
#include <interface/fs.h>
#include <type_traits>

#include "../../kernel.h"

// #define DEBUG_API

#ifdef DEBUG_API
#define dbg_api(Format, ...) func(Format, ##__VA_ARGS__)
#else
#define dbg_api(Format, ...)
#endif

using enum PCI::PCICommands;

#define VMWARE_MAGIC 0x564D5868 /* hXMV */
#define VMWARE_PORT 0x5658
#define CMD_GETVERSION 0xA

namespace Driver
{
	int RegisterFunction(dev_t MajorID, void *Function, __driverRegFunc Type)
	{
		dbg_api("%d, %#lx, %d", MajorID, (uintptr_t)Function, Type);

		std::unordered_map<dev_t, DriverObject> &Drivers =
			DriverManager->GetDrivers();

		auto itr = Drivers.find(MajorID);
		if (itr == Drivers.end())
			return -EINVAL;

		DriverObject *drv = &itr->second;

		switch (Type)
		{
		case _drf_Entry:
			drv->Entry = (int (*)())Function;
			debug("Entry %#lx for %s", (uintptr_t)Function, drv->Path.c_str());
			break;
		case _drf_Final:
			drv->Final = (int (*)())Function;
			debug("Finalize %#lx for %s", (uintptr_t)Function, drv->Path.c_str());
			break;
		case _drf_Panic:
			drv->Panic = (int (*)())Function;
			debug("Panic %#lx for %s", (uintptr_t)Function, drv->Path.c_str());
			break;
		case _drf_Probe:
			drv->Probe = (int (*)())Function;
			debug("Probe %#lx for %s", (uintptr_t)Function, drv->Path.c_str());
			break;
		default:
			assert(!"Invalid driver function type");
		}
		return 0;
	}

	int GetDriverInfo(dev_t MajorID, const char *Name, const char *Description, const char *Author, const char *Version, const char *License)
	{
		dbg_api("%d, %s, %s, %s, %s, %s", MajorID, Name, Description, Author, Version, License);

		std::unordered_map<dev_t, DriverObject> &Drivers =
			DriverManager->GetDrivers();

		auto itr = Drivers.find(MajorID);
		if (itr == Drivers.end())
			return -EINVAL;

		DriverObject *drv = &itr->second;

		strncpy(drv->Name, Name, sizeof(drv->Name));
		strncpy(drv->Description, Description, sizeof(drv->Description));
		strncpy(drv->Author, Author, sizeof(drv->Author));
		strncpy(drv->Version, Version, sizeof(drv->Version));
		strncpy(drv->License, License, sizeof(drv->License));
		return 0;
	}

	/* --------- */

	int RegisterInterruptHandler(dev_t MajorID, uint8_t IRQ, void *Handler)
	{
		dbg_api("%d, %d, %#lx", MajorID, IRQ, Handler);

		std::unordered_map<dev_t, DriverObject> &Drivers =
			DriverManager->GetDrivers();

		auto itr = Drivers.find(MajorID);
		if (itr == Drivers.end())
			return -EINVAL;

		DriverObject *drv = &itr->second;

		if (drv->InterruptHandlers->contains(IRQ))
			return -EEXIST;

		Interrupts::AddHandler((void (*)(CPU::TrapFrame *))Handler, IRQ);
		drv->InterruptHandlers->insert(std::pair<uint8_t, void *>(IRQ, Handler));
		return 0;
	}

	int OverrideInterruptHandler(dev_t MajorID, uint8_t IRQ, void *Handler)
	{
		dbg_api("%d, %d, %#lx", MajorID, IRQ, Handler);

		debug("Overriding IRQ %d with %#lx", IRQ, Handler);

		std::unordered_map<dev_t, DriverObject> &Drivers =
			DriverManager->GetDrivers();

		foreach (auto &var in Drivers)
		{
			DriverObject *drv = &var.second;

			foreach (auto &ih in * drv->InterruptHandlers)
			{
				if (ih.first == IRQ)
				{
					debug("Removing IRQ %d: %#lx for %s", IRQ, (uintptr_t)ih.second, drv->Path.c_str());
					Interrupts::RemoveHandler((void (*)(CPU::TrapFrame *))ih.second, IRQ);
					drv->InterruptHandlers->erase(IRQ);
					break;
				}
			}
		}

		return RegisterInterruptHandler(MajorID, IRQ, Handler);
	}

	int UnregisterInterruptHandler(dev_t MajorID, uint8_t IRQ, void *Handler)
	{
		dbg_api("%d, %d, %#lx", MajorID, IRQ, Handler);

		std::unordered_map<dev_t, DriverObject> &Drivers =
			DriverManager->GetDrivers();

		auto itr = Drivers.find(MajorID);
		if (itr == Drivers.end())
			return -EINVAL;

		DriverObject *drv = &itr->second;
		Interrupts::RemoveHandler((void (*)(CPU::TrapFrame *))Handler, IRQ);
		drv->InterruptHandlers->erase(IRQ);
		return 0;
	}

	int UnregisterAllInterruptHandlers(dev_t MajorID, void *Handler)
	{
		dbg_api("%d, %#lx", MajorID, Handler);

		std::unordered_map<dev_t, DriverObject> &Drivers =
			DriverManager->GetDrivers();

		auto itr = Drivers.find(MajorID);
		if (itr == Drivers.end())
			return -EINVAL;

		DriverObject *drv = &itr->second;
		foreach (auto &i in * drv->InterruptHandlers)
		{
			Interrupts::RemoveHandler((void (*)(CPU::TrapFrame *))Handler, i.first);
			debug("Removed IRQ %d: %#lx for %s", i.first, (uintptr_t)Handler, drv->Path.c_str());
		}
		drv->InterruptHandlers->clear();
		return 0;
	}

	/* --------- */

	void d_KPrint(dev_t MajorID, const char *Format, va_list args)
	{
		dbg_api("%d %s, %#lx", MajorID, Format, args);

		_KPrint(Format, args);
	}

	void KernelLog(dev_t MajorID, const char *Format, va_list args)
	{
		dbg_api("%d, %s, %#lx", MajorID, Format, args);

		fctprintf(uart_wrapper, nullptr, "DRVER| %ld: ", MajorID);
		vfctprintf(uart_wrapper, nullptr, Format, args);
		uart_wrapper('\n', nullptr);
	}

	/* --------- */

	void *RequestPages(dev_t MajorID, size_t Pages)
	{
		dbg_api("%d, %d", MajorID, Pages);

		std::unordered_map<dev_t, DriverObject> &Drivers =
			DriverManager->GetDrivers();
		auto itr = Drivers.find(MajorID);
		assert(itr != Drivers.end());

		return itr->second.vma->RequestPages(Pages);
	}

	void FreePages(dev_t MajorID, void *Pointer, size_t Pages)
	{
		dbg_api("%d, %#lx, %d", MajorID, Pointer, Pages);

		std::unordered_map<dev_t, DriverObject> &Drivers =
			DriverManager->GetDrivers();

		auto itr = Drivers.find(MajorID);
		assert(itr != Drivers.end());

		itr->second.vma->FreePages(Pointer, Pages);
	}

	/* --------- */

	void AppendMapFlag(dev_t MajorID, void *Address, PageMapFlags Flag)
	{
		dbg_api("%d, %#lx, %d", MajorID, Address, Flag);

		Memory::Virtual vmm(KernelPageTable);
		vmm.GetPTE(Address)->raw |= Flag;
	}

	void RemoveMapFlag(dev_t MajorID, void *Address, PageMapFlags Flag)
	{
		dbg_api("%d, %#lx, %d", MajorID, Address, Flag);

		Memory::Virtual vmm(KernelPageTable);
		vmm.GetPTE(Address)->raw &= ~Flag;
	}

	void MapPages(dev_t MajorID, void *PhysicalAddress, void *VirtualAddress, size_t Pages, uint32_t Flags)
	{
		dbg_api("%d, %#lx, %#lx, %d, %d", MajorID, PhysicalAddress, VirtualAddress, Pages, Flags);

		Memory::Virtual vmm(KernelPageTable);
		vmm.Map(VirtualAddress, PhysicalAddress, Pages, Flags);
	}

	void UnmapPages(dev_t MajorID, void *VirtualAddress, size_t Pages)
	{
		dbg_api("%d, %#lx, %d", MajorID, VirtualAddress, Pages);

		Memory::Virtual vmm(KernelPageTable);
		vmm.Unmap(VirtualAddress, Pages);
	}

	/* --------- */

	pid_t CreateKernelProcess(dev_t MajorID, const char *Name)
	{
		dbg_api("%d, %s", MajorID, Name);

		Tasking::PCB *pcb = TaskManager->CreateProcess(nullptr, Name, Tasking::System,
													   true, 0, 0);

		return pcb->ID;
	}

	pid_t CreateKernelThread(dev_t MajorID, pid_t pId, const char *Name, void *EntryPoint, void *Argument)
	{
		dbg_api("%d, %d, %s, %#lx, %#lx", MajorID, pId, Name, EntryPoint, Argument);

		Tasking::PCB *parent = TaskManager->GetProcessByID(pId);
		if (!parent)
			return -EINVAL;

		CriticalSection cs;
		Tasking::TCB *tcb = TaskManager->CreateThread(parent, (Tasking::IP)EntryPoint);
		if (Argument)
			tcb->SYSV_ABI_Call((uintptr_t)Argument);
		tcb->Rename(Name);
		return tcb->ID;
	}

	pid_t GetCurrentProcess(dev_t MajorID)
	{
		dbg_api("%d", MajorID);

		return TaskManager->GetCurrentProcess()->ID;
	}

	int KillProcess(dev_t MajorID, pid_t pId, int ExitCode)
	{
		dbg_api("%d, %d, %d", MajorID, pId, ExitCode);

		Tasking::PCB *pcb = TaskManager->GetProcessByID(pId);
		if (!pcb)
			return -EINVAL;
		TaskManager->KillProcess(pcb, (Tasking::KillCode)ExitCode);
		return 0;
	}

	int KillThread(dev_t MajorID, pid_t tId, pid_t pId, int ExitCode)
	{
		dbg_api("%d, %d, %d", MajorID, tId, ExitCode);

		Tasking::TCB *tcb = TaskManager->GetThreadByID(tId, TaskManager->GetProcessByID(pId));
		if (!tcb)
			return -EINVAL;
		TaskManager->KillThread(tcb, (Tasking::KillCode)ExitCode);
		return 0;
	}

	void Yield(dev_t MajorID)
	{
		dbg_api("%d", MajorID);

		TaskManager->Yield();
	}

	void Sleep(dev_t MajorID, uint64_t Milliseconds)
	{
		dbg_api("%d, %d", MajorID, Milliseconds);

		TaskManager->Sleep(Milliseconds);
	}

	/* --------- */

	__PCIArray *GetPCIDevices(dev_t MajorID, uint16_t _Vendors[], uint16_t _Devices[])
	{
		dbg_api("%d, %#lx, %#lx", MajorID, _Vendors, _Devices);

		std::unordered_map<dev_t, DriverObject> &Drivers =
			DriverManager->GetDrivers();

		auto itr = Drivers.find(MajorID);
		if (itr == Drivers.end())
			return nullptr;

		std::list<uint16_t> VendorIDs;
		for (int i = 0; _Vendors[i] != 0x0; i++)
			VendorIDs.push_back(_Vendors[i]);

		std::list<uint16_t> DeviceIDs;
		for (int i = 0; _Devices[i] != 0x0; i++)
			DeviceIDs.push_back(_Devices[i]);

		std::list<PCI::PCIDevice> Devices = PCIManager->FindPCIDevice(VendorIDs, DeviceIDs);
		if (Devices.empty())
			return nullptr;

		Memory::VirtualMemoryArea *vma = itr->second.vma;
		__PCIArray *head = nullptr;
		__PCIArray *array = nullptr;

		foreach (auto &dev in Devices)
		{
			/* TODO: optimize memory allocation */
			PCI::PCIDevice *dptr = (PCI::PCIDevice *)vma->RequestPages(TO_PAGES(sizeof(PCI::PCIDevice)));
			memcpy(dptr, &dev, sizeof(PCI::PCIDevice));

			__PCIArray *newArray = (__PCIArray *)vma->RequestPages(TO_PAGES(sizeof(__PCIArray)));

			if (unlikely(head == nullptr))
			{
				head = newArray;
				array = head;
			}
			else
			{
				array->Next = newArray;
				array = newArray;
			}

			array->Device = dptr;
			array->Next = nullptr;

			debug("Found %02x.%02x.%02x: %04x:%04x",
				  dev.Bus, dev.Device, dev.Function,
				  dev.Header->VendorID, dev.Header->DeviceID);
		}

		return head;
	}

	void InitializePCI(dev_t MajorID, void *_Header)
	{
		dbg_api("%d, %#lx", MajorID, _Header);

		PCI::PCIDeviceHeader *Header = (PCI::PCIDeviceHeader *)_Header;

		debug("Header Type: %d", Header->HeaderType);
		switch (Header->HeaderType)
		{
		case 128:
			warn("Unknown header type %d! Guessing PCI Header 0",
				 Header->HeaderType);
			[[fallthrough]];
		case 0: /* PCI Header 0 */
		{
			PCI::PCIHeader0 *hdr0 = (PCI::PCIHeader0 *)Header;

			uint32_t BAR[6];
			size_t BARsSize[6];

			BAR[0] = hdr0->BAR0;
			BAR[1] = hdr0->BAR1;
			BAR[2] = hdr0->BAR2;
			BAR[3] = hdr0->BAR3;
			BAR[4] = hdr0->BAR4;
			BAR[5] = hdr0->BAR5;

			debug("Type: %d; IOBase: %#lx; MemoryBase: %#lx",
				  BAR[0] & 1, BAR[1] & (~3), BAR[0] & (~15));

			/* BARs Size */
			for (short i = 0; i < 6; i++)
			{
				if (BAR[i] == 0)
					continue;

				size_t size;
				if ((BAR[i] & 1) == 0) /* Memory Base */
				{
					hdr0->BAR0 = 0xFFFFFFFF;
					size = hdr0->BAR0;
					hdr0->BAR0 = BAR[i];
					BARsSize[i] = size & (~15);
					BARsSize[i] = ~BARsSize[i] + 1;
					BARsSize[i] = BARsSize[i] & 0xFFFFFFFF;
					debug("BAR%d %#lx size: %d",
						  i, BAR[i], BARsSize[i]);
				}
				else if ((BAR[i] & 1) == 1) /* I/O Base */
				{
					hdr0->BAR1 = 0xFFFFFFFF;
					size = hdr0->BAR1;
					hdr0->BAR1 = BAR[i];
					BARsSize[i] = size & (~3);
					BARsSize[i] = ~BARsSize[i] + 1;
					BARsSize[i] = BARsSize[i] & 0xFFFF;
					debug("BAR%d %#lx size: %d",
						  i, BAR[i], BARsSize[i]);
				}
			}

			Memory::Virtual vmm(KernelPageTable);

			/* Mapping the BARs */
			for (short i = 0; i < 6; i++)
			{
				if (BAR[i] == 0)
					continue;

				if ((BAR[i] & 1) == 0) /* Memory Base */
				{
					uintptr_t BARBase = BAR[i] & (~15);
					size_t BARSize = BARsSize[i];

					debug("Mapping BAR%d %#lx-%#lx",
						  i, BARBase, BARBase + BARSize);

					if (BARSize == 0)
					{
						warn("BAR%d size is zero!", i);
						BARSize++;
					}

					vmm.Map((void *)BARBase, (void *)BARBase,
							BARSize, Memory::RW | Memory::PWT | Memory::PCD);
				}
				else if ((BAR[i] & 1) == 1) /* I/O Base */
				{
					uintptr_t BARBase = BAR[i] & (~3);
					size_t BARSize = BARsSize[i];

					debug("Mapping BAR%d %#x-%#x",
						  i, BARBase, BARBase + BARSize);

					if (BARSize == 0)
					{
						warn("BAR%d size is zero!", i);
						BARSize++;
					}

					vmm.Map((void *)BARBase, (void *)BARBase,
							BARSize, Memory::RW | Memory::PWT | Memory::PCD);
				}
			}
			break;
		}
		case 1: /* PCI Header 1 (PCI-to-PCI Bridge) */
		{
			fixme("PCI Header 1 (PCI-to-PCI Bridge) not implemented yet");
			break;
		}
		case 2: /* PCI Header 2 (PCI-to-CardBus Bridge) */
		{
			fixme("PCI Header 2 (PCI-to-CardBus Bridge) not implemented yet");
			break;
		}
		default:
		{
			error("Unknown header type %d", Header->HeaderType);
			break;
		}
		}

		Header->Command |= PCI_COMMAND_MASTER |
						   PCI_COMMAND_IO |
						   PCI_COMMAND_MEMORY;
		Header->Command &= ~PCI_COMMAND_INTX_DISABLE;
	}

	uint32_t GetBAR(dev_t MajorID, uint8_t i, void *_Header)
	{
		dbg_api("%d, %d, %#lx", MajorID, i, _Header);

		PCI::PCIDeviceHeader *Header = (PCI::PCIDeviceHeader *)_Header;

		switch (Header->HeaderType)
		{
		case 128:
			warn("Unknown header type %d! Guessing PCI Header 0",
				 Header->HeaderType);
			[[fallthrough]];
		case 0: /* PCI Header 0 */
		{
			PCI::PCIHeader0 *hdr0 =
				(PCI::PCIHeader0 *)Header;
			switch (i)
			{
			case 0:
				return hdr0->BAR0;
			case 1:
				return hdr0->BAR1;
			case 2:
				return hdr0->BAR2;
			case 3:
				return hdr0->BAR3;
			case 4:
				return hdr0->BAR4;
			case 5:
				return hdr0->BAR5;
			default:
				assert(!"Invalid BAR index");
			}
		}
		case 1: /* PCI Header 1 (PCI-to-PCI Bridge) */
		{
			PCI::PCIHeader1 *hdr1 =
				(PCI::PCIHeader1 *)Header;
			switch (i)
			{
			case 0:
				return hdr1->BAR0;
			case 1:
				return hdr1->BAR1;
			default:
				assert(!"Invalid BAR index");
			}
		}
		case 2: /* PCI Header 2 (PCI-to-CardBus Bridge) */
		{
			assert(!"PCI-to-CardBus Bridge not supported");
		}
		default:
			assert(!"Invalid PCI header type");
		}
	}

	/* --------- */

	void *api__memcpy(dev_t MajorID, void *Destination, const void *Source, size_t Length)
	{
		dbg_api("%d, %#lx, %#lx, %d", MajorID, Destination, Source, Length);

		return memcpy(Destination, Source, Length);
	}

	void *api__memset(dev_t MajorID, void *Destination, int Value, size_t Length)
	{
		dbg_api("%d, %#lx, %d, %d", MajorID, Destination, Value, Length);

		return memset(Destination, Value, Length);
	}

	void *api__memmove(dev_t MajorID, void *Destination, const void *Source, size_t Length)
	{
		dbg_api("%d, %#lx, %#lx, %d", MajorID, Destination, Source, Length);

		return memmove(Destination, Source, Length);
	}

	int api__memcmp(dev_t MajorID, const void *Left, const void *Right, size_t Length)
	{
		dbg_api("%d, %#lx, %#lx, %d", MajorID, Left, Right, Length);

		return memcmp(Left, Right, Length);
	}

	size_t api__strlen(dev_t MajorID, const char *String)
	{
		dbg_api("%d, %s", MajorID, String);

		return strlen(String);
	}

	char *api__strcpy(dev_t MajorID, char *Destination, const char *Source)
	{
		dbg_api("%d, %#lx, %s", MajorID, Destination, Source);

		return strcpy(Destination, Source);
	}

	char *api__strcat(dev_t MajorID, char *Destination, const char *Source)
	{
		dbg_api("%d, %#lx, %s", MajorID, Destination, Source);

		return strcat(Destination, Source);
	}

	int api__strcmp(dev_t MajorID, const char *Left, const char *Right)
	{
		dbg_api("%d, %s, %s", MajorID, Left, Right);

		return strcmp(Left, Right);
	}

	int api__strncmp(dev_t MajorID, const char *Left, const char *Right, size_t Length)
	{
		dbg_api("%d, %s, %s, %d", MajorID, Left, Right, Length);

		return strncmp(Left, Right, Length);
	}

	char *api__strchr(dev_t MajorID, const char *String, int Character)
	{
		dbg_api("%d, %s, %d", MajorID, String, Character);

		return strchr(String, Character);
	}

	char *api__strrchr(dev_t MajorID, const char *String, int Character)
	{
		dbg_api("%d, %s, %d", MajorID, String, Character);

		stub;
		return nullptr;
		// return strrchr(String, Character);
	}

	char *api__strstr(dev_t MajorID, const char *Haystack, const char *Needle)
	{
		dbg_api("%d, %s, %s", MajorID, Haystack, Needle);

		return strstr(Haystack, Needle);
	}

	/* --------- */

	void PopulateDriverAPI(void *API)
	{
		__driverAPI *api = (__driverAPI *)API;

		api->RegisterFunction = RegisterFunction;
		api->GetDriverInfo = GetDriverInfo;

		api->RegisterInterruptHandler = RegisterInterruptHandler;
		api->OverrideInterruptHandler = OverrideInterruptHandler;
		api->UnregisterInterruptHandler = UnregisterInterruptHandler;
		api->UnregisterAllInterruptHandlers = UnregisterAllInterruptHandlers;

		api->KPrint = d_KPrint;
		api->KernelLog = KernelLog;

		api->RequestPages = RequestPages;
		api->FreePages = FreePages;

		api->AppendMapFlag = AppendMapFlag;
		api->RemoveMapFlag = RemoveMapFlag;
		api->MapPages = MapPages;
		api->UnmapPages = UnmapPages;

		api->CreateKernelProcess = CreateKernelProcess;
		api->CreateKernelThread = CreateKernelThread;
		api->GetCurrentProcess = GetCurrentProcess;
		api->KillProcess = KillProcess;
		api->KillThread = KillThread;
		api->Yield = Yield;
		api->Sleep = Sleep;

		api->GetPCIDevices = GetPCIDevices;
		api->InitializePCI = InitializePCI;
		api->GetBAR = GetBAR;

		api->memcpy = api__memcpy;
		api->memset = api__memset;
		api->memmove = api__memmove;
		api->memcmp = api__memcmp;
		api->strlen = api__strlen;
		api->strcpy = api__strcpy;
		api->strcat = api__strcat;
		api->strcmp = api__strcmp;
		api->strncmp = api__strncmp;
		api->strchr = api__strchr;
		api->strrchr = api__strrchr;
		api->strstr = api__strstr;
	}
}

dev_t __api_RegisterFileSystem(FileSystemInfo *Info, struct Inode *Root)
{
	return fs->RegisterFileSystem(Info, Root);
}

int __api_UnregisterFileSystem(dev_t Device)
{
	return fs->UnregisterFileSystem(Device);
}

struct APISymbols
{
	const char *Name;
	void *Function;
};

static struct APISymbols APISymbols[] = {
	{"RegisterFileSystem", (void *)__api_RegisterFileSystem},
	{"UnregisterFileSystem", (void *)__api_UnregisterFileSystem},
};

/* Checking functions signatures */
static_assert(std::is_same_v<decltype(__api_RegisterFileSystem), decltype(RegisterFileSystem)>);
static_assert(std::is_same_v<decltype(__api_UnregisterFileSystem), decltype(UnregisterFileSystem)>);
