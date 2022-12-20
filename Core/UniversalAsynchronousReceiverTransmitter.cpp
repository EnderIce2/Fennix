#include <uart.hpp>

#include <vector.hpp>
#include <debug.h>

volatile bool serialports[8] = {false, false, false, false, false, false, false, false};
Vector<UniversalAsynchronousReceiverTransmitter::Events *> RegisteredEvents;

#if defined(__amd64__) || defined(__i386__)
__no_instrument_function uint8_t NoProfiler_inportb(uint16_t Port)
{
    uint8_t Result;
    asm("in %%dx, %%al"
        : "=a"(Result)
        : "d"(Port));
    return Result;
}

__no_instrument_function void NoProfiler_outportb(uint16_t Port, uint8_t Data)
{
    asmv("out %%al, %%dx"
         :
         : "a"(Data), "d"(Port));
}
#endif

namespace UniversalAsynchronousReceiverTransmitter
{
#define SERIAL_ENABLE_DLAB 0x80
#define SERIAL_RATE_38400_LO 0x03
#define SERIAL_RATE_38400_HI 0x00
#define SERIAL_BUFFER_EMPTY 0x20

    SafeFunction __no_instrument_function UART::UART(SerialPorts Port)
    {
#if defined(__amd64__) || defined(__i386__)
        if (Port == COMNULL)
            return;

        this->Port = Port;
        int PortNumber = 0;

        switch (Port)
        {
        case COM1:
            PortNumber = 0;
            break;
        case COM2:
            PortNumber = 1;
            break;
        case COM3:
            PortNumber = 2;
            break;
        case COM4:
            PortNumber = 3;
            break;
        case COM5:
            PortNumber = 4;
            break;
        case COM6:
            PortNumber = 5;
            break;
        case COM7:
            PortNumber = 6;
            break;
        case COM8:
            PortNumber = 7;
            break;
        default:
            return;
        }

        if (serialports[PortNumber])
            return;

        // Initialize the serial port
        NoProfiler_outportb(Port + 1, 0x00);                 // Disable all interrupts
        NoProfiler_outportb(Port + 3, SERIAL_ENABLE_DLAB);   // Enable DLAB (set baud rate divisor)
        NoProfiler_outportb(Port + 0, SERIAL_RATE_38400_LO); // Set divisor to 3 (lo byte) 38400 baud
        NoProfiler_outportb(Port + 1, SERIAL_RATE_38400_HI); //                  (hi byte)
        NoProfiler_outportb(Port + 3, 0x03);                 // 8 bits, no parity, one stop bit
        NoProfiler_outportb(Port + 2, 0xC7);                 // Enable FIFO, clear them, with 14-byte threshold
        NoProfiler_outportb(Port + 4, 0x0B);                 // IRQs enabled, RTS/DSR set

        // Check if the serial port is faulty.
        if (NoProfiler_inportb(Port + 0) != 0xAE)
        {
            static int once = 0;
            if (!once++)
                warn("Serial port %#llx is faulty.", Port);
            // serialports[Port] = false; // ignore for now
            // return;
        }

        // Set to normal operation mode.
        NoProfiler_outportb(Port + 4, 0x0F);
        serialports[PortNumber] = true;
#endif
    }

    SafeFunction __no_instrument_function UART::~UART() {}

    SafeFunction __no_instrument_function void UART::Write(uint8_t Char)
    {
#if defined(__amd64__) || defined(__i386__)
        while ((NoProfiler_inportb(Port + 5) & SERIAL_BUFFER_EMPTY) == 0)
            ;
        NoProfiler_outportb(Port, Char);
#endif
        foreach (auto e in RegisteredEvents)
            if (e->GetRegisteredPort() == Port || e->GetRegisteredPort() == COMNULL)
                e->OnSent(Char);
    }

    SafeFunction __no_instrument_function uint8_t UART::Read()
    {
#if defined(__amd64__) || defined(__i386__)
        while ((NoProfiler_inportb(Port + 5) & 1) == 0)
            ;
        return NoProfiler_inportb(Port);
#endif
        foreach (auto e in RegisteredEvents)
        {
            if (e->GetRegisteredPort() == Port || e->GetRegisteredPort() == COMNULL)
            {
#if defined(__amd64__) || defined(__i386__)
                e->OnReceived(NoProfiler_inportb(Port));
#endif
            }
        }
    }

    SafeFunction __no_instrument_function Events::Events(SerialPorts Port)
    {
        this->Port = Port;
        RegisteredEvents.push_back(this);
    }

    SafeFunction __no_instrument_function Events::~Events()
    {
        for (uintptr_t i = 0; i < RegisteredEvents.size(); i++)
            if (RegisteredEvents[i] == this)
            {
                RegisteredEvents.remove(i);
                return;
            }
    }
}
