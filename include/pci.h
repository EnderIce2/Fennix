#ifndef __FENNIX_API_PCI_H__
#define __FENNIX_API_PCI_H__

#include <types.h>

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

struct PCIHeader0
{
    PCIDeviceHeader Header;
    uint32_t BAR0;
    uint32_t BAR1;
    uint32_t BAR2;
    uint32_t BAR3;
    uint32_t BAR4;
    uint32_t BAR5;
    uint32_t CardbusCISPtr;
    uint16_t SubsystemVendorID;
    uint16_t SubsystemID;
    uint32_t ExpansionROMBaseAddr;
    uint8_t CapabilitiesPtr;
    uint8_t Rsv0;
    uint16_t Rsv1;
    uint32_t Rsv2;
    uint8_t InterruptLine;
    uint8_t InterruptPin;
    uint8_t MinGrant;
    uint8_t MaxLatency;
};

#endif // !__FENNIX_API_PCI_H__
