#ifndef __FENNIX_KERNEL_BOOT_INFO_H__
#define __FENNIX_KERNEL_BOOT_INFO_H__

enum MemoryType
{
    Unknown,
    Usable,
    Reserved,
    ACPIReclaimable,
    ACPINVS,
    BadMemory,
    BootloaderReclaimable,
    KernelAndModules,
    Framebuffer
};

#define MAX_FRAMEBUFFERS 16
#define MAX_MEMORY_ENTRIES 256
#define MAX_MODULES 16

struct BootInfo
{
    struct FramebufferInfo
    {
        void *BaseAddress;
        __UINT32_TYPE__ Width;
        __UINT32_TYPE__ Height;
        __UINT64_TYPE__ Pitch;
        __UINT16_TYPE__ BitsPerPixel;
        __UINT8_TYPE__ MemoryModel;
        __UINT8_TYPE__ RedMaskSize;
        __UINT8_TYPE__ RedMaskShift;
        __UINT8_TYPE__ GreenMaskSize;
        __UINT8_TYPE__ GreenMaskShift;
        __UINT8_TYPE__ BlueMaskSize;
        __UINT8_TYPE__ BlueMaskShift;
        void *ExtendedDisplayIdentificationData;
        __UINT64_TYPE__ EDIDSize;
    } Framebuffer[MAX_FRAMEBUFFERS];

    struct MemoryInfo
    {
        struct MemoryEntryInfo
        {
            void *BaseAddress;
            __UINT64_TYPE__ Length;
            enum MemoryType Type;
        } Entry[MAX_MEMORY_ENTRIES];
        __UINT64_TYPE__ Entries;
        __UINT64_TYPE__ Size;
    } Memory;

    struct ModuleInfo
    {
        void *Address;
        char Path[256];
        char CommandLine[256];
        __UINT64_TYPE__ Size;
    } Modules[MAX_MODULES];

    struct RSDPInfo
    {
        /**
         * @brief Signature
         */
        __UINT8_TYPE__ Signature[8];
        /**
         * @brief Checksum
         */
        __UINT8_TYPE__ Checksum;
        /**
         * @brief OEM ID
         */
        __UINT8_TYPE__ OEMID[6];
        /**
         * @brief Revision
         */
        __UINT8_TYPE__ Revision;
        /**
         * @brief Address of the Root System Description Table
         */
        __UINT32_TYPE__ RSDTAddress;
        /* END OF RSDP 1.0 */

        /**
         * @brief Length
         */
        __UINT32_TYPE__ Length;
        /**
         * @brief Extended System Descriptor Table
         */
        __UINT64_TYPE__ XSDTAddress;
        /**
         * @brief Extended checksum
         */
        __UINT8_TYPE__ ExtendedChecksum;
        /**
         * @brief Reserved
         */
        __UINT8_TYPE__ Reserved[3];
    } __attribute__((packed)) * RSDP;

    struct KernelInfo
    {
        void *PhysicalBase;
        void *VirtualBase;
        void *FileBase;
        char CommandLine[256];
        __UINT64_TYPE__ Size;
    } Kernel;

    struct BootloaderInfo
    {
        char Name[256];
        char Version[64];
    } Bootloader;

    void *SMBIOSPtr;
};

#endif // !__FENNIX_KERNEL_BOOT_INFO_H__
