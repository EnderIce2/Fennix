#include "kernel.h"

#include <power.hpp>
#include <printf.h>

void KernelMainThread()
{
    KPrint("Kernel main thread started!");
    // asm("int $0x1");

    KPrint("\e22AA11Starting color test.");
    uint32_t Color = 0x000000;
    int itrupd = 0;
    int BoxSize = 64;

    uint32_t x, y;

    Display->GetBufferCursor(0, &x, &y);

    while (1)
    {
        Color++;
        itrupd++;

        Display->SetBufferCursor(0, 0, 0);

        for (int i = 0; i < BoxSize; i++)
            for (int j = 0; j < BoxSize; j++)
                Display->SetPixel(i, j, Color, 0);

        int r = (Color >> 16) & 0xFF;
        int g = (Color >> 8) & 0xFF;
        int b = (Color >> 0) & 0xFF;
        printf_("\e%02x%02x%02x0x%06X",
                255 - r, 255 - g, 255 - b,
                Color);

        if (itrupd == BoxSize)
            itrupd = 0;

        if (itrupd == 0)
            Display->SetBuffer(0);

        if (Color > 0xFFFFFF)
            break;
    }

    Display->SetBufferCursor(0, x, y);
    KPrint("\e22AA11Color test finished.");

    while (1)
        CPU::Halt();
}

void KernelShutdownThread(bool Reboot)
{
    KPrint("Kernel shutdown thread started!");
    if (Reboot)
        PowerManager->Reboot();
    else
        PowerManager->Shutdown();

    CPU::Stop();
}
