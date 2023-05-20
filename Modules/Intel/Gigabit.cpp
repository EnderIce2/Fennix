#include "gigabit.hpp"

#include <net/net.hpp>
#include <debug.h>
#include <pci.hpp>
#include <io.h>

#include "../../DAPI.hpp"
#include "../drv.hpp"

using namespace PCI;

namespace Gigabit
{
	KernelAPI KAPI;

	PCIDeviceHeader *PCIBaseAddress;
	uint32_t CurrentPacket;
	BARData BAR;
	bool EEPROMAvailable;

	uint16_t RXCurrent;
	uint16_t TXCurrent;
	RXDescriptor *RX[E1000_NUM_RX_DESC];
	TXDescriptor *TX[E1000_NUM_TX_DESC];

	MediaAccessControl MAC;
	InternetProtocol::Version4 IP;

	void WriteCMD(uint16_t Address, uint32_t Value)
	{
		if (BAR.Type == 0)
			mmioout32(BAR.MemoryBase + Address, Value);
		else
		{
			outportl(BAR.IOBase, Address);
			outportl(BAR.IOBase + 4, Value);
		}
	}

	uint32_t ReadCMD(uint16_t Address)
	{
		if (BAR.Type == 0)
			return mmioin32(BAR.MemoryBase + Address);
		else
		{
			outportl(BAR.IOBase, Address);
			return inportl(BAR.IOBase + 0x4);
		}
	}

	uint32_t ReadEEPROM(uint8_t Address)
	{
		uint16_t Data = 0;
		uint32_t temp = 0;
		if (EEPROMAvailable)
		{
			WriteCMD(REG::EEPROM, (1) | ((uint32_t)(Address) << 8));
			while (!((temp = ReadCMD(REG::EEPROM)) & (1 << 4)))
				;
		}
		else
		{
			WriteCMD(REG::EEPROM, (1) | ((uint32_t)(Address) << 2));
			while (!((temp = ReadCMD(REG::EEPROM)) & (1 << 1)))
				;
		}
		Data = (uint16_t)((temp >> 16) & 0xFFFF);
		return Data;
	}

	MediaAccessControl GetMAC()
	{
		MediaAccessControl mac;
		if (EEPROMAvailable)
		{
			uint32_t temp;
			temp = ReadEEPROM(0);
			mac.Address[0] = temp & 0xff;
			mac.Address[1] = (uint8_t)(temp >> 8);
			temp = ReadEEPROM(1);
			mac.Address[2] = temp & 0xff;
			mac.Address[3] = (uint8_t)(temp >> 8);
			temp = ReadEEPROM(2);
			mac.Address[4] = temp & 0xff;
			mac.Address[5] = (uint8_t)(temp >> 8);
		}
		else
		{
			uint8_t *BaseMac8 = (uint8_t *)(BAR.MemoryBase + 0x5400);
			uint32_t *BaseMac32 = (uint32_t *)(BAR.MemoryBase + 0x5400);
			if (BaseMac32[0] != 0)
				for (int i = 0; i < 6; i++)
					mac.Address[i] = BaseMac8[i];
			else
			{
				error("No MAC address found.");
				return MediaAccessControl();
			}
		}
		return mac;
	}

	void InitializeRX()
	{
		debug("Initializing RX...");
		uintptr_t Ptr = (uintptr_t)KAPI.Memory.RequestPage((((sizeof(RXDescriptor) * E1000_NUM_RX_DESC + 16)) / KAPI.Memory.PageSize) + 1);

		for (int i = 0; i < E1000_NUM_RX_DESC; i++)
		{
			RX[i] = (RXDescriptor *)(Ptr + i * 16);
			RX[i]->Address = (uint64_t)(uintptr_t)KAPI.Memory.RequestPage(((8192 + 16) / KAPI.Memory.PageSize) + 1);
			RX[i]->Status = 0;
		}

		WriteCMD(REG::TXDESCLO, (uint32_t)(Ptr >> 32));
		WriteCMD(REG::TXDESCHI, (uint32_t)(Ptr & 0xFFFFFFFF));

		WriteCMD(REG::RXDESCLO, (uint32_t)Ptr);
		WriteCMD(REG::RXDESCHI, 0);

		WriteCMD(REG::RXDESCLEN, E1000_NUM_RX_DESC * 16);

		WriteCMD(REG::RXDESCHEAD, 0);
		WriteCMD(REG::RXDESCTAIL, E1000_NUM_RX_DESC - 1);
		RXCurrent = 0;
		WriteCMD(REG::RCTRL, RCTL::EN | RCTL::SBP | RCTL::UPE | RCTL::MPE | RCTL::LBM_NONE | RTCL::RDMTS_HALF | RCTL::BAM | RCTL::SECRC | RCTL::BSIZE_8192);
	}

	void InitializeTX()
	{
		debug("Initializing TX...");
		uintptr_t Ptr = (uintptr_t)KAPI.Memory.RequestPage(((sizeof(TXDescriptor) * E1000_NUM_RX_DESC + 16) / KAPI.Memory.PageSize) + 1);

		for (short i = 0; i < E1000_NUM_TX_DESC; i++)
		{
			TX[i] = (TXDescriptor *)((uintptr_t)Ptr + i * 16);
			TX[i]->Address = 0;
			TX[i]->Command = 0;
			TX[i]->Status = TSTA::DD;
		}

		WriteCMD(REG::TXDESCHI, (uint32_t)((uint64_t)Ptr >> 32));
		WriteCMD(REG::TXDESCLO, (uint32_t)((uint64_t)Ptr & 0xFFFFFFFF));

		WriteCMD(REG::TXDESCLEN, E1000_NUM_TX_DESC * 16);

		WriteCMD(REG::TXDESCHEAD, 0);
		WriteCMD(REG::TXDESCTAIL, 0);
		TXCurrent = 0;
		WriteCMD(REG::TCTRL, TCTL::EN_ | TCTL::PSP | (15 << TCTL::CT_SHIFT) | (64 << TCTL::COLD_SHIFT) | TCTL::RTLC);

		WriteCMD(REG::TCTRL, 0b0110000000000111111000011111010);
		WriteCMD(REG::TIPG, 0x0060200A);
	}

	int DriverEntry(void *Data)
	{
		if (!Data)
			return INVALID_KERNEL_API;
		KAPI = *(KernelAPI *)Data;
		if (KAPI.Version.Major < 0 || KAPI.Version.Minor < 0 || KAPI.Version.Patch < 0)
			return KERNEL_API_VERSION_NOT_SUPPORTED;
		return OK;
	}

	int CallbackHandler(KernelCallback *Data)
	{
		switch (Data->Reason)
		{
		case AcknowledgeReason:
		{
			debug("Kernel acknowledged the driver.");
			break;
		}
		case ConfigurationReason:
		{
			debug("Driver received configuration data.");
			PCIBaseAddress = reinterpret_cast<PCIDeviceHeader *>(Data->RawPtr);
			switch (PCIBaseAddress->DeviceID)
			{
			case 0x100E:
			{
				trace("Found Intel 82540EM Gigabit Ethernet Controller.");

				PCIBaseAddress->Command |= PCI_COMMAND_MASTER | PCI_COMMAND_IO | PCI_COMMAND_MEMORY;
				uint32_t PCIBAR0 = ((PCIHeader0 *)PCIBaseAddress)->BAR0;
				uint32_t PCIBAR1 = ((PCIHeader0 *)PCIBaseAddress)->BAR1;

				BAR.Type = PCIBAR0 & 1;
				BAR.IOBase = (uint16_t)(PCIBAR1 & (~3));
				BAR.MemoryBase = PCIBAR0 & (~15);

				// Detect EEPROM
				WriteCMD(REG::EEPROM, 0x1);
				for (int i = 0; i < 1000 && !EEPROMAvailable; i++)
					if (ReadCMD(REG::EEPROM) & 0x10)
						EEPROMAvailable = true;
					else
						EEPROMAvailable = false;

				// Get MAC address
				if (!GetMAC().Valid())
					return NOT_AVAILABLE;
				else
					debug("MAC address found.");
				MAC = GetMAC();

				// Start link
				uint32_t cmdret = ReadCMD(REG::CTRL);
				WriteCMD(REG::CTRL, cmdret | ECTRL::SLU);

				for (int i = 0; i < 0x80; i++)
					WriteCMD((uint16_t)(0x5200 + i * 4), 0);

				WriteCMD(REG::IMASK, 0x1F6DC);
				WriteCMD(REG::IMASK, 0xFF & ~4);
				ReadCMD(0xC0);

				InitializeRX();
				InitializeTX();
				return OK;
			}
			case 0x100F:
			{
				trace("Found Intel 82545EM Gigabit Ethernet Controller.");
				PCIBaseAddress->Command |= PCI_COMMAND_MASTER | PCI_COMMAND_IO | PCI_COMMAND_MEMORY;
				uint32_t PCIBAR0 = ((PCIHeader0 *)PCIBaseAddress)->BAR0;
				uint32_t PCIBAR1 = ((PCIHeader0 *)PCIBaseAddress)->BAR1;

				BAR.Type = PCIBAR0 & 1;
				BAR.IOBase = (uint16_t)(PCIBAR1 & (~3));
				BAR.MemoryBase = PCIBAR0 & (~15);

				// Detect EEPROM
				WriteCMD(REG::EEPROM, 0x1);
				for (int i = 0; i < 1000 && !EEPROMAvailable; i++)
					if (ReadCMD(REG::EEPROM) & 0x10)
						EEPROMAvailable = true;
					else
						EEPROMAvailable = false;

				// Get MAC address
				if (!GetMAC().Valid())
					return NOT_AVAILABLE;
				else
					debug("MAC address found.");
				MAC = GetMAC();

				return NOT_IMPLEMENTED;
			}
			case 0x10D3:
			{
				trace("Found Intel 82574L Gigabit Ethernet Controller.");

				PCIBaseAddress->Command |= PCI_COMMAND_MASTER | PCI_COMMAND_IO | PCI_COMMAND_MEMORY;
				uint32_t PCIBAR0 = ((PCIHeader0 *)PCIBaseAddress)->BAR0;
				uint32_t PCIBAR1 = ((PCIHeader0 *)PCIBaseAddress)->BAR1;

				BAR.Type = PCIBAR0 & 1;
				BAR.IOBase = (uint16_t)(PCIBAR1 & (~3));
				BAR.MemoryBase = PCIBAR0 & (~15);

				// Detect EEPROM
				WriteCMD(REG::EEPROM, 0x1);
				for (int i = 0; i < 1000 && !EEPROMAvailable; i++)
					if (ReadCMD(REG::EEPROM) & 0x10)
						EEPROMAvailable = true;
					else
						EEPROMAvailable = false;

				// Get MAC address
				if (!GetMAC().Valid())
					return NOT_AVAILABLE;
				else
					debug("MAC address found.");
				MAC = GetMAC();

				return NOT_IMPLEMENTED;
			}
			case 0x10EA:
			{
				fixme("Found Intel I217-LM Gigabit Ethernet Controller.");
				return NOT_IMPLEMENTED;
			}
			case 0x153A:
			{
				fixme("Found Intel 82577LM Gigabit Ethernet Controller.");
				return NOT_IMPLEMENTED;
			}
			default:
			{
				error("Unsupported Intel Ethernet Controller.");
				return DEVICE_NOT_SUPPORTED;
			}
			}
			return ERROR;
		}
		case FetchReason:
		{
			memcpy(Data->NetworkCallback.Fetch.Name, (void *)"Intel Gigabit Ethernet Controller", 34);
			Data->NetworkCallback.Fetch.MAC = MAC.ToHex();
			break;
		}
		case SendReason:
		{
			TX[TXCurrent]->Address = (uint64_t)Data->NetworkCallback.Send.Data;
			TX[TXCurrent]->Length = (uint16_t)Data->NetworkCallback.Send.Length;
			TX[TXCurrent]->Command = CMD::EOP | CMD::IFCS | CMD::RS;
			TX[TXCurrent]->Status = 0;
			uint16_t OldTXCurrent = TXCurrent;
			TXCurrent = (uint16_t)((TXCurrent + 1) % E1000_NUM_TX_DESC);
			WriteCMD(REG::TXDESCTAIL, TXCurrent);
			while (!(TX[OldTXCurrent]->Status & 0xFF))
				;
			break;
		}
		case StopReason:
		{
			// Clearing Enable bit in Receive Control Register
			uint32_t cmdret = ReadCMD(REG::RCTRL);
			WriteCMD(REG::RCTRL, cmdret & ~RCTL::EN);

			// Masking Interrupt Mask, Interrupt Throttling Rate & Interrupt Auto-Mask
			WriteCMD(REG::IMASK, 0x00000000);
			WriteCMD(REG::ITR, 0x00000000);
			WriteCMD(REG::IAM, 0x00000000);

			// Clearing SLU bit in Device Control Register
			cmdret = ReadCMD(REG::CTRL);
			WriteCMD(REG::CTRL, cmdret & ~ECTRL::SLU);

			// Clear the Interrupt Cause Read register by reading it
			ReadCMD(REG::ICR);

			// Powering down the device (?)
			WriteCMD(REG::CTRL, PCTRL::POWER_DOWN);
			/* TODO: Stop link; further testing required */
			debug("Driver stopped.");
			break;
		}
		default:
		{
			warn("Unknown reason.");
			break;
		}
		}
		return OK;
	}

	int InterruptCallback(CPURegisters *)
	{
		WriteCMD(REG::IMASK, 0x1);
		uint32_t status = ReadCMD(0xC0);
		UNUSED(status);

		while ((RX[RXCurrent]->Status & 0x1))
		{
			uint8_t *Data = (uint8_t *)RX[RXCurrent]->Address;
			uint16_t DataLength = RX[RXCurrent]->Length;
			KAPI.Command.Network.ReceivePacket(KAPI.Info.DriverUID, Data, DataLength);
			RX[RXCurrent]->Status = 0;
			uint16_t OldRXCurrent = RXCurrent;
			RXCurrent = (uint16_t)((RXCurrent + 1) % E1000_NUM_RX_DESC);
			WriteCMD(REG::RXDESCTAIL, OldRXCurrent);
		}
		return OK;
	}
}
