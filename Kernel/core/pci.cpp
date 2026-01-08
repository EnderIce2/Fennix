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

#include <pci.hpp>

#include <power.hpp>
#include <acpi.hpp>

#include "../kernel.h"

namespace PCI
{
	namespace Descriptors
	{
		const char *u8ToHexString(uint8_t Value)
		{
			static char Buffer[3];
			memset(Buffer, 0, 3);
			for (size_t i = 0; i < 2; i++)
			{
				uint8_t Digit = (Value >> (4 - (i * 4))) & 0xF;
				if (Digit < 10)
					Buffer[i] = s_cst(char, '0' + Digit);
				else
					Buffer[i] = s_cst(char, 'A' + (Digit - 10));
			}
			return Buffer;
		}

		const char *u32ToHexString(uint32_t Value)
		{
			static char Buffer[9];
			memset(Buffer, 0, 9);
			for (size_t i = 0; i < 8; i++)
			{
				uint8_t Digit = (Value >> (28 - (i * 4))) & 0xF;
				if (Digit < 10)
					Buffer[i] = s_cst(char, '0' + Digit);
				else
					Buffer[i] = s_cst(char, 'A' + (Digit - 10));
			}
			return Buffer;
		}

		const char *MassStorageControllerSubclassName(uint8_t SubclassCode)
		{
			switch (SubclassCode)
			{
			case 0x00:
				return "SCSI Bus Controller";
			case 0x01:
				return "IDE Controller";
			case 0x02:
				return "Floppy Disk Controller";
			case 0x03:
				return "IPI Bus Controller";
			case 0x04:
				return "RAID Controller";
			case 0x05:
				return "ATA Controller";
			case 0x06:
				return "Serial ATA";
			case 0x07:
				return "Serial Attached SCSI Controller";
			case 0x08:
				return "Non-Volatile Memory Controller";
			case 0x80:
				return "Mass Storage Controller";
			default:
				break;
			}
			fixme("Unknown mass storage controller %02x", SubclassCode);
			return u8ToHexString(SubclassCode);
		}

		const char *NetworkControllerSubclassName(uint8_t SubclassCode)
		{
			switch (SubclassCode)
			{
			case 0x00:
				return "Ethernet Controller";
			case 0x01:
				return "Token Ring Controller";
			case 0x02:
				return "FDDI Controller";
			case 0x03:
				return "ATM Controller";
			case 0x04:
				return "ISDN Controller";
			case 0x05:
				return "WorldFip Controller";
			case 0x06:
				return "PICMG HyperCard Controller";
			case 0x07:
				return "Infiniband Controller";
			case 0x08:
				return "Fabric Controller";
			case 0x80:
				return "Network Controller";
			default:
				break;
			}
			fixme("Unknown network controller %02x", SubclassCode);
			return u8ToHexString(SubclassCode);
		}

		const char *DisplayControllerSubclassName(uint8_t SubclassCode)
		{
			switch (SubclassCode)
			{
			case 0x00:
				return "VGA Compatible Controller";
			case 0x01:
				return "XGA Controller";
			case 0x02:
				return "3D Controller";
			case 0x80:
				return "Display Controller";
			default:
				break;
			}
			fixme("Unknown display controller %02x", SubclassCode);
			return u8ToHexString(SubclassCode);
		}

		const char *CommunicationControllerSubclassName(uint8_t SubclassCode)
		{
			switch (SubclassCode)
			{
			case 0x00:
				return "Serial Controller";
			case 0x01:
				return "Parallel Controller";
			case 0x02:
				return "Multi-Serial Controller";
			case 0x03:
				return "IEEE-1284 Controller";
			case 0x04:
				return "ATM Controller";
			case 0x05:
				return "Object Storage Controller";
			case 0x80:
				return "Communication controller";
			default:
				break;
			}
			fixme("Unknown communication controller %02x", SubclassCode);
			return u8ToHexString(SubclassCode);
		}

		const char *BaseSystemPeripheralSubclassName(uint8_t SubclassCode)
		{
			// not sure if it's right
			switch (SubclassCode)
			{
			case 0x00:
				return "Unclassified";
			case 0x01:
				return "Keyboard";
			case 0x02:
				return "Pointing Device";
			case 0x03:
				return "Mouse";
			case 0x04:
				return "Scanner";
			case 0x05:
				return "Gameport";
			case 0x80:
				return "Unclassified";
			default:
				break;
			}
			fixme("Unknown base system peripheral %02x", SubclassCode);
			return u8ToHexString(SubclassCode);
		}

		const char *SerialBusControllerSubclassName(uint8_t SubclassCode)
		{
			switch (SubclassCode)
			{
			case 0x00:
				return "FireWire (IEEE 1394) Controller";
			case 0x01:
				return "ACCESS Bus Controller";
			case 0x02:
				return "SSA Controller";
			case 0x03:
				return "USB Controller";
			case 0x04:
				return "Fibre Channel Controller";
			case 0x05:
				return "SMBus Controller";
			case 0x06:
				return "Infiniband Controller";
			case 0x07:
				return "IPMI Interface Controller";
			case 0x08:
				return "SERCOS Interface (IEC 61491) Controller";
			case 0x09:
				return "CANbus Controller";
			case 0x80:
				return "Serial Bus Controller";
			default:
				break;
			}
			fixme("Unknown serial bus controller %02x", SubclassCode);
			return u8ToHexString(SubclassCode);
		}

		const char *BridgeDeviceSubclassName(uint8_t SubclassCode)
		{
			switch (SubclassCode)
			{
			case 0x00:
				return "Host Bridge";
			case 0x01:
				return "ISA Bridge";
			case 0x02:
				return "EISA Bridge";
			case 0x03:
				return "MCA Bridge";
			case 0x04:
				return "PCI-to-PCI Bridge";
			case 0x05:
				return "PCMCIA Bridge";
			case 0x06:
				return "NuBus Bridge";
			case 0x07:
				return "CardBus Bridge";
			case 0x08:
				return "RACEway Bridge";
			case 0x09:
				return "PCI-to-PCI Bridge";
			case 0x0A:
				return "InfiniBand-to-PCI Host Bridge";
			case 0x80:
				return "Bridge Device";
			default:
				break;
			}
			fixme("Unknown bridge device %02x", SubclassCode);
			return u8ToHexString(SubclassCode);
		}

		const char *WirelessControllerSubclassName(uint8_t SubclassCode)
		{
			switch (SubclassCode)
			{
			case 0x11:
				return "Bluetooth";
			case 0x20:
				return "802.1a controller";
			case 0x21:
				return "802.1b controller";
			case 0x80:
				return "Wireless controller";
			default:
				break;
			}
			fixme("Unknown wireless controller %02x", SubclassCode);
			return u8ToHexString(SubclassCode);
		}

		const char *GetVendorName(uint32_t VendorID)
		{
			switch (VendorID)
			{
			case 0x106B:
				return "Apple Inc.";
			case 0x104B:
				return "Bus Logic";
			case 0x1000:
				return "Symbios Logic";
			case 0x1B36:
			case 0x1AF4:
				return "Red Hat, Inc.";
			case 0x10EC:
				return "Realtek Semiconductor Co., Ltd.";
			case 0x80EE:
				return "VirtualBox";
			case 0x1274:
				return "Ensoniq";
			case 0x1234:
				return "QEMU";
			case 0x15AD:
				return "VMware";
			case 0x8086:
				return "Intel Corporation";
			case 0x1022:
				return "Advanced Micro Devices, Inc.";
			case 0x10DE:
				return "NVIDIA Corporation";
			case 0x1AE0:
				return "Google, Inc.";
			case 0x1a58:
				return "Razer USA Ltd.";
			case 0x1414:
				return "Microsoft Corporation";
			default:
				break;
			}
			fixme("Unknown vendor %04x", VendorID);
			return u32ToHexString(VendorID);
		}

		const char *GetDeviceName(uint32_t VendorID, uint32_t DeviceID)
		{
			switch (VendorID)
			{
			case Apple:
			{
				switch (DeviceID)
				{
				case 0x3f:
					return "KeyLargo/Intrepid USB";
				default:
					break;
				}
				break;
			}
			case BusLogic:
			{
				switch (DeviceID)
				{
				case 0x1040:
					return "BT-946C (BA80C30) [MultiMaster 10]";
				default:
					break;
				}
				break;
			}
			case SymbiosLogic:
			{
				switch (DeviceID)
				{
				case 0x30:
					return "53c1030 PCI-X Fusion-MPT Dual Ultra320 SCSI";
				case 0x54:
					return "SAS1068 PCI-X Fusion-MPT SAS";
				case 0x1000:
					return "63C815";
				default:
					break;
				}
				break;
			}
			case RedHat:
			{
				switch (DeviceID)
				{
				case 0x1000:
				case 0x1041:
					return "Virtio network device";
				case 0x1001:
				case 0x1042:
					return "Virtio block device";
				case 0x1002:
				case 0x1045:
					return "Virtio memory balloon";
				case 0x1003:
				case 0x1043:
					return "Virtio console";
				case 0x1004:
				case 0x1048:
					return "Virtio SCSI";
				case 0x1005:
				case 0x1044:
					return "Virtio RNG";
				case 0x1009:
				case 0x1049:
				case 0x105a:
					return "Virtio filesystem";
				case 0x1050:
					return "Virtio GPU";
				case 0x1052:
					return "Virtio input";
				case 0x1053:
					return "Virtio socket";
				case 1110:
					return "Inter-VM shared memory";
				case 0x1af41100:
					return "QEMU Virtual Machine";
				default:
					break;
				}
				break;
			}
			case REDHat2:
			{
				switch (DeviceID)
				{
				case 0x0001:
					return "QEMU PCI-PCI bridge";
				case 0x0002:
					return "QEMU PCI 16550A Adapter";
				case 0x0003:
					return "QEMU PCI Dual-port 16550A Adapter";
				case 0x0004:
					return "QEMU PCI Quad-port 16550A Adapter";
				case 0x0005:
					return "QEMU PCI Test Device";
				case 0x0006:
					return "PCI Rocker Ethernet switch device";
				case 0x0007:
					return "PCI SD Card Host Controller Interface";
				case 0x0008:
					return "QEMU PCIe Host bridge";
				case 0x0009:
					return "QEMU PCI Expander bridge";
				case 0x000A:
					return "PCI-PCI bridge (multiseat)";
				case 0x000B:
					return "QEMU PCIe Expander bridge";
				case 0x000C:
					return "QEMU PCIe Root Port";
				case 0x000D:
					return "QEMU XHCI Host Controller";
				case 0x0010:
					return "QEMU NVM Express Controller";
				case 0x0100:
					return "QXL Paravirtual Graphic Card";
				case 0x1AF41100:
					return "QEMU Virtual Machine";
				default:
					break;
				}
				break;
			}
			case Realtek:
			{
				switch (DeviceID)
				{
				case 0x8029:
					return "RTL-8029(AS)";
				case 0x8139:
					return "RTL-8139/8139C/8139C+ Ethernet Controller";
				case 0x8161:
					return "RTL8111/8168/8411 PCI Express Gigabit Ethernet Controller";
				case 0x8168:
					return "RTL8111/8168/8411 PCI Express Gigabit Ethernet Controller";
				case 0xC821:
					return "RTL8821CE 802.11ac PCIe Wireless Network Adapter";
				default:
					break;
				}
				break;
			}
			case VirtualBox:
			{
				switch (DeviceID)
				{
				case 0xCAFE:
					return "VirtualBox Guest Service";
				case 0xBEEF:
					return "VirtualBox Graphics Adapter";
				case 0x0021:
					return "USB Tablet";
				case 0x0022:
					return "Multitouch tablet";
				case 0x4E56:
					return "NVM Express";
				default:
					break;
				}
				break;
			}
			case Ensoniq:
			{
				switch (DeviceID)
				{
				case 0x1371:
					return "ES1371/ES1373 / Creative Labs CT2518";
				case 0x5000:
					return "ES1370 [AudioPCI]";
				default:
					break;
				}
				break;
			}
			case QEMU:
			{
				switch (DeviceID)
				{
				case 0x1111:
					return "QEMU Display";
				default:
					break;
				}
				break;
			}
			case VMware:
			{
				switch (DeviceID)
				{
				case 0x0740:
					return "Virtual Machine Communication Interface";
				case 0x0405:
					return "SVGA II Adapter";
				case 0x0790:
					return "PCI bridge";
				case 0x07A0:
					return "PCI Express Root Port";
				case 0x0774:
					return "USB1.1 UHCI Controller";
				case 0x0770:
					return "USB2 EHCI Controller";
				case 0x0779:
					return "USB3 xHCI 1.0 Controller";
				case 0x07E0:
					return "SATA AHCI controller";
				case 0x07F0:
					return "NVM Express";
				default:
					break;
				}
				break;
			}
			case IntelCorporation:
			{
				switch (DeviceID)
				{
				case 0x1229:
					return "82557/8/9/0/1 Ethernet Pro 100";
				case 0x1209:
					return "8255xER/82551IT Fast Ethernet Controller";
				case 0x1004:
					return "82543GC Gigabit Ethernet Controller (Copper)";
				case 0x100E:
					return "82540EM Gigabit Ethernet Controller";
				case 0x7190:
					return "440BX/ZX/DX - 82443BX/ZX/DX Host bridge";
				case 0x7191:
					return "440BX/ZX/DX - 82443BX/ZX/DX AGP bridge";
				case 0x7110:
					return "82371AB/EB/MB PIIX4 ISA";
				case 0x7111:
					return "82371AB/EB/MB PIIX4 IDE";
				case 0x7113:
					return "82371AB/EB/MB PIIX4 ACPI";
				case 0x1e31:
					return "7 Series/C210 Series Chipset Family USB xHCI Host Controller";
				case 0x100F:
					return "82545EM Gigabit Ethernet Controller (Copper)";
				case 0x1903:
					return "Xeon E3-1200 v5/E3-1500 v5/6th Gen Core Processor Thermal Subsystem";
				case 0x1911:
					return "Xeon E3-1200 v5/v6 / E3-1500 v5 / 6th/7th Gen Core Processor Gaussian Mixture Model";
				case 0x1371:
					return "ES1371/ES1373 / Creative Labs CT2518";
				case 0x27b9:
					return "82801GBM (ICH7-M) LPC Interface Bridge";
				case 0x07E0:
					return "SATA AHCI controller";
				case 0x293E:
					return "82801I (ICH9 Family) HD Audio Controller";
				case 0x2935:
					return "82801I (ICH9 Family) USB UHCI Controller #2";
				case 0x2936:
					return "82801I (ICH9 Family) USB UHCI Controller #3";
				case 0x293A:
					return "82801I (ICH9 Family) USB2 EHCI Controller #1";
				case 0x2934:
					return "82801I (ICH9 Family) USB UHCI Controller #1";
				case 0x2668:
					return "82801FB/FBM/FR/FW/FRW (ICH6 Family) High Definition Audio Controller";
				case 0x265C:
					return "82801FB/FBM/FR/FW/FRW (ICH6 Family) USB2 EHCI Controller";
				case 0x2415:
					return "82801AA AC'97 Audio Controller";
				case 0x10D3:
					return "82574L Gigabit Network Connection";
				case 0x29C0:
					return "82G33/G31/P35/P31 Express DRAM Controller";
				case 0x2918:
					return "82801IB (ICH9) LPC Interface Controller";
				case 0x2829:
					return "82801HM/HEM (ICH8M/ICH8M-E) SATA Controller [AHCI mode]";
				case 0x2922:
					return "82801IR/IO/IH (ICH9R/DO/DH) 6 port SATA Controller [AHCI mode]";
				case 0x2930:
					return "82801I (ICH9 Family) SMBus Controller";
				case 0x269E:
					return "631xESB/632xESB IDE Controller";
				case 0x282A:
					return "82801 Mobile SATA Controller [RAID mode]";
				case 0x5914:
					return "Xeon E3-1200 v6/7th Gen Core Processor Host Bridge/DRAM Registers";
				case 0x5917:
					return "UHD Graphics 620";
				case 0x9D10:
					return "Sunrise Point-LP PCI Express Root Port #1";
				case 0x9D11:
					return "Sunrise Point-LP PCI Express Root Port #2";
				case 0x9D12:
					return "Sunrise Point-LP PCI Express Root Port #1";
				case 0x9D13:
					return "Sunrise Point-LP PCI Express Root Port #1";
				case 0x9D14:
					return "Sunrise Point-LP PCI Express Root Port #5";
				case 0x9D15:
					return "Sunrise Point-LP PCI Express Root Port #6";
				case 0x9D21:
					return "Sunrise Point-LP PMC";
				case 0x9D23:
					return "Sunrise Point-LP SMBus";
				case 0x9D2F:
					return "Sunrise Point-LP USB 3.0 xHCI Controller";
				case 0x9D31:
					return "Sunrise Point-LP Thermal subsystem";
				case 0x9D3A:
					return "Sunrise Point-LP CSME HECI #1";
				case 0x9D4E:
					return "Intel(R) 100 Series Chipset Family LPC Controller/eSPI Controller - 9D4E";
				case 0x9D71:
					return "Sunrise Point-LP HD Audio";
				default:
					break;
				}
				break;
			}
			case AdvancedMicroDevices:
			{
				switch (DeviceID)
				{
				case 0x2000:
					return "79C970 [PCnet32 LANCE]";
				default:
					break;
				}
				break;
			}
			case NVIDIACorporation:
			{
				switch (DeviceID)
				{
				case 0x174D:
					return "GM108M [GeForce MX130]";
				default:
					break;
				}
				break;
			default:
				break;
			}
			}
			fixme("Unknown device %04x:%04x", VendorID, DeviceID);
			return u32ToHexString(DeviceID);
		}

		const char *GetSubclassName(uint8_t ClassCode, uint8_t SubclassCode)
		{
			switch (ClassCode)
			{
			case 0x00:
				return "Unclassified";
			case 0x01:
				return MassStorageControllerSubclassName(SubclassCode);
			case 0x02:
				return NetworkControllerSubclassName(SubclassCode);
			case 0x03:
				return DisplayControllerSubclassName(SubclassCode);
			case 0x04:
				return "Multimedia controller";
			case 0x05:
				return "Memory Controller";
			case 0x06:
				return BridgeDeviceSubclassName(SubclassCode);
			case 0x07:
				return CommunicationControllerSubclassName(SubclassCode);
			case 0x08:
				return BaseSystemPeripheralSubclassName(SubclassCode);
			case 0x09:
				return "Input device controller";
			case 0x0A:
				return "Docking station";
			case 0x0B:
				return "Processor";
			case 0x0C:
				return SerialBusControllerSubclassName(SubclassCode);
			case 0x0D:
				return WirelessControllerSubclassName(SubclassCode);
			case 0x0E:
				return "Intelligent controller";
			case 0x0F:
				return "Satellite communication controller";
			case 0x10:
				return "Encryption controller";
			case 0x11:
				return "Signal processing accelerators";
			case 0x12:
				return "Processing accelerators";
			case 0x13:
				return "Non-Essential Instrumentation";
			case 0x40:
				return "Coprocessor";
			default:
				break;
			}
			fixme("Unknown subclass name %02x:%02x", ClassCode, SubclassCode);
			return u8ToHexString(SubclassCode);
		}

		const char *GetProgIFName(uint8_t ClassCode, uint8_t SubclassCode, uint8_t ProgIF)
		{
			switch (ClassCode)
			{
			case 0x01:
			{
				switch (SubclassCode)
				{
				case 0x06:
				{
					switch (ProgIF)
					{
					case 0:
						return "Vendor Specific SATA Controller";
					case 1:
						return "AHCI SATA Controller";
					case 2:
						return "Serial Storage Bus SATA Controller";
					default:
						return "SATA controller";
					}
					break;
				}
				case 0x08:
				{
					switch (ProgIF)
					{
					case 0x01:
						return "NVMHCI Controller";
					case 0x02:
						return "NVM Express Controller";
					default:
						return "Non-Volatile Memory Controller";
					}
					break;
				}
				default:
					break;
				}
			default:
				break;
			}
			case 0x03:
			{
				switch (SubclassCode)
				{
				case 0x00:
					switch (ProgIF)
					{
					case 0x00:
						return "VGA Controller";
					case 0x01:
						return "8514-Compatible Controller";
					default:
						return "VGA Compatible Controller";
					}
					break;
				default:
					break;
				}
				break;
			}
			case 0x07:
			{
				switch (SubclassCode)
				{
				case 0x00:
				{
					switch (ProgIF)
					{
					case 0x00:
						return "Serial controller <8250>";
					case 0x01:
						return "Serial controller <16450>";
					case 0x02:
						return "Serial controller <16550>";
					case 0x03:
						return "Serial controller <16650>";
					case 0x04:
						return "Serial controller <16750>";
					case 0x05:
						return "Serial controller <16850>";
					case 0x06:
						return "Serial controller <16950";
					default:
						return "Serial controller";
					}
					break;
				}
				default:
					break;
				}
				break;
			}
			case 0x0C:
			{
				switch (SubclassCode)
				{
				case 0x00:
				{
					switch (ProgIF)
					{
					case 0x00:
						return "Generic FireWire (IEEE 1394) Controller";
					case 0x10:
						return "OHCI FireWire (IEEE 1394) Controller";
					default:
						break;
					}
					break;
				}
				case 0x03:
				{
					switch (ProgIF)
					{
					case 0x00:
						return "UHCI (USB1) Controller";
					case 0x10:
						return "OHCI (USB1) Controller";
					case 0x20:
						return "EHCI (USB2) Controller";
					case 0x30:
						return "XHCI (USB3) Controller";
					case 0x80:
						return "Unspecified";
					case 0xFE:
						return "USB Device";
					default:
						break;
					}
					break;
				}
				default:
					break;
				}
				break;
			}
			}
			// not really a fixme
			// fixme("Unknown prog IF name %02x:%02x:%02x", ClassCode, SubclassCode, ProgIF);
			return u8ToHexString(ProgIF);
		}
	}

	uintptr_t PCIDevice::GetBAR(int8_t Index)
	{
		assert(Index >= 0 && Index < 6);

		switch (GetHeaderType())
		{
		case 128:
			warn("Unknown header type %d! Guessing PCI Header 0", GetHeaderType());
			[[fallthrough]];
		case 0: /* PCI Header 0 */
		{
			PCIHeader0 *hdr = (PCIHeader0 *)Header;
			uint32_t bar = hdr->BAR[Index];
			if (bar & 0x1)
				return bar & 0xFFFFFFFFFFFFFFFC; /* I/O Space */

			if (bar & 0x4) /* 64-bit Memory Space */
			{
				uint64_t bar64 = bar;
				if (Index >= 5)
					warn("BAR%d is part of a 64-bit address, but it's the last BAR!", Index);
				else
					bar64 |= ((uint64_t)hdr->BAR[Index + 1] << 32);
				return bar64 & 0xFFFFFFFFFFFFFFF0;
			}
			else /* 32-bit Memory Space */
				return bar & 0xFFFFFFFFFFFFFFF0;
		}
		case 1: /* PCI Header 1 (PCI-to-PCI Bridge) */
		{
			PCIHeader1 *hdr = (PCIHeader1 *)Header;
			if (Index >= 2)
			{
				error("PCI-to-PCI Bridge has only 2 BARs!");
				return 0;
			}

			uint32_t bar = hdr->BAR[Index];
			if (bar & 0x1) /* I/O Space */
				return bar & 0xFFFFFFFFFFFFFFFC;

			if (bar & 0x4) /* 64-bit Memory Space */
			{
				uint64_t bar64 = bar;
				if (Index >= 1)
					warn("BAR%d is part of a 64-bit address, but it's the last BAR!", Index);
				else
					bar64 |= ((uint64_t)hdr->BAR[Index + 1] << 32);
				return bar64 & 0xFFFFFFFFFFFFFFF0;
			}
			else /* 32-bit Memory Space */
				return bar & 0xFFFFFFFFFFFFFFF0;
		}
		case 2: /* PCI Header 2 (PCI-to-CardBus Bridge) */
		{
			PCIHeader2 *hdr = (PCIHeader2 *)Header;
			/* CardBus bridges have specific memory/IO windows, not standard BARs */
			if (Index == 0)
			{
				uint32_t bar = hdr->CardbusSocketRegistersBaseAddress;
				if (bar & 0x1) /* I/O Space */
					return bar & 0xFFFFFFFFFFFFFFFC;

				if (bar & 0x4) /* 64-bit Memory Space */
				{
					uint64_t bar64 = bar;
					warn("CardbusSocketRegistersBaseAddress is 64-bit");
					return bar64 & 0xFFFFFFFFFFFFFFF0;
				}
				else /* 32-bit Memory Space */
					return bar & 0xFFFFFFFFFFFFFFF0;
			}
			else if (Index == 1)
			{
				/* LegacyBaseAddress */
				uint32_t bar = hdr->LegacyBaseAddress;
				if (bar & 0x1) /* I/O Space */
					return bar & 0xFFFFFFFFFFFFFFFC;

				if (bar & 0x4) /* 64-bit Memory Space */
				{
					uint64_t bar64 = bar;
					warn("LegacyBaseAddress is 64-bit");
					return bar64 & 0xFFFFFFFFFFFFFFF0;
				}
				else /* 32-bit Memory Space */
					return bar & 0xFFFFFFFFFFFFFFF0;
			}
			else
			{
				error("PCI-to-CardBus Bridge has only 2 logical address registers!");
				return 0;
			}
		}
		default:
		{
			error("Unknown header type %d", GetHeaderType());
			return 0;
		}
		}
	}

#ifdef DEBUG
	void e(PCIDevice dev)
	{
		PCIDeviceHeader *hdr = dev.Header;

		debug("%02x.%02x.%02x - %#x:%#x\t\t%s / %s / %s / %s / %s",
			  dev.Bus, dev.Device, dev.Function,
			  hdr->VendorID, hdr->DeviceID,
			  Descriptors::GetVendorName(hdr->VendorID),
			  Descriptors::GetDeviceName(hdr->VendorID, hdr->DeviceID),
			  Descriptors::DeviceClasses[hdr->Class],
			  Descriptors::GetSubclassName(hdr->Class, hdr->Subclass),
			  Descriptors::GetProgIFName(hdr->Class, hdr->Subclass, hdr->ProgIF));
	}
#endif

	void Manager::MapPCIAddresses(PCIDevice Device, Memory::PageTable *Table)
	{
		switch (Device.GetHeaderType())
		{
		case 128:
			warn("Unknown header type %d! Guessing PCI Header 0", Device.GetHeaderType());
			[[fallthrough]];
		case 0: /* PCI Header 0 */
		{
			PCIHeader0 *hdr = (PCIHeader0 *)Device.Header;
			for (char i = 0; i < 6; i++)
			{
				if (hdr->BAR[i] == 0)
					continue;

				uintptr_t base = Device.GetBAR(i);
				size_t size = hdr->BAR[i].GetSize();

				if ((hdr->BAR[i] & 1) == 1) /* I/O Base */
				{
					debug("no need to map BAR%d %#x-%#x as it's an I/O space", i, base, base + size);
					break;
				}

				uint32_t flags = Memory::RW;
				if (hdr->BAR[i].Memory.Prefetchable)
					flags |= Memory::PWT; /* Use write-through for prefetchable regions */
				else
					flags |= Memory::PWT | Memory::PCD; /* Uncacheable for non-prefetchable */

				debug("mapping %s region %#lx-%#lx", hdr->BAR[i].Memory.Prefetchable ? "prefetchable" : "non-prefetchable", base, base + size);
				Memory::Virtual(Table).Map((void *)base, (void *)base, size, flags);

				if ((hdr->BAR[i] & 0x4) == 0x4) /* 64-bit Memory Space */
					i++;						/* Skip the next BAR as it's part of this 64-bit BAR */
			}
			break;
		}
		case 1: /* PCI Header 1 (PCI-to-PCI Bridge) */
		{
			PCIHeader1 *hdr = (PCIHeader1 *)Device.Header;
			for (char i = 0; i < 2; i++)
			{
				if (hdr->BAR[i] == 0)
					continue;

				uintptr_t base = Device.GetBAR(i);
				size_t size = hdr->BAR[i].GetSize();

				if ((hdr->BAR[i] & 1) == 1) /* I/O Base */
				{
					debug("no need to map BAR%d %#x-%#x as it's an I/O space", i, base, base + size);
					break;
				}

				uint32_t flags = Memory::RW;
				if (hdr->BAR[i].Memory.Prefetchable)
					flags |= Memory::PWT; /* Use write-through for prefetchable regions */
				else
					flags |= Memory::PWT | Memory::PCD; /* Uncacheable for non-prefetchable */

				debug("mapping %s region %#lx-%#lx", hdr->BAR[i].Memory.Prefetchable ? "prefetchable" : "non-prefetchable", base, base + size);
				Memory::Virtual(Table).Map((void *)base, (void *)base, size, flags);

				if ((hdr->BAR[i] & 0x4) == 0x4) /* 64-bit Memory Space */
					i++;						/* Skip the next BAR as it's part of this 64-bit BAR */
			}
			break;
		}
		case 2: /* PCI Header 2 (PCI-to-CardBus Bridge) */
		{
			fixme("Mapping PCI-to-CardBus Bridge BARs is not implemented yet");
			break;
		}
		default:
		{
			error("Unknown header type %d", Device.GetHeaderType());
			break;
		}
		}
	}

	void Manager::InitializeDevice(PCIDevice Device, Memory::PageTable *Table)
	{
		this->MapPCIAddresses(Device, Table);

		PCI::PCIDeviceHeader *Header = Device.Header;

		Header->Command |= PCI_COMMAND_MASTER |
						   PCI_COMMAND_IO |
						   PCI_COMMAND_MEMORY;
		Header->Command &= ~PCI_COMMAND_INTX_DISABLE;
	}

	void Manager::EnumerateFunction(uint64_t DeviceAddress, uint32_t Function, PCIDevice dev)
	{
		dev.Function = Function;

		uintptr_t Offset = Function << 12;
		uint64_t FunctionAddress = DeviceAddress + Offset;
		Memory::Virtual(KernelPageTable).Map((void *)FunctionAddress, (void *)FunctionAddress, Memory::PTFlag::RW);
		PCIDeviceHeader *PCIDeviceHdr = (PCIDeviceHeader *)FunctionAddress;
		dev.Header = PCIDeviceHdr;

		if (PCIDeviceHdr->DeviceID == 0)
			return;
		if (PCIDeviceHdr->DeviceID == 0xFFFF)
			return;

		Devices.push_back(dev);
#ifdef DEBUG
		e(dev);
#endif
	}

	void Manager::EnumerateDevice(uint64_t BusAddress, uint32_t Device, PCIDevice dev)
	{
		dev.Device = Device;

		uintptr_t Offset = Device << 15;
		uint64_t DeviceAddress = BusAddress + Offset;
		Memory::Virtual(KernelPageTable).Map((void *)DeviceAddress, (void *)DeviceAddress, Memory::PTFlag::RW);
		PCIDeviceHeader *PCIDeviceHdr = (PCIDeviceHeader *)DeviceAddress;

		if (PCIDeviceHdr->DeviceID == 0)
			return;
		if (PCIDeviceHdr->DeviceID == 0xFFFF)
			return;

		for (uint32_t Function = 0; Function < 8; Function++)
			EnumerateFunction(DeviceAddress, Function, dev);
	}

	void Manager::EnumerateBus(uint64_t BaseAddress, uint32_t Bus, PCIDevice dev)
	{
		dev.Bus = Bus;

		uintptr_t Offset = Bus << 20;
		uint64_t BusAddress = BaseAddress + Offset;
		Memory::Virtual(KernelPageTable).Map((void *)BusAddress, (void *)BusAddress, Memory::PTFlag::RW);
		PCIDeviceHeader *PCIDeviceHdr = (PCIDeviceHeader *)BusAddress;

		if (Bus != 0) // TODO: VirtualBox workaround (UNTESTED ON REAL HARDWARE!)
		{
			if (PCIDeviceHdr->DeviceID == 0)
				return;
			if (PCIDeviceHdr->DeviceID == 0xFFFF)
				return;
		}

		debug("PCI Bus DeviceID:%#x VendorID:%#x BIST:%#x Cache:%#x Class:%#x Cmd:%#x HdrType:%#x LatencyTimer:%#x ProgIF:%#x RevID:%#x Status:%#x SubClass:%#x ",
			  PCIDeviceHdr->DeviceID, PCIDeviceHdr->VendorID, PCIDeviceHdr->BIST,
			  PCIDeviceHdr->CacheLineSize, PCIDeviceHdr->Class, PCIDeviceHdr->Command,
			  PCIDeviceHdr->HeaderType, PCIDeviceHdr->LatencyTimer, PCIDeviceHdr->ProgIF,
			  PCIDeviceHdr->RevisionID, PCIDeviceHdr->Status, PCIDeviceHdr->Subclass);

		for (uint32_t Device = 0; Device < 32; Device++)
			EnumerateDevice(BusAddress, Device, dev);
	}

	std::list<PCIDevice> Manager::FindPCIDevice(uint8_t Class, uint8_t Subclass, uint8_t ProgIF)
	{
		std::list<PCIDevice> DeviceFound;
		for (auto dev : Devices)
		{
			if (dev.Header->Class == Class &&
				dev.Header->Subclass == Subclass &&
				dev.Header->ProgIF == ProgIF)
			{
				DeviceFound.push_back(dev);
			}
		}
		return DeviceFound;
	}

	std::list<PCIDevice> Manager::FindPCIDevice(uint16_t VendorID, uint16_t DeviceID)
	{
		std::list<PCIDevice> DeviceFound;
		for (auto dev : Devices)
		{
			if (dev.Header->VendorID == VendorID &&
				dev.Header->DeviceID == DeviceID)
			{
				DeviceFound.push_back(dev);
			}
		}
		return DeviceFound;
	}

	std::list<PCIDevice> Manager::FindPCIDevice(std::list<uint16_t> VendorIDs, std::list<uint16_t> DeviceIDs)
	{
		std::list<PCIDevice> DeviceFound;
		for (auto dev : Devices)
		{
			for (auto VendorID : VendorIDs)
			{
				for (auto DeviceID : DeviceIDs)
				{
					if (dev.Header->VendorID == VendorID &&
						dev.Header->DeviceID == DeviceID)
					{
						DeviceFound.push_back(dev);
					}
				}
			}
		}
		return DeviceFound;
	}

	Manager::Manager()
	{
#if defined(__amd64__) || defined(__i386__)
		if (!PowerManager->GetACPI())
		{
			error("ACPI not found");
			return;
		}

		if (!((ACPI::ACPI *)PowerManager->GetACPI())->MCFG)
		{
			error("MCFG not found");
			return;
		}

		int Entries = s_cst(int, ((((ACPI::ACPI *)PowerManager->GetACPI())->MCFG->Header.Length) - sizeof(ACPI::ACPI::MCFGHeader)) / sizeof(DeviceConfig));
		Memory::Virtual vmm(KernelPageTable);
		for (int t = 0; t < Entries; t++)
		{
			DeviceConfig *NewDeviceConfig = (DeviceConfig *)((uintptr_t)((ACPI::ACPI *)PowerManager->GetACPI())->MCFG + sizeof(ACPI::ACPI::MCFGHeader) + (sizeof(DeviceConfig) * t));
			vmm.Map((void *)NewDeviceConfig->BaseAddress, (void *)NewDeviceConfig->BaseAddress, Memory::PTFlag::RW);
			debug("PCI Entry %d Address:%p BUS:%#x-%#x", t, NewDeviceConfig->BaseAddress,
				  NewDeviceConfig->StartBus, NewDeviceConfig->EndBus);

			PCIDevice dev{};
			dev.Config = NewDeviceConfig;
			for (uint32_t Bus = NewDeviceConfig->StartBus; Bus < NewDeviceConfig->EndBus; Bus++)
				EnumerateBus(NewDeviceConfig->BaseAddress, Bus, dev);
		}
#elif defined(__aarch64__)
		error("PCI not implemented on aarch64");
#endif
	}

	Manager::~Manager() {}
}
