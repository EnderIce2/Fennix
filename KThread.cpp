#include "kernel.h"

#include <power.hpp>
#include <printf.h>

void kerneltest1()
{
    uint32_t Color = 0x000000;
    int BoxSize = 64;
    while (1)
    {
        Color++;
        for (int i = 0; i < BoxSize; i++)
            for (int j = 0; j < BoxSize; j++)
                Display->SetPixel(i + 64, j, Color, 0);
        Display->SetBuffer(0);
        if (Color > 0xFFFFFF)
            break;
    }
}
void kerneltest2()
{
    uint32_t Color = 0x000000;
    int BoxSize = 64;
    while (1)
    {
        Color++;
        for (int i = 0; i < BoxSize; i++)
            for (int j = 0; j < BoxSize; j++)
                Display->SetPixel(i + 128, j, Color, 0);
        Display->SetBuffer(0);
        if (Color > 0xFFFFFF)
            break;
    }
}
void kerneltest3()
{
    uint32_t Color = 0x000000;
    int BoxSize = 64;
    while (1)
    {
        Color++;
        for (int i = 0; i < BoxSize; i++)
            for (int j = 0; j < BoxSize; j++)
                Display->SetPixel(i + 192, j, Color, 0);
        Display->SetBuffer(0);
        if (Color > 0xFFFFFF)
            break;
    }
}
void kerneltest4()
{
    uint32_t Color = 0x000000;
    int BoxSize = 64;
    while (1)
    {
        Color++;
        for (int i = 0; i < BoxSize; i++)
            for (int j = 0; j < BoxSize; j++)
                Display->SetPixel(i + 256, j, Color, 0);
        Display->SetBuffer(0);
        if (Color > 0xFFFFFF)
            break;
    }
}
void kerneltest5()
{
    uint32_t Color = 0x000000;
    int BoxSize = 64;
    while (1)
    {
        Color++;
        for (int i = 0; i < BoxSize; i++)
            for (int j = 0; j < BoxSize; j++)
                Display->SetPixel(i + 320, j, Color, 0);
        Display->SetBuffer(0);
        if (Color > 0xFFFFFF)
            break;
    }
}
void kerneltest6()
{
    uint32_t Color = 0x000000;
    int BoxSize = 64;
    while (1)
    {
        Color++;
        for (int i = 0; i < BoxSize; i++)
            for (int j = 0; j < BoxSize; j++)
                Display->SetPixel(i + 384, j, Color, 0);
        Display->SetBuffer(0);
        if (Color > 0xFFFFFF)
            break;
    }
}
void kerneltest7()
{
    uint32_t Color = 0x000000;
    int BoxSize = 64;
    while (1)
    {
        Color++;
        for (int i = 0; i < BoxSize; i++)
            for (int j = 0; j < BoxSize; j++)
                Display->SetPixel(i + 448, j, Color, 0);
        Display->SetBuffer(0);
        if (Color > 0xFFFFFF)
            break;
    }
}
void kerneltest8()
{
    uint32_t Color = 0x000000;
    int BoxSize = 64;
    while (1)
    {
        Color++;
        for (int i = 0; i < BoxSize; i++)
            for (int j = 0; j < BoxSize; j++)
                Display->SetPixel(i + 512, j, Color, 0);
        Display->SetBuffer(0);
        if (Color > 0xFFFFFF)
            break;
    }
}
void kerneltest9()
{
    uint32_t Color = 0x000000;
    int BoxSize = 64;
    while (1)
    {
        Color++;
        for (int i = 0; i < BoxSize; i++)
            for (int j = 0; j < BoxSize; j++)
                Display->SetPixel(i + 576, j, Color, 0);
        Display->SetBuffer(0);
        if (Color > 0xFFFFFF)
            break;
    }
}
void kerneltest10()
{
    uint32_t Color = 0x000000;
    int BoxSize = 64;
    while (1)
    {
        Color++;
        for (int i = 0; i < BoxSize; i++)
            for (int j = 0; j < BoxSize; j++)
                Display->SetPixel(i + 640, j, Color, 0);
        Display->SetBuffer(0);
        if (Color > 0xFFFFFF)
            break;
    }
}

void KernelMainThread()
{
    KPrint("Kernel main thread started!");

    KPrint("Kernel Compiled at: %s %s with C++ Standard: %d\n", __DATE__, __TIME__, CPP_LANGUAGE_STANDARD);
    KPrint("C++ Language Version (__cplusplus) :%ld\n", __cplusplus);

    // asm("int $0x1");

    KPrint("\e22AA11Starting color test.");

    CPU::Interrupts(CPU::Disable);
    TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (Tasking::IP)kerneltest1);
    KPrint("\e22AA11Thread 1 created.");
    TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (Tasking::IP)kerneltest2);
    KPrint("\e22AA11Thread 2 created.");
    TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (Tasking::IP)kerneltest3);
    KPrint("\e22AA11Thread 3 created.");
    TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (Tasking::IP)kerneltest4);
    KPrint("\e22AA11Thread 4 created.");
    TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (Tasking::IP)kerneltest5);
    KPrint("\e22AA11Thread 5 created.");
    TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (Tasking::IP)kerneltest6);
    KPrint("\e22AA11Thread 6 created.");
    TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (Tasking::IP)kerneltest7);
    KPrint("\e22AA11Thread 7 created.");
    TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (Tasking::IP)kerneltest8);
    KPrint("\e22AA11Thread 8 created.");
    TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (Tasking::IP)kerneltest9);
    KPrint("\e22AA11Thread 9 created.");
    TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (Tasking::IP)kerneltest10);
    KPrint("\e22AA11Thread 10 created.");
    CPU::Interrupts(CPU::Enable);

    uint32_t Color = 0x000000;
    int BoxSize = 64;
    uint32_t x, y;

    Display->GetBufferCursor(0, &x, &y);

    while (1)
    {
        Color++;

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
