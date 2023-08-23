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

#ifndef __FENNIX_KERNEL_PCI_H__
#define __FENNIX_KERNEL_PCI_H__

#include <types.h>

#include <memory.hpp>
#include <debug.h>
#include <vector>

namespace PCI
{
	namespace Descriptors
	{
		enum PCIVendors
		{
			SymbiosLogic = 0x1000,
			RedHat = 0x1AF4,
			REDHat2 = 0x1B36,
			Realtek = 0x10EC,
			VirtualBox = 0x80EE,
			Ensoniq = 0x1274,
			QEMU = 0x1234,
			VMware = 0x15AD,
			IntelCorporation = 0x8086,
			AdvancedMicroDevices = 0x1022,
			NVIDIACorporation = 0x10DE
		};

		const char *const DeviceClasses[]{
			"Unclassified",
			"Mass Storage Controller",
			"Network Controller",
			"Display Controller",
			"Multimedia Controller",
			"Memory Controller",
			"Bridge Device",
			"Simple Communication Controller",
			"Base System Peripheral",
			"Input Device Controller",
			"Docking Station",
			"Processor",
			"Serial Bus Controller",
			"Wireless Controller",
			"Intelligent Controller",
			"Satellite Communication Controller",
			"Encryption Controller",
			"Signal Processing Controller",
			"Processing Accelerator",
			"Non Essential Instrumentation"};

		const char *MassStorageControllerSubclassName(uint8_t SubclassCode);
		const char *NetworkControllerSubclassName(uint8_t SubclassCode);
		const char *DisplayControllerSubclassName(uint8_t SubclassCode);
		const char *CommunicationControllerSubclassName(uint8_t SubclassCode);
		const char *BaseSystemPeripheralSubclassName(uint8_t SubclassCode);
		const char *SerialBusControllerSubclassName(uint8_t SubclassCode);
		const char *BridgeDeviceSubclassName(uint8_t SubclassCode);
		const char *WirelessControllerSubclassName(uint8_t SubclassCode);
		const char *GetVendorName(uint32_t VendorID);
		const char *GetDeviceName(uint32_t VendorID, uint32_t DeviceID);
		const char *GetSubclassName(uint8_t ClassCode, uint8_t SubclassCode);
		const char *GetProgIFName(uint8_t ClassCode, uint8_t SubclassCode, uint8_t ProgIF);
	}

	/* https://sites.uclouvain.be/SystInfo/usr/include/linux/pci_regs.h.html */
	enum PCICommands
	{
		/** @brief Enable response in I/O space */
		PCI_COMMAND_IO = 0x1,
		/** @brief Enable response in Memory space */
		PCI_COMMAND_MEMORY = 0x2,
		/** @brief Enable bus mastering */
		PCI_COMMAND_MASTER = 0x4,
		/** @brief Enable response to special cycles */
		PCI_COMMAND_SPECIAL = 0x8,
		/** @brief Use memory write and invalidate */
		PCI_COMMAND_INVALIDATE = 0x10,
		/** @brief Enable palette snooping */
		PCI_COMMAND_VGA_PALETTE = 0x20,
		/** @brief Enable parity checking */
		PCI_COMMAND_PARITY = 0x40,
		/** @brief Enable address/data stepping */
		PCI_COMMAND_WAIT = 0x80,
		/** @brief Enable SERR */
		PCI_COMMAND_SERR = 0x100,
		/** @brief Enable back-to-back writes */
		PCI_COMMAND_FAST_BACK = 0x200,
		/** @brief INTx Emulation Disable */
		PCI_COMMAND_INTX_DISABLE = 0x400
	};

	struct PCIDeviceHeader
	{
		uint16_t VendorID;
		uint16_t DeviceID;
		uint16_t Command;
		uint16_t Status;
		uint8_t RevisionID;
		uint8_t ProgIF;
		uint8_t Subclass;
		uint8_t Class;
		uint8_t CacheLineSize;
		uint8_t LatencyTimer;
		uint8_t HeaderType;
		uint8_t BIST;
	};

	/**
	 * @brief PCI Header Type 0
	 *
	 */
	struct PCIHeader0
	{
		PCIDeviceHeader Header;
		uint32_t BAR0;
		uint32_t BAR1;
		uint32_t BAR2;
		uint32_t BAR3;
		uint32_t BAR4;
		uint32_t BAR5;
		uint32_t CardbusCISPointer;
		uint16_t SubsystemVendorID;
		uint16_t SubsystemID;
		uint32_t ExpansionROMBaseAddress;
		uint8_t CapabilitiesPointer;
		uint8_t Reserved0;
		uint16_t Reserved1;
		uint32_t Reserved2;
		uint8_t InterruptLine;
		uint8_t InterruptPin;
		uint8_t MinGrant;
		uint8_t MaxLatency;
	};

	/**
	 * @brief PCI Header Type 1 (PCI-to-PCI Bridge)
	 */
	struct PCIHeader1
	{
		PCIDeviceHeader Header;
		uint32_t BAR0;
		uint32_t BAR1;
		uint8_t PrimaryBusNumber;
		uint8_t SecondaryBusNumber;
		uint8_t SubordinateBusNumber;
		uint8_t SecondaryLatencyTimer;
		uint8_t IOBase;
		uint8_t IOLimit;
		uint16_t SecondaryStatus;
		uint16_t MemoryBase;
		uint16_t MemoryLimit;
		uint16_t PrefetchableMemoryBase;
		uint16_t PrefetchableMemoryLimit;
		uint32_t PrefetchableMemoryBaseUpper32;
		uint32_t PrefetchableMemoryLimitUpper32;
		uint16_t IOBaseUpper16;
		uint16_t IOLimitUpper16;
		uint8_t CapabilitiesPointer;
		uint8_t Reserved0;
		uint16_t Reserved1;
		uint32_t ExpansionROMBaseAddress;
		uint8_t InterruptLine;
		uint8_t InterruptPin;
		uint16_t BridgeControl;
	};

	/**
	 * @brief PCI Header Type 2 (PCI-to-CardBus Bridge)
	 */
	struct PCIHeader2
	{
		PCIDeviceHeader Header;
		uint32_t CardbusSocketRegistersBaseAddress;
		uint8_t CapabilitiesPointer;
		uint8_t Reserved0;
		uint16_t SecondaryStatus;
		uint8_t PCIbusNumber;
		uint8_t CardbusBusNumber;
		uint8_t SubordinateBusNumber;
		uint8_t CardbusLatencyTimer;
		uint32_t MemoryBase0;
		uint32_t MemoryLimit0;
		uint32_t MemoryBase1;
		uint32_t MemoryLimit1;
		uint32_t IOBase0;
		uint32_t IOLimit0;
		uint32_t IOBase1;
		uint32_t IOLimit1;
		uint8_t InterruptLine;
		uint8_t InterruptPin;
		uint16_t BridgeControl;
		uint16_t SubsystemVendorID;
		uint16_t SubsystemID;
		uint32_t LegacyBaseAddress;
	};

	struct DeviceConfig
	{
		uint64_t BaseAddress;
		uint16_t PCISegGroup;
		uint8_t StartBus;
		uint8_t EndBus;
		uint32_t Reserved;
	} __packed;

	class PCI
	{
	private:
		std::vector<PCIDeviceHeader *> Devices;

	public:
		std::vector<PCIDeviceHeader *> &GetDevices() { return Devices; }
		void MapPCIAddresses(PCIDeviceHeader *PCIDevice, Memory::PageTable *Table = nullptr);
		void EnumerateFunction(uint64_t DeviceAddress, uintptr_t Function);
		void EnumerateDevice(uint64_t BusAddress, uintptr_t Device);
		void EnumerateBus(uint64_t BaseAddress, uintptr_t Bus);
		std::vector<PCIDeviceHeader *> FindPCIDevice(uint8_t Class, uint8_t Subclass, uint8_t ProgIF);
		std::vector<PCIDeviceHeader *> FindPCIDevice(int VendorID, int DeviceID);

		PCI();
		~PCI();
	};
}

#endif // !__FENNIX_KERNEL_PCI_H__
