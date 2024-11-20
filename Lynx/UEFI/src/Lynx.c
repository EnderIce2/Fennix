#include <efi.h>
#include <efilib.h>

#include "Memory/memory.hpp"
#include "FileLoader.h"

#include "printf.h"

void port_byte_out(short unsigned int port, unsigned char value)
{
    __asm__ volatile("outb %0, %1"
                     :
                     : "a"(value), "Nd"(port));
}

unsigned char port_byte_in(short unsigned int port)
{
    unsigned char ReturnValue;
    __asm__ volatile("inb %1, %0"
                     : "=a"(ReturnValue)
                     : "Nd"(port));
    return ReturnValue;
}

int strlen(const char s[])
{
    int i = 0;
    while (s[i] != '\0')
        ++i;
    return i;
}

int init_serial()
{
    // TODO: fix crash on virtualbox (or virtualbox is faulty???????????)
    port_byte_out(0x3F8 + 1, 0x00);
    port_byte_out(0x3F8 + 3, 0x80);
    port_byte_out(0x3F8 + 0, 0x03);
    port_byte_out(0x3F8 + 1, 0x00);
    port_byte_out(0x3F8 + 3, 0x03);
    port_byte_out(0x3F8 + 2, 0xC7);
    port_byte_out(0x3F8 + 4, 0x0B);
    port_byte_out(0x3F8 + 4, 0x1E);
    port_byte_out(0x3F8 + 0, 0xAE);
    if (port_byte_in(0x3F8 + 0) != 0xAE)
    {
        return -1; // serial port is faulty
    }
    port_byte_out(0x3F8 + 4, 0x0F);
    return 0;
}

void printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    printf_(format, args);
    va_end(args);
}

extern void putchar(char c)
{
    while ((port_byte_in(0x3F8 + 5) & 0x20) == 0)
        ;
    port_byte_out(0x3F8, c);
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    InitializeLib(ImageHandle, SystemTable);
    SystemTable->BootServices->SetWatchdogTimer(0, 0, 0, NULL);
    Print(L"Lynx Bootloader © EnderIce2 2022\n");
    Print(L"UEFI not implemented\n");
            while (1)
            asm("hlt");
    InitializeMemoryManagement(ImageHandle, SystemTable);
    EFI_FILE *Kernel = LoadFile(NULL, L"fennix.elf", ImageHandle, SystemTable);

    if (Kernel == NULL)
    {
        Print(L"Kernel not found\n");
        while (1)
            asm("hlt");
    }

    while (1)
        asm("hlt");
    return EFI_SUCCESS;
}
