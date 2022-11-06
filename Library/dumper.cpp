#include "dumper.hpp"

#include <printf.h>
#include <uart.hpp>
#include <lock.hpp>
#include <md5.h>

NewLock(DumperLock);

using namespace UniversalAsynchronousReceiverTransmitter;

static inline void print_wrapper(char c, void *unused)
{
    UART(COM1).Write(c);
    UNUSED(unused);
}

int vprintf(const char *format, va_list list) { return vfctprintf(print_wrapper, NULL, format, list); }

void WriteRaw(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void DumpData(const char *Description, void *Address, unsigned long Length)
{
    SmartLock(DumperLock);
    WriteRaw("-------------------------------------------------------------------------\n");
    unsigned char *AddressChar = (unsigned char *)Address;
    unsigned char Buffer[17];
    unsigned long Iterate;

    if (Description != nullptr)
        WriteRaw("%s:\n", Description);

    for (Iterate = 0; Iterate < Length; Iterate++)
    {
        if ((Iterate % 16) == 0)
        {
            if (Iterate != 0)
                WriteRaw("  %s\n", Buffer);
            WriteRaw("  %04x ", Iterate);
        }

        WriteRaw(" %02x", AddressChar[Iterate]);

        if ((AddressChar[Iterate] < 0x20) || (AddressChar[Iterate] > 0x7e))
            Buffer[Iterate % 16] = '.';
        else
            Buffer[Iterate % 16] = AddressChar[Iterate];

        Buffer[(Iterate % 16) + 1] = '\0';
    }

    while ((Iterate % 16) != 0)
    {
        WriteRaw("   ");
        Iterate++;
    }

    WriteRaw("  %s\n", Buffer);
    WriteRaw("-------------------------------------------------------------------------\n");
    WriteRaw("Length: %ld bytes\n", Length);
    uint8_t *result = md5File(AddressChar, Length);
    WriteRaw("MD5: ");
    for (int i = 0; i < 16; i++)
        WriteRaw("%02x", result[i]);
    WriteRaw("\n-------------------------------------------------------------------------\n");
}
