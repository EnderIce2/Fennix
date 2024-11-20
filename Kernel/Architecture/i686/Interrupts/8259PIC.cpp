#include "pic.hpp"

#include <io.h>

namespace PIC
{
    PIC::PIC(uint8_t MasterCommandPort, uint8_t MasterDataPort, uint8_t SlaveCommandPort, uint8_t SlaveDataPort, uint8_t MasterOffset, uint8_t SlaveOffset)
    {
        this->MasterCommandPort = MasterCommandPort;
        this->MasterDataPort = MasterDataPort;
        this->SlaveCommandPort = SlaveCommandPort;
        this->SlaveDataPort = SlaveDataPort;
        this->MasterOffset = MasterOffset;
        this->SlaveOffset = SlaveOffset;

        MasterMask = 0xFF;
        SlaveMask = 0xFF;

        // ICW1
        outb(MasterCommandPort, 0x11);
        outb(SlaveCommandPort, 0x11);

        // ICW2
        outb(MasterDataPort, MasterOffset);
        outb(SlaveDataPort, SlaveOffset);

        // ICW3
        outb(MasterDataPort, 0x04);
        outb(SlaveDataPort, 0x02);

        // ICW4
        outb(MasterDataPort, 0x01);
        outb(SlaveDataPort, 0x01);

        // OCW1
        outb(MasterDataPort, MasterMask);
        outb(SlaveDataPort, SlaveMask);
    }

    PIC::~PIC()
    {
        outb(MasterDataPort, 0xFF);
        outb(SlaveDataPort, 0xFF);
    }

    void PIC::Mask(uint8_t IRQ)
    {
        uint16_t Port;
        uint8_t Value;

        if (IRQ < 8)
        {
            Port = MasterDataPort;
            Value = MasterMask & ~(1 << IRQ);
            MasterMask = Value;
        }
        else
        {
            Port = SlaveDataPort;
            Value = SlaveMask & ~(1 << (IRQ - 8));
            SlaveMask = Value;
        }

        outb(Port, Value);
    }

    void PIC::Unmask(uint8_t IRQ)
    {
        uint16_t Port;
        uint8_t Value;

        if (IRQ < 8)
        {
            Port = MasterDataPort;
            Value = MasterMask | (1 << IRQ);
            MasterMask = Value;
        }
        else
        {
            Port = SlaveDataPort;
            Value = SlaveMask | (1 << (IRQ - 8));
            SlaveMask = Value;
        }

        outb(Port, Value);
    }

    void PIC::SendEOI(uint8_t IRQ)
    {
        if (IRQ >= 8)
            outb(SlaveCommandPort, 0x20);

        outb(MasterCommandPort, 0x20);
    }

    PIT::PIT(uint16_t Port, uint16_t Frequency)
    {
        this->Port = Port;
        this->Frequency = Frequency;
    }

    PIT::~PIT()
    {
    }

    void PIT::PrepareSleep(uint32_t Milliseconds)
    {
        uint16_t Divisor = 1193182 / Frequency;
        uint8_t Low = (uint8_t)(Divisor & 0xFF);
        uint8_t High = (uint8_t)((Divisor >> 8) & 0xFF);

        outb(Port + 3, 0x36);
        outb(Port + 0, Low);
        outb(Port + 1, High);
    }

    void PIT::PerformSleep()
    {
        uint8_t Value = inb(Port + 0);
        while (Value != 0)
            Value = inb(Port + 0);
    }
}
