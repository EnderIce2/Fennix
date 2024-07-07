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
#include <interface/aip.h>
#include <interface/input.h>
#include <interface/pci.h>

#include "../../kernel.h"

#define DEBUG_API

#ifdef DEBUG_API
#define dbg_api(Format, ...) func(Format, ##__VA_ARGS__)
#else
#define dbg_api(Format, ...)
#endif

namespace v0
{
	typedef int CriticalState;

	void KernelPrint(dev_t DriverID, const char *Format, va_list args)
	{
		dbg_api("%d, %s, %#lx", DriverID, Format, args);

		_KPrint(Format, args);
	}

	void KernelLog(dev_t DriverID, const char *Format, va_list args)
	{
		dbg_api("%d, %s, %#lx", DriverID, Format, args);

		fctprintf(uart_wrapper, nullptr, "DRVER| %ld: ", DriverID);
		vfctprintf(uart_wrapper, nullptr, Format, args);
		uart_wrapper('\n', nullptr);
	}

	/* --------- */

	CriticalState EnterCriticalSection(dev_t DriverID)
	{
		dbg_api("%d", DriverID);

		CriticalState cs;

#if defined(__i386__) || defined(__x86_64__)

		uintptr_t Flags;
#if defined(__x86_64__)
		asmv("pushfq");
		asmv("popq %0"
			 : "=r"(Flags));
#else
		asmv("pushfl");
		asmv("popl %0"
			 : "=r"(Flags));
#endif
		cs = Flags & (1 << 9);
		asmv("cli");

#elif defined(__arm__) || defined(__aarch64__)

		uintptr_t Flags;
		asmv("mrs %0, cpsr"
			 : "=r"(Flags));
		cs = Flags & (1 << 7);
		asmv("cpsid i");

#endif

		return cs;
	}

	void LeaveCriticalSection(dev_t DriverID, CriticalState PreviousState)
	{
		dbg_api("%d, %d", DriverID, PreviousState);

#if defined(__i386__) || defined(__x86_64__)

		if (PreviousState)
			asmv("sti");

#elif defined(__arm__) || defined(__aarch64__)

		if (PreviousState)
			asmv("cpsie i");

#endif
	}

	int RegisterInterruptHandler(dev_t DriverID, uint8_t IRQ, void *Handler)
	{
		dbg_api("%d, %d, %#lx", DriverID, IRQ, Handler);

		std::unordered_map<dev_t, Driver::DriverObject> &drivers =
			DriverManager->GetDrivers();
		const auto it = drivers.find(DriverID);
		if (it == drivers.end())
			ReturnLogError(-EINVAL, "Driver %d not found", DriverID);
		const Driver::DriverObject *drv = &it->second;

		if (drv->InterruptHandlers->contains(IRQ))
			return -EEXIST;

		Interrupts::AddHandler((void (*)(CPU::TrapFrame *))Handler, IRQ);
		auto ih = drv->InterruptHandlers;
		ih->insert(std::pair<uint8_t, void *>(IRQ, Handler));
		return 0;
	}

	int OverrideInterruptHandler(dev_t DriverID, uint8_t IRQ, void *Handler)
	{
		dbg_api("%d, %d, %#lx", DriverID, IRQ, Handler);

		debug("Overriding IRQ %d with %#lx", IRQ, Handler);

		std::unordered_map<dev_t, Driver::DriverObject> &drivers =
			DriverManager->GetDrivers();

		for (auto &var : drivers)
		{
			Driver::DriverObject *drv = &var.second;
			for (const auto &ih : *drv->InterruptHandlers)
			{
				if (ih.first != IRQ)
					continue;

				debug("Removing IRQ %d: %#lx for %s", IRQ, (uintptr_t)ih.second, drv->Path.c_str());
				Interrupts::RemoveHandler((void (*)(CPU::TrapFrame *))ih.second, IRQ);
				drv->InterruptHandlers->erase(IRQ);
				break;
			}
		}

		return RegisterInterruptHandler(DriverID, IRQ, Handler);
	}

	int UnregisterInterruptHandler(dev_t DriverID, uint8_t IRQ, void *Handler)
	{
		dbg_api("%d, %d, %#lx", DriverID, IRQ, Handler);

		std::unordered_map<dev_t, Driver::DriverObject> &drivers =
			DriverManager->GetDrivers();
		const auto it = drivers.find(DriverID);
		if (it == drivers.end())
			ReturnLogError(-EINVAL, "Driver %d not found", DriverID);
		const Driver::DriverObject *drv = &it->second;

		Interrupts::RemoveHandler((void (*)(CPU::TrapFrame *))Handler, IRQ);
		auto ih = drv->InterruptHandlers;
		ih->erase(IRQ);
		return 0;
	}

	int UnregisterAllInterruptHandlers(dev_t DriverID, void *Handler)
	{
		dbg_api("%d, %#lx", DriverID, Handler);

		std::unordered_map<dev_t, Driver::DriverObject> &drivers =
			DriverManager->GetDrivers();
		const auto it = drivers.find(DriverID);
		if (it == drivers.end())
			ReturnLogError(-EINVAL, "Driver %d not found", DriverID);
		const Driver::DriverObject *drv = &it->second;

		for (auto &i : *drv->InterruptHandlers)
		{
			Interrupts::RemoveHandler((void (*)(CPU::TrapFrame *))Handler, i.first);
			debug("Removed IRQ %d: %#lx for %s", i.first, (uintptr_t)Handler, drv->Path.c_str());
		}
		auto ih = drv->InterruptHandlers;
		ih->clear();
		return 0;
	}

	/* --------- */

	dev_t RegisterFileSystem(dev_t DriverID, FileSystemInfo *Info, struct Inode *Root)
	{
		dbg_api("%d, %#lx, %#lx", DriverID, Info, Root);

		return fs->RegisterFileSystem(Info, Root);
	}

	int UnregisterFileSystem(dev_t DriverID, dev_t Device)
	{
		dbg_api("%d, %d", DriverID, Device);

		return fs->UnregisterFileSystem(Device);
	}

	/* --------- */

	pid_t CreateKernelProcess(dev_t DriverID, const char *Name)
	{
		dbg_api("%d, %s", DriverID, Name);

		Tasking::PCB *pcb = TaskManager->CreateProcess(nullptr, Name, Tasking::System,
													   true, 0, 0);

		return pcb->ID;
	}

	pid_t CreateKernelThread(dev_t DriverID, pid_t pId, const char *Name, void *EntryPoint, void *Argument)
	{
		dbg_api("%d, %d, %s, %#lx, %#lx", DriverID, pId, Name, EntryPoint, Argument);

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

	pid_t GetCurrentProcess(dev_t DriverID)
	{
		dbg_api("%d", DriverID);

		return TaskManager->GetCurrentProcess()->ID;
	}

	int KillProcess(dev_t DriverID, pid_t pId, int ExitCode)
	{
		dbg_api("%d, %d, %d", DriverID, pId, ExitCode);

		Tasking::PCB *pcb = TaskManager->GetProcessByID(pId);
		if (!pcb)
			return -EINVAL;
		TaskManager->KillProcess(pcb, (Tasking::KillCode)ExitCode);
		return 0;
	}

	int KillThread(dev_t DriverID, pid_t tId, pid_t pId, int ExitCode)
	{
		dbg_api("%d, %d, %d", DriverID, tId, ExitCode);

		Tasking::TCB *tcb = TaskManager->GetThreadByID(tId, TaskManager->GetProcessByID(pId));
		if (!tcb)
			return -EINVAL;
		TaskManager->KillThread(tcb, (Tasking::KillCode)ExitCode);
		return 0;
	}

	void Yield(dev_t DriverID)
	{
		dbg_api("%d", DriverID);

		TaskManager->Yield();
	}

	void Sleep(dev_t DriverID, uint64_t Milliseconds)
	{
		dbg_api("%d, %d", DriverID, Milliseconds);

		TaskManager->Sleep(Milliseconds);
	}

	/* --------- */

	void PIC_EOI(dev_t DriverID, uint8_t IRQ)
	{
		dbg_api("%d, %d", DriverID, IRQ);

		if (IRQ >= 8)
			outb(PIC2_CMD, _PIC_EOI);
		outb(PIC1_CMD, _PIC_EOI);
	}

	void IRQ_MASK(dev_t DriverID, uint8_t IRQ)
	{
		dbg_api("%d, %d", DriverID, IRQ);

		uint16_t port;
		uint8_t value;

		if (IRQ < 8)
			port = PIC1_DATA;
		else
		{
			port = PIC2_DATA;
			IRQ -= 8;
		}

		value = inb(port) | (1 << IRQ);
		outb(port, value);
	}

	void IRQ_UNMASK(dev_t DriverID, uint8_t IRQ)
	{
		dbg_api("%d, %d", DriverID, IRQ);

		uint16_t port;
		uint8_t value;

		if (IRQ < 8)
			port = PIC1_DATA;
		else
		{
			port = PIC2_DATA;
			IRQ -= 8;
		}

		value = inb(port) & ~(1 << IRQ);
		outb(port, value);
	}

	void PS2Wait(dev_t DriverID, const bool Output)
	{
		dbg_api("%d, %d", DriverID, Output);

		int Timeout = 100000;
		PS2_STATUSES Status = {.Raw = inb(PS2_STATUS)};
		while (Timeout--)
		{
			if (!Output) /* FIXME: Reverse? */
			{
				if (Status.OutputBufferFull == 0)
					return;
			}
			else
			{
				if (Status.InputBufferFull == 0)
					return;
			}
			Status.Raw = inb(PS2_STATUS);
		}

		warn("PS/2 controller timeout! (Status: %#x, %d)", Status, Output);
	}

	void PS2WriteCommand(dev_t DriverID, uint8_t Command)
	{
		dbg_api("%d, %d", DriverID, Command);

		WaitInput;
		outb(PS2_CMD, Command);
	}

	void PS2WriteData(dev_t DriverID, uint8_t Data)
	{
		dbg_api("%d, %d", DriverID, Data);

		WaitInput;
		outb(PS2_DATA, Data);
	}

	uint8_t PS2ReadData(dev_t DriverID)
	{
		dbg_api("%d", DriverID);

		WaitOutput;
		return inb(PS2_DATA);
	}

	uint8_t PS2ReadStatus(dev_t DriverID)
	{
		dbg_api("%d", DriverID);

		WaitOutput;
		return inb(PS2_STATUS);
	}

	uint8_t PS2ReadAfterACK(dev_t DriverID)
	{
		dbg_api("%d", DriverID);

		uint8_t ret = PS2ReadData(DriverID);
		while (ret == PS2_ACK)
		{
			WaitOutput;
			ret = inb(PS2_DATA);
		}
		return ret;
	}

	void PS2ClearOutputBuffer(dev_t DriverID)
	{
		dbg_api("%d", DriverID);

		PS2_STATUSES Status;
		int timeout = 0x500;
		while (timeout--)
		{
			Status.Raw = inb(PS2_STATUS);
			if (Status.OutputBufferFull == 0)
				return;
			inb(PS2_DATA);
		}
	}

	int PS2ACKTimeout(dev_t DriverID)
	{
		dbg_api("%d", DriverID);

		int timeout = 0x500;
		while (timeout > 0)
		{
			if (PS2ReadData(DriverID) == PS2_ACK)
				return 0;
			timeout--;
		}
		return -ETIMEDOUT;
	}

	/* --------- */

	void *AllocateMemory(dev_t DriverID, size_t Pages)
	{
		dbg_api("%d, %d", DriverID, Pages);

		std::unordered_map<dev_t, Driver::DriverObject> &Drivers =
			DriverManager->GetDrivers();

		auto itr = Drivers.find(DriverID);
		assert(itr != Drivers.end());

		void *ptr = itr->second.vma->RequestPages(Pages);
		memset(ptr, 0, FROM_PAGES(Pages));
		return ptr;
	}

	void FreeMemory(dev_t DriverID, void *Pointer, size_t Pages)
	{
		dbg_api("%d, %#lx, %d", DriverID, Pointer, Pages);

		std::unordered_map<dev_t, Driver::DriverObject> &Drivers =
			DriverManager->GetDrivers();

		auto itr = Drivers.find(DriverID);
		assert(itr != Drivers.end());

		itr->second.vma->FreePages(Pointer, Pages);
	}

	void *MemoryCopy(dev_t DriverID, void *Destination, const void *Source, size_t Length)
	{
		dbg_api("%d, %#lx, %#lx, %d", DriverID, Destination, Source, Length);

		return memcpy(Destination, Source, Length);
	}

	void *MemorySet(dev_t DriverID, void *Destination, int Value, size_t Length)
	{
		dbg_api("%d, %#lx, %d, %d", DriverID, Destination, Value, Length);

		return memset(Destination, Value, Length);
	}

	void *MemoryMove(dev_t DriverID, void *Destination, const void *Source, size_t Length)
	{
		dbg_api("%d, %#lx, %#lx, %d", DriverID, Destination, Source, Length);

		return memmove(Destination, Source, Length);
	}

	size_t StringLength(dev_t DriverID, const char String[])
	{
		dbg_api("%d, %s", DriverID, String);

		return strlen(String);
	}

	char *_strstr(dev_t DriverID, const char *Haystack, const char *Needle)
	{
		dbg_api("%d, %s, %s", DriverID, Haystack, Needle);

		return (char *)strstr(Haystack, Needle);
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

	void *Znwm(size_t Size)
	{
		dbg_api("%d", Size);

		return malloc(Size);
	}

	void ZdlPvm(void *Pointer, size_t Size)
	{
		dbg_api("%d, %#lx", Pointer, Size);

		free(Pointer);
	}

	/* --------- */

	__PCIArray *GetPCIDevices(dev_t DriverID, uint16_t _Vendors[], uint16_t _Devices[])
	{
		dbg_api("%d, %#lx, %#lx", DriverID, _Vendors, _Devices);

		std::unordered_map<dev_t, Driver::DriverObject> &Drivers =
			DriverManager->GetDrivers();

		auto itr = Drivers.find(DriverID);
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

	void InitializePCI(dev_t DriverID, void *_Header)
	{
		dbg_api("%d, %#lx", DriverID, _Header);

		PCI::PCIDevice *__device = (PCI::PCIDevice *)_Header;
		PCI::PCIDeviceHeader *Header = (PCI::PCIDeviceHeader *)__device->Header;

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

	uint32_t GetBAR(dev_t DriverID, uint8_t i, void *_Header)
	{
		dbg_api("%d, %d, %#lx", DriverID, i, _Header);

		PCI::PCIDevice *__device = (PCI::PCIDevice *)_Header;
		PCI::PCIDeviceHeader *Header = (PCI::PCIDeviceHeader *)__device->Header;

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

	uint8_t iLine(dev_t DriverID, PCIDevice *Device)
	{
		dbg_api("%d, %#lx", DriverID, Device);

		PCIHeader0 *Header = (PCIHeader0 *)Device->Header;
		return Header->InterruptLine;
	}

	uint8_t iPin(dev_t DriverID, PCIDevice *Device)
	{
		dbg_api("%d, %#lx", DriverID, Device);

		PCIHeader0 *Header = (PCIHeader0 *)Device->Header;
		return Header->InterruptPin;
	}

	/* --------- */

	dev_t RegisterDevice(dev_t DriverID, DeviceType Type, const InodeOperations *Operations)
	{
		dbg_api("%d, %d, %#lx", DriverID, Type, Operations);

		return DriverManager->RegisterDevice(DriverID, Type, Operations);
	}

	int UnregisterDevice(dev_t DriverID, dev_t Device)
	{
		dbg_api("%d, %d", DriverID, Device);

		return DriverManager->UnregisterDevice(DriverID, Device);
	}

	int ReportInputEvent(dev_t DriverID, InputReport *Report)
	{
		dbg_api("%d, %#lx", DriverID, Report);

		return DriverManager->ReportInputEvent(DriverID, Report);
	}
}

struct APISymbols
{
	const char *Name;
	void *Function;
};

static struct APISymbols APISymbols_v0[] = {
	{"__KernelPrint", (void *)v0::KernelPrint},
	{"__KernelLog", (void *)v0::KernelLog},

	{"__EnterCriticalSection", (void *)v0::EnterCriticalSection},
	{"__LeaveCriticalSection", (void *)v0::LeaveCriticalSection},

	{"__RegisterInterruptHandler", (void *)v0::RegisterInterruptHandler},
	{"__OverrideInterruptHandler", (void *)v0::OverrideInterruptHandler},
	{"__UnregisterInterruptHandler", (void *)v0::UnregisterInterruptHandler},
	{"__UnregisterAllInterruptHandlers", (void *)v0::UnregisterAllInterruptHandlers},

	{"__RegisterFileSystem", (void *)v0::RegisterFileSystem},
	{"__UnregisterFileSystem", (void *)v0::UnregisterFileSystem},

	{"__CreateKernelProcess", (void *)v0::CreateKernelProcess},
	{"__CreateKernelThread", (void *)v0::CreateKernelThread},
	{"__GetCurrentProcess", (void *)v0::GetCurrentProcess},
	{"__KillProcess", (void *)v0::KillProcess},
	{"__KillThread", (void *)v0::KillThread},
	{"__Yield", (void *)v0::Yield},
	{"__Sleep", (void *)v0::Sleep},

	{"__PIC_EOI", (void *)v0::PIC_EOI},
	{"__IRQ_MASK", (void *)v0::IRQ_MASK},
	{"__IRQ_UNMASK", (void *)v0::IRQ_UNMASK},
	{"__PS2Wait", (void *)v0::PS2Wait},
	{"__PS2WriteCommand", (void *)v0::PS2WriteCommand},
	{"__PS2WriteData", (void *)v0::PS2WriteData},
	{"__PS2ReadData", (void *)v0::PS2ReadData},
	{"__PS2ReadStatus", (void *)v0::PS2ReadStatus},
	{"__PS2ReadAfterACK", (void *)v0::PS2ReadAfterACK},
	{"__PS2ClearOutputBuffer", (void *)v0::PS2ClearOutputBuffer},
	{"__PS2ACKTimeout", (void *)v0::PS2ACKTimeout},

	{"__AllocateMemory", (void *)v0::AllocateMemory},
	{"__FreeMemory", (void *)v0::FreeMemory},
	{"__MemoryCopy", (void *)v0::MemoryCopy},
	{"__MemorySet", (void *)v0::MemorySet},
	{"__MemoryMove", (void *)v0::MemoryMove},
	{"__StringLength", (void *)v0::StringLength},
	{"__strstr", (void *)v0::_strstr},
	{"__MapPages", (void *)v0::MapPages},
	{"__UnmapPages", (void *)v0::UnmapPages},
	{"__AppendMapFlag", (void *)v0::AppendMapFlag},
	{"__RemoveMapFlag", (void *)v0::RemoveMapFlag},
	{"_Znwm", (void *)v0::Znwm},
	{"_ZdlPvm", (void *)v0::ZdlPvm},

	{"__GetPCIDevices", (void *)v0::GetPCIDevices},
	{"__InitializePCI", (void *)v0::InitializePCI},
	{"__GetBAR", (void *)v0::GetBAR},
	{"__iLine", (void *)v0::iLine},
	{"__iPin", (void *)v0::iPin},

	{"__RegisterDevice", (void *)v0::RegisterDevice},
	{"__UnregisterDevice", (void *)v0::UnregisterDevice},
	{"__ReportInputEvent", (void *)v0::ReportInputEvent},
};

long __KernelUndefinedFunction(long arg0, long arg1, long arg2, long arg3,
							   long arg4, long arg5, long arg6, long arg7)
{
	debug("%#lx, %#lx, %#lx, %#lx, %#lx, %#lx, %#lx, %#lx",
		  arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
	assert(!"Undefined kernel driver API function called!");
	CPU::Stop();
}

void *GetSymbolByName(const char *Name, int Version)
{
	switch (Version)
	{
	case 0:
	{
		for (auto sym : APISymbols_v0)
		{
			if (strcmp(Name, sym.Name) != 0)
				continue;

			debug("Symbol %s found in API version %d", Name, Version);
			return sym.Function;
		}
		break;
	}
	default:
		assert(!"Invalid API version");
	}

	error("Symbol %s not found in API version %d", Name, Version);
	KPrint("Driver API symbol \"%s\" not found!", Name);
	return (void *)__KernelUndefinedFunction;
}
