#include <pci.h>
#include <io.h>

#include "../../../Kernel/DAPI.hpp"
#include "../../../Kernel/Fex.hpp"

extern "C" int DriverEntry(void *Data);
int CallbackHandler(KernelCallback *Data);
int InterruptCallback(CPURegisters *Registers);

HEAD(FexFormatType_Driver, FexOSType_Fennix, DriverEntry);

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

__attribute__((section(".extended"))) FexExtended ExtendedHeader = {
    .Driver = {
        .Name = "PS/2 Mouse Driver",
        .Type = FexDriverType_Input,
        .TypeFlags = FexDriverInputTypes_Mouse,
        .OverrideOnConflict = false,
        .Callback = CallbackHandler,
        .InterruptCallback = InterruptCallback,
        .Bind = {
            .Type = BIND_INTERRUPT,
            .Interrupt = {
                .Vector = {12}, // IRQ12
            }}}};

KernelAPI *KAPI;

#define print(msg) KAPI->Util.DebugPrint((char *)(msg), KAPI->Info.DriverUID)

/* --------------------------------------------------------------------------------------------------------- */

enum Config
{
    READ_CONFIG = 0x20,
    WRITE_CONFIG = 0x60
};

enum Ports
{
    DATA = 0x60,
    STATUS = 0x64,
    COMMAND = 0x64,
};

enum State
{
    OUTPUT_FULL = (1 << 0),
    INPUT_FULL = (1 << 1),
    MOUSE_BYTE = (1 << 5)
};

#define PS2LeftButton 0b00000001
#define PS2MiddleButton 0b00000100
#define PS2RightButton 0b00000010
#define PS2XSign 0b00010000
#define PS2YSign 0b00100000
#define PS2XOverflow 0b01000000
#define PS2YOverflow 0b10000000

int MouseX = 0, MouseY = 0, MouseZ = 0;
int MouseLeft = 0, MouseMiddle = 0, MouseRight = 0;

uint8_t Packet[4];
bool PacketReady = false;
uint8_t Cycle = 0;

void WaitRead()
{
    uint64_t Timeout = 100000;
    while (Timeout--)
        if (inb(Ports::STATUS) & State::OUTPUT_FULL)
            return;
}

void WaitWrite()
{
    uint64_t Timeout = 100000;
    while (Timeout--)
        if ((inb(Ports::STATUS) & State::INPUT_FULL) == 0)
            return;
}

uint8_t Read()
{
    WaitRead();
    return inb(Ports::DATA);
}

void Write(uint16_t Port, uint8_t Value)
{
    WaitWrite();
    outb(Port, Value);
}

int DriverEntry(void *Data)
{
    if (!Data)
        return INVALID_KERNEL_API;
    KAPI = (KernelAPI *)Data;
    if (KAPI->Version.Major < 0 || KAPI->Version.Minor < 0 || KAPI->Version.Patch < 0)
        return KERNEL_API_VERSION_NOT_SUPPORTED;

    return OK;
}

int CallbackHandler(KernelCallback *Data)
{
    switch (Data->Reason)
    {
    case AcknowledgeReason:
    {
        print("Kernel acknowledged the driver.");
        break;
    }
    case ConfigurationReason:
    {
        outb(COMMAND, 0xA8);
        Write(COMMAND, READ_CONFIG);
        uint8_t Status = Read();
        Status |= 0b10;
        Write(COMMAND, WRITE_CONFIG);
        Write(DATA, Status);
        Write(COMMAND, 0xD4);
        Write(DATA, 0xF6);
        Read();
        Write(COMMAND, 0xD4);
        Write(DATA, 0xF4);
        Read();

        print("PS/2 mouse configured.");
        break;
    }
    case FetchReason:
    {
        Data->InputCallback.Mouse.X = MouseX;
        Data->InputCallback.Mouse.Y = MouseY;
        Data->InputCallback.Mouse.Z = MouseZ;
        Data->InputCallback.Mouse.Buttons.Left = MouseLeft;
        Data->InputCallback.Mouse.Buttons.Right = MouseRight;
        Data->InputCallback.Mouse.Buttons.Middle = MouseMiddle;
        break;
    }
    case StopReason:
    {
        outb(COMMAND, 0xA8);
        Write(COMMAND, READ_CONFIG);
        uint8_t Status = Read();
        Status &= ~0b10;
        Write(COMMAND, WRITE_CONFIG);
        Write(DATA, Status);
        Write(COMMAND, 0xD4);
        Write(DATA, 0xF5);
        Read();

        print("Driver stopped.");
        break;
    }
    default:
    {
        print("Unknown reason.");
        break;
    }
    }
    return OK;
}

int InterruptCallback(CPURegisters *)
{
    uint8_t Data = inb(0x60);

    if (__builtin_expect(!!(PacketReady), 0))
    {
        bool XNegative, YNegative, XOverflow, YOverflow;

        if (Packet[0] & PS2XSign)
            XNegative = true;
        else
            XNegative = false;

        if (Packet[0] & PS2YSign)
            YNegative = true;
        else
            YNegative = false;

        if (Packet[0] & PS2XOverflow)
            XOverflow = true;
        else
            XOverflow = false;

        if (Packet[0] & PS2YOverflow)
            YOverflow = true;
        else
            YOverflow = false;

        if (!XNegative)
        {
            MouseX += Packet[1];
            if (XOverflow)
                MouseX += 255;
        }
        else
        {
            Packet[1] = 256 - Packet[1];
            MouseX -= Packet[1];
            if (XOverflow)
                MouseX -= 255;
        }

        if (!YNegative)
        {
            MouseY -= Packet[2];
            if (YOverflow)
                MouseY -= 255;
        }
        else
        {
            Packet[2] = 256 - Packet[2];
            MouseY += Packet[2];
            if (YOverflow)
                MouseY += 255;
        }

        uint32_t Width = KAPI->Display.GetWidth();
        uint32_t Height = KAPI->Display.GetHeight();

        if (MouseX < 0)
            MouseX = 0;

        if ((uint32_t)MouseX > Width - 1)
            MouseX = Width - 1;

        if (MouseY < 0)
            MouseY = 0;
        if ((uint32_t)MouseY > Height - 1)
            MouseY = Height - 1;

        MouseLeft = 0;
        MouseMiddle = 0;
        MouseRight = 0;

        if (Packet[0] & PS2LeftButton)
            MouseLeft = 1;
        if (Packet[0] & PS2MiddleButton)
            MouseMiddle = 1;
        if (Packet[0] & PS2RightButton)
            MouseRight = 1;
        PacketReady = false;
    }

    switch (Cycle)
    {
    case 0:
    {
        if ((Data & 0b00001000) == 0)
            break;
        Packet[0] = Data;
        Cycle++;
        break;
    }
    case 1:
    {
        Packet[1] = Data;
        Cycle++;
        break;
    }
    case 2:
    {
        Packet[2] = Data;
        PacketReady = true;
        Cycle = 0;
        break;
    }
    }
    return OK;
}
