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

#include <memory.hpp>
#include <power.hpp>
#if defined(a64)
#include "../Architecture/amd64/acpi.hpp"
#elif defined(a32)
#elif defined(aa64)
#endif

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
            case SymbiosLogic:
            {
                switch (DeviceID)
                {
                case 0x30:
                    return "53c1030 PCI-X Fusion-MPT Dual Ultra320 SCSI";
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
            default:
                break;
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

#ifdef DEBUG
    void e(PCIDeviceHeader *hdr)
    {
        debug("%#x:%#x\t\t%s / %s / %s / %s / %s",
              hdr->VendorID, hdr->DeviceID,
              Descriptors::GetVendorName(hdr->VendorID),
              Descriptors::GetDeviceName(hdr->VendorID, hdr->DeviceID),
              Descriptors::DeviceClasses[hdr->Class],
              Descriptors::GetSubclassName(hdr->Class, hdr->Subclass),
              Descriptors::GetProgIFName(hdr->Class, hdr->Subclass, hdr->ProgIF));
    }
#endif

    void PCI::EnumerateFunction(uintptr_t DeviceAddress, uintptr_t Function)
    {
        uintptr_t Offset = Function << 12;
        uintptr_t FunctionAddress = DeviceAddress + Offset;
        Memory::Virtual(KernelPageTable).Map((void *)FunctionAddress, (void *)FunctionAddress, Memory::PTFlag::RW);
        PCIDeviceHeader *PCIDeviceHdr = (PCIDeviceHeader *)FunctionAddress;
        if (PCIDeviceHdr->DeviceID == 0)
            return;
        if (PCIDeviceHdr->DeviceID == 0xFFFF)
            return;
        Devices.push_back(PCIDeviceHdr);
#ifdef DEBUG
        e(PCIDeviceHdr);
#endif
    }

    void PCI::EnumerateDevice(uintptr_t BusAddress, uintptr_t Device)
    {
        uintptr_t Offset = Device << 15;
        uintptr_t DeviceAddress = BusAddress + Offset;
        Memory::Virtual(KernelPageTable).Map((void *)DeviceAddress, (void *)DeviceAddress, Memory::PTFlag::RW);
        PCIDeviceHeader *PCIDeviceHdr = (PCIDeviceHeader *)DeviceAddress;
        if (PCIDeviceHdr->DeviceID == 0)
            return;
        if (PCIDeviceHdr->DeviceID == 0xFFFF)
            return;
        for (uintptr_t Function = 0; Function < 8; Function++)
            EnumerateFunction(DeviceAddress, Function);
    }

    void PCI::EnumerateBus(uintptr_t BaseAddress, uintptr_t Bus)
    {
        uintptr_t Offset = Bus << 20;
        uintptr_t BusAddress = BaseAddress + Offset;
        Memory::Virtual(KernelPageTable).Map((void *)BusAddress, (void *)BusAddress, Memory::PTFlag::RW);
        PCIDeviceHeader *PCIDeviceHdr = (PCIDeviceHeader *)BusAddress;
        if (Bus != 0) // TODO: VirtualBox workaround (UNTESTED ON REAL HARDWARE!)
        {
            if (PCIDeviceHdr->DeviceID == 0)
                return;
            if (PCIDeviceHdr->DeviceID == 0xFFFF)
                return;
        }
        debug("PCI Bus DeviceID:%#llx VendorID:%#llx BIST:%#llx Cache:%#llx Class:%#llx Cmd:%#llx HdrType:%#llx LatencyTimer:%#llx ProgIF:%#llx RevID:%#llx Status:%#llx SubClass:%#llx ",
              PCIDeviceHdr->DeviceID, PCIDeviceHdr->VendorID, PCIDeviceHdr->BIST,
              PCIDeviceHdr->CacheLineSize, PCIDeviceHdr->Class, PCIDeviceHdr->Command,
              PCIDeviceHdr->HeaderType, PCIDeviceHdr->LatencyTimer, PCIDeviceHdr->ProgIF,
              PCIDeviceHdr->RevisionID, PCIDeviceHdr->Status, PCIDeviceHdr->Subclass);
        for (uintptr_t Device = 0; Device < 32; Device++)
            EnumerateDevice(BusAddress, Device);
    }

    std::vector<PCIDeviceHeader *> PCI::FindPCIDevice(uint8_t Class, uint8_t Subclass, uint8_t ProgIF)
    {
        std::vector<PCIDeviceHeader *> DeviceFound;
        for (auto var : Devices)
            if (var->Class == Class && var->Subclass == Subclass && var->ProgIF == ProgIF)
                DeviceFound.push_back(var);
        return DeviceFound;
    }

    std::vector<PCIDeviceHeader *> PCI::FindPCIDevice(int VendorID, int DeviceID)
    {
        std::vector<PCIDeviceHeader *> DeviceFound;
        for (auto var : Devices)
            if (var->VendorID == VendorID && var->DeviceID == DeviceID)
                DeviceFound.push_back(var);
        return DeviceFound;
    }

    PCI::PCI()
    {
#if defined(a64)
        int Entries = s_cst(int, ((((ACPI::ACPI *)PowerManager->GetACPI())->MCFG->Header.Length) - sizeof(ACPI::ACPI::MCFGHeader)) / sizeof(DeviceConfig));
        Memory::Virtual vma = Memory::Virtual(KernelPageTable);
        for (int t = 0; t < Entries; t++)
        {
            DeviceConfig *NewDeviceConfig = (DeviceConfig *)((uintptr_t)((ACPI::ACPI *)PowerManager->GetACPI())->MCFG + sizeof(ACPI::ACPI::MCFGHeader) + (sizeof(DeviceConfig) * t));
            vma.Map((void *)NewDeviceConfig->BaseAddress, (void *)NewDeviceConfig->BaseAddress, Memory::PTFlag::RW);
            debug("PCI Entry %d Address:%#llx BUS:%#llx-%#llx", t, NewDeviceConfig->BaseAddress,
                  NewDeviceConfig->StartBus, NewDeviceConfig->EndBus);
            for (uintptr_t Bus = NewDeviceConfig->StartBus; Bus < NewDeviceConfig->EndBus; Bus++)
                EnumerateBus(NewDeviceConfig->BaseAddress, Bus);
        }
#elif defined(a32)
        error("PCI not implemented on i386");
#elif defined(aa64)
        error("PCI not implemented on aarch64");
#endif
    }

    PCI::~PCI()
    {
    }
}
