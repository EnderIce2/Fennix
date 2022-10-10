#ifndef __FENNIX_KERNEL_BOOT_INFO_H__
#define __FENNIX_KERNEL_BOOT_INFO_H__

#include <types.h>

enum MemoryType
{
    Usable,
    Reserved,
    ACPIReclaimable,
    ACPINVS,
    BadMemory,
    BootloaderReclaimable,
    KernelAndModules,
    Framebuffer,
    Unknown
};

#define MAX_FRAMEBUFFERS 16
#define MAX_MEMORY_ENTRIES 256
#define MAX_MODULES 16

struct BootInfo
{
    struct FramebufferInfo
    {
        void *BaseAddress;
        uint64_t Width;
        uint64_t Height;
        uint64_t Pitch;
        uint16_t BitsPerPixel;
        uint8_t MemoryModel;
        uint8_t RedMaskSize;
        uint8_t RedMaskShift;
        uint8_t GreenMaskSize;
        uint8_t GreenMaskShift;
        uint8_t BlueMaskSize;
        uint8_t BlueMaskShift;
        void *ExtendedDisplayIdentificationData;
        uint64_t EDIDSize;
    } Framebuffer[MAX_FRAMEBUFFERS];

    struct MemoryInfo
    {
        struct MemoryEntryInfo
        {
            void *BaseAddress;
            uint64_t Length;
            enum MemoryType Type;
        } Entry[MAX_MEMORY_ENTRIES];
        uint64_t Entries;
        uint64_t Size;
    } Memory;

    struct ModuleInfo
    {
        void *Address;
        char Path[256];
        char CommandLine[256];
        uint64_t Size;
    } Modules[MAX_MODULES];

    struct RSDPInfo
    {
        /**
         * @brief Signature
         */
        unsigned char Signature[8];
        /**
         * @brief Checksum
         */
        uint8_t Checksum;
        /**
         * @brief OEM ID
         */
        uint8_t OEMID[6];
        /**
         * @brief Revision
         */
        uint8_t Revision;
        /**
         * @brief Address of the Root System Description Table
         */
        uint32_t RSDTAddress;
        /* END OF RSDP 1.0 */

        /**
         * @brief Length
         */
        uint32_t Length;
        /**
         * @brief Extended System Descriptor Table
         */
        uint64_t XSDTAddress;
        /**
         * @brief Extended checksum
         */
        uint8_t ExtendedChecksum;
        /**
         * @brief Reserved
         */
        uint8_t Reserved[3];
    } __attribute__((packed)) * RSDP;

    struct KernelInfo
    {
        void *PhysicalBase;
        void *VirtualBase;
        void *FileBase;
        char CommandLine[256];
        uint64_t Size;
    } Kernel;

    struct BootloaderInfo
    {
        char Name[256];
        char Version[64];
    } Bootloader;
};

#endif // !__FENNIX_KERNEL_BOOT_INFO_H__
