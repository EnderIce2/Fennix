#ifndef __FENNIX_KERNEL_ACPI_H__
#define __FENNIX_KERNEL_ACPI_H__

#include <types.h>

#include <boot/binfo.h>
#include <ints.hpp>
#include <vector.hpp>
#include <cpu.hpp>

namespace ACPI
{
    class ACPI
    {
    public:
        struct ACPIHeader
        {
            unsigned char Signature[4];
            uint32_t Length;
            uint8_t Revision;
            uint8_t Checksum;
            uint8_t OEMID[6];
            uint8_t OEMTableID[8];
            uint32_t OEMRevision;
            uint32_t CreatorID;
            uint32_t CreatorRevision;
        } __attribute__((packed));

        struct GenericAddressStructure
        {
            uint8_t AddressSpace;
            uint8_t BitWidth;
            uint8_t BitOffset;
            uint8_t AccessSize;
            uint64_t Address;
        } __attribute__((packed));

        struct MCFGHeader
        {
            struct ACPIHeader Header;
            uint64_t Reserved;
        } __attribute__((packed));

        struct HPETHeader
        {
            ACPIHeader Header;
            uint8_t HardwareRevID;
            uint8_t ComparatorCount : 5;
            uint8_t CounterSize : 1;
            uint8_t Reserved : 1;
            uint8_t LegacyReplacement : 1;
            uint16_t PCIVendorID;
            struct GenericAddressStructure Address;
            uint8_t HPETNumber;
            uint16_t MinimumTick;
            uint8_t PageProtection;
        } __attribute__((packed));

        struct FADTHeader
        {
            ACPIHeader Header;
            uint32_t FirmwareCtrl;
            uint32_t Dsdt;
            uint8_t Reserved;
            uint8_t PreferredPowerManagementProfile;
            uint16_t SCI_Interrupt;
            uint32_t SMI_CommandPort;
            uint8_t AcpiEnable;
            uint8_t AcpiDisable;
            uint8_t S4BIOS_REQ;
            uint8_t PSTATE_Control;
            uint32_t PM1aEventBlock;
            uint32_t PM1bEventBlock;
            uint32_t PM1aControlBlock;
            uint32_t PM1bControlBlock;
            uint32_t PM2ControlBlock;
            uint32_t PMTimerBlock;
            uint32_t GPE0Block;
            uint32_t GPE1Block;
            uint8_t PM1EventLength;
            uint8_t PM1ControlLength;
            uint8_t PM2ControlLength;
            uint8_t PMTimerLength;
            uint8_t GPE0Length;
            uint8_t GPE1Length;
            uint8_t GPE1Base;
            uint8_t CStateControl;
            uint16_t WorstC2Latency;
            uint16_t WorstC3Latency;
            uint16_t FlushSize;
            uint16_t FlushStride;
            uint8_t DutyOffset;
            uint8_t DutyWidth;
            uint8_t DayAlarm;
            uint8_t MonthAlarm;
            uint8_t Century;
            uint16_t BootArchitectureFlags;
            uint8_t Reserved2;
            uint32_t Flags;
            struct GenericAddressStructure ResetReg;
            uint8_t ResetValue;
            uint8_t Reserved3[3];
            uint64_t X_FirmwareControl;
            uint64_t X_Dsdt;
            struct GenericAddressStructure X_PM1aEventBlock;
            struct GenericAddressStructure X_PM1bEventBlock;
            struct GenericAddressStructure X_PM1aControlBlock;
            struct GenericAddressStructure X_PM1bControlBlock;
            struct GenericAddressStructure X_PM2ControlBlock;
            struct GenericAddressStructure X_PMTimerBlock;
            struct GenericAddressStructure X_GPE0Block;
            struct GenericAddressStructure X_GPE1Block;
        } __attribute__((packed));

        struct BGRTHeader
        {
            ACPIHeader Header;
            uint16_t Version;
            uint8_t Status;
            uint8_t ImageType;
            uint64_t ImageAddress;
            uint32_t ImageOffsetX;
            uint32_t ImageOffsetY;
        };

        struct SRATHeader
        {
            ACPIHeader Header;
            uint32_t TableRevision; // Must be value 1
            uint64_t Reserved;      // Reserved, must be zero
        };

        struct TPM2Header
        {
            ACPIHeader Header;
            uint32_t Flags;
            uint64_t ControlAddress;
            uint32_t StartMethod;
        };

        struct TCPAHeader
        {
            ACPIHeader Header;
            uint16_t Reserved;
            uint32_t MaxLogLength;
            uint64_t LogAddress;
        };

        struct WAETHeader
        {
            ACPIHeader Header;
            uint32_t Flags;
        };

        struct HESTHeader
        {
            ACPIHeader Header;
            uint32_t ErrorSourceCount;
        };

        struct MADTHeader
        {
            ACPIHeader Header;
            uint32_t LocalControllerAddress;
            uint32_t Flags;
            char Entries[];
        } __attribute__((packed));

        ACPIHeader *XSDT = nullptr;
        MCFGHeader *MCFG = nullptr;
        HPETHeader *HPET = nullptr;
        FADTHeader *FADT = nullptr;
        BGRTHeader *BGRT = nullptr;
        SRATHeader *SRAT = nullptr;
        TPM2Header *TPM2 = nullptr;
        TCPAHeader *TCPA = nullptr;
        WAETHeader *WAET = nullptr;
        MADTHeader *MADT = nullptr;
        HESTHeader *HEST = nullptr;
        bool XSDTSupported = false;

        void *FindTable(ACPIHeader *ACPIHeader, char *Signature);
        void SearchTables(ACPIHeader *Header);
        ACPI(BootInfo *Info);
        ~ACPI();
    };

    class MADT
    {
    public:
        struct APICHeader
        {
            uint8_t Type;
            uint8_t Length;
        } __attribute__((packed));

        struct MADTIOApic
        {
            struct APICHeader Header;
            uint8_t APICID;
            uint8_t reserved;
            uint32_t Address;
            uint32_t GSIBase;
        } __attribute__((packed));

        struct MADTIso
        {
            struct APICHeader Header;
            uint8_t BuSSource;
            uint8_t IRQSource;
            uint32_t GSI;
            uint16_t Flags;
        } __attribute__((packed));

        struct MADTNmi
        {
            struct APICHeader Header;
            uint8_t processor;
            uint16_t flags;
            uint8_t lint;
        } __attribute__((packed));

        struct LocalAPIC
        {
            struct APICHeader Header;
            uint8_t ACPIProcessorId;
            uint8_t APICId;
            uint32_t Flags;
        } __attribute__((packed));

        struct LAPIC
        {
            uint8_t id;
            uintptr_t PhysicalAddress;
            void *VirtualAddress;
        };

        std::vector<MADTIOApic *> ioapic;
        std::vector<MADTIso *> iso;
        std::vector<MADTNmi *> nmi;
        std::vector<LocalAPIC *> lapic;
        struct LAPIC *LAPICAddress;
        uint16_t CPUCores;

        MADT(ACPI::MADTHeader *madt);
        ~MADT();
    };

    class DSDT : public Interrupts::Handler
    {
    private:
        uint32_t SMI_CMD = 0;
        uint8_t ACPI_ENABLE = 0;
        uint8_t ACPI_DISABLE = 0;
        uint32_t PM1a_CNT = 0;
        uint32_t PM1b_CNT = 0;
        uint16_t SLP_TYPa = 0;
        uint16_t SLP_TYPb = 0;
        uint16_t SLP_EN = 0;
        uint16_t SCI_EN = 0;
        uint8_t PM1_CNT_LEN = 0;

        ACPI *acpi;
        void OnInterruptReceived(CPU::x64::TrapFrame *Frame);

    public:
        bool ACPIShutdownSupported = false;

        void Reboot();
        void Shutdown();

        DSDT(ACPI *acpi);
        ~DSDT();
    };
}

#endif // !__FENNIX_KERNEL_ACPI_H__
