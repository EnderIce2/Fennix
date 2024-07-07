/*
	This file is part of Fennix Drivers.

	Fennix Drivers is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Drivers is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Drivers. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef __FENNIX_API_PCI_H__
#define __FENNIX_API_PCI_H__

#include <types.h>

/* https://sites.uclouvain.be/SystInfo/usr/include/linux/pci_regs.h.html */
typedef enum
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
} PCI_COMMANDS;

typedef struct
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
} __attribute__((packed)) PCIDeviceHeader;

typedef struct
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
} __attribute__((packed)) PCIHeader0;

typedef struct
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
} __attribute__((packed)) PCIHeader1;

typedef struct
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
} __attribute__((packed)) PCIHeader2;

typedef struct
{
	uint64_t BaseAddress;
	uint16_t PCISegGroup;
	uint8_t StartBus;
	uint8_t EndBus;
	uint32_t Reserved;
} __attribute__((packed)) DeviceConfig;

typedef struct
{
	PCIDeviceHeader *Header;
	DeviceConfig *Config;
	uint32_t Bus;
	uint32_t Device;
	uint32_t Function;
} __attribute__((packed)) PCIDevice;

typedef struct
{
	PCIDevice *Device;
	/* PCIArray */ void *Next;
} __attribute__((packed)) PCIArray;

#ifdef __cplusplus
extern "C"
{
#endif

	PCIArray *GetPCIDevices(uint16_t Vendors[], uint16_t Devices[]);
	void InitializePCI(PCIDevice *Device);
	uint32_t GetBAR(uint8_t Index, PCIDevice *Device);
	uint8_t iLine(PCIDevice *Device);
	uint8_t iPin(PCIDevice *Device);

#ifdef __cplusplus
}
#endif

#endif // !__FENNIX_API_PCI_H__
