#ifndef __FENNIX_FILE_FEX_H__
#define __FENNIX_FILE_FEX_H__

// TODO: EXPERIMENTAL

/**
 * @brief Fex file format (not finalized)
 *
 * @note Instead of HEAD, we can include everything in linker script like this:
 * @example .header : { BYTE(0x46) BYTE(0x45) BYTE(0x58) BYTE(0x0) } for 'F' 'E' 'X' '\0'
 *
 */

/** @brief This may change */
#define EXTENDED_SECTION_ADDRESS 0x20

enum FexFormatType
{
    FexFormatType_Unknown,
    FexFormatType_Executable,
    FexFormatType_Driver
    /* ... */
};

enum FexOSType
{
    FexOSType_Unknown,
    FexOSType_Fennix,
    FexOSType_Linux
    /* ... */
};

enum FexDriverType
{
    FexDriverType_Unknown,
    FexDriverType_Generic,
    FexDriverType_Display,
    FexDriverType_Network,
    FexDriverType_Storage,
    FexDriverType_FileSystem,
    FexDriverType_Input,
    FexDriverType_Audio
    /* ... */
};

struct Fex
{
    char Magic[4];
    enum FexFormatType Type : 4;
    enum FexOSType OS : 4;
    int (*Pointer)(void *);
} __attribute__((packed));

struct FexExtended
{
    struct
    {

    } Executable;

    struct
    {
        char Name[64];
        enum FexDriverType Type : 4;
        int (*Callback)(struct KernelCallback *);

        struct DriverBind
        {
            int Type;
            struct
            {
                unsigned char Vector[16];
            } Interrupt;

            struct
            {
                unsigned int ProcessId[16];
            } Process;

            struct
            {
                unsigned short VendorID[16];
                unsigned short DeviceID[16];
                unsigned short Class;
                unsigned short SubClass;
                unsigned short ProgIF;
            } PCI;
        } Bind;
    } Driver;
} __attribute__((packed));

/**
 * @brief Add file header
 *
 * @param FormatType FexFormatType
 * @param OperatingSystem FexOSType
 * @param Address Pointer to the start function
 *
 * @note Must include ".header : { *(.header .header.*) }" in linker script
 */
#define HEAD(FormatType, OperatingSystem, Address)        \
    __attribute__((section(".header"))) Fex FexHeader = { \
        .Magic = {'F', 'E', 'X', '\0'},                   \
        .Type = FormatType,                               \
        .OS = OperatingSystem,                            \
        .Pointer = Address}

#endif // !__FENNIX_FILE_FEX_H__
