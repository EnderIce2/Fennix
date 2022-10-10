#include <uart.hpp>

#include <vector.hpp>
#include <debug.h>
#include <io.h>

volatile bool serialports[8] = {false, false, false, false, false, false, false, false};
Vector<UniversalAsynchronousReceiverTransmitter::Events *> RegisteredEvents;

namespace UniversalAsynchronousReceiverTransmitter
{
#define SERIAL_ENABLE_DLAB 0x80
#define SERIAL_RATE_38400_LO 0x03
#define SERIAL_RATE_38400_HI 0x00
#define SERIAL_BUFFER_EMPTY 0x20

    UART::UART(SerialPorts Port)
    {
#if defined(__amd64__) || defined(__i386__)
        if (Port == COMNULL)
            return;

        this->Port = Port;

        if (serialports[Port])
            return;

        // Initialize the serial port
        outb(Port + 1, 0x00);                 // Disable all interrupts
        outb(Port + 3, SERIAL_ENABLE_DLAB);   // Enable DLAB (set baud rate divisor)
        outb(Port + 0, SERIAL_RATE_38400_LO); // Set divisor to 3 (lo byte) 38400 baud
        outb(Port + 1, SERIAL_RATE_38400_HI); //                  (hi byte)
        outb(Port + 3, 0x03);                 // 8 bits, no parity, one stop bit
        outb(Port + 2, 0xC7);                 // Enable FIFO, clear them, with 14-byte threshold
        outb(Port + 4, 0x0B);                 // IRQs enabled, RTS/DSR set

        // Check if the serial port is faulty.
        if (inb(Port + 0) != 0xAE)
        {
            static int once = 0;
            if (!once++)
                warn("Serial port %#lx is faulty.", Port);
            // serialports[Port] = false; // ignore for now
            // return;
        }

        // Set to normal operation mode.
        outb(Port + 4, 0x0F);
        serialports[Port] = true;
#endif
    }

    UART::~UART() {}

    void UART::Write(uint8_t Char)
    {
#if defined(__amd64__) || defined(__i386__)
        while ((inb(Port + 5) & SERIAL_BUFFER_EMPTY) == 0)
            ;
        outb(Port, Char);
#endif
        foreach (auto e in RegisteredEvents)
            if (e->GetRegisteredPort() == Port || e->GetRegisteredPort() == COMNULL)
                e->OnSent(Char);
    }

    uint8_t UART::Read()
    {
#if defined(__amd64__) || defined(__i386__)
        while ((inb(Port + 5) & 1) == 0)
            ;
        return inb(Port);
#endif
        foreach (auto e in RegisteredEvents)
        {
            if (e->GetRegisteredPort() == Port || e->GetRegisteredPort() == COMNULL)
            {
#if defined(__amd64__) || defined(__i386__)
                e->OnReceived(inb(Port));
#endif
            }
        }
    }

    Events::Events(SerialPorts Port)
    {
        this->Port = Port;
        RegisteredEvents.push_back(this);
    }

    Events::~Events()
    {
        for (uint64_t i = 0; i < RegisteredEvents.size(); i++)
            if (RegisteredEvents[i] == this)
            {
                RegisteredEvents.remove(i);
                return;
            }
    }
}
