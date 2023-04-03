/*
   BSD 3-Clause License

   Copyright (c) 2023, EnderIce2
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this
      list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

   3. Neither the name of the copyright holder nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
   OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

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

enum FexDriverInputTypes
{
    FexDriverInputTypes_None = 0b00000000,
    FexDriverInputTypes_Mouse = 0b00000001,
    FexDriverInputTypes_Keyboard = 0b00000010,
    /* ... */
};

struct Fex
{
    char Magic[4];
    enum FexFormatType Type : 4;
    enum FexOSType OS : 4;
    int (*EntryPoint)(void *);
} __attribute__((packed));

union KernelCallback;
union CPURegisters;

struct FexExtended
{
    struct
    {

    } Executable;

    struct
    {
        char Name[64];
        enum FexDriverType Type : 4;
        enum FexDriverInputTypes TypeFlags : 4;
        char OverrideOnConflict : 1;
        int (*Callback)(union KernelCallback *);
        int (*InterruptCallback)(union CPURegisters *);

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

            struct
            {
                char AttachToMouse;
                char AttachToKeyboard;
            } Input;
        } Bind;
    } Driver;
} __attribute__((packed));

/**
 * @brief Add file header
 *
 * @param FormatType FexFormatType
 * @param OperatingSystem FexOSType
 * @param Address EntryPoint to the start function
 *
 * @note Must include ".header : { *(.header .header.*) }" in linker script
 */
#define HEAD(FormatType, OperatingSystem, Address)               \
    __attribute__((section(".header"))) struct Fex FexHeader = { \
        .Magic = {'F', 'E', 'X', '\0'},                          \
        .Type = FormatType,                                      \
        .OS = OperatingSystem,                                   \
        .EntryPoint = Address}

#endif // !__FENNIX_FILE_FEX_H__
