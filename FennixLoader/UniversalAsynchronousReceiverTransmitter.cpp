#include <uart.hpp>
#include <debug.h>

volatile bool serialports[8] = {false, false, false, false, false, false, false, false};


__always_inline inline uint8_t inportb(uint16_t Port)
{
    uint8_t Result;
    asm("in %%dx, %%al"
        : "=a"(Result)
        : "d"(Port));
    return Result;
}

__always_inline inline void outportb(uint16_t Port, uint8_t Data)
{
    asmv("out %%al, %%dx"
         :
         : "a"(Data), "d"(Port));
}

namespace UniversalAsynchronousReceiverTransmitter
{
#define SERIAL_ENABLE_DLAB 0x80
#define SERIAL_RATE_115200_LO 0x01
#define SERIAL_RATE_115200_HI 0x00
#define SERIAL_RATE_57600_LO 0x02
#define SERIAL_RATE_57600_HI 0x00
#define SERIAL_RATE_38400_LO 0x03
#define SERIAL_RATE_38400_HI 0x00
#define SERIAL_BUFFER_EMPTY 0x20

    /* TODO: Serial Port implementation needs reword. https://wiki.osdev.org/Serial_Ports */

    SafeFunction UART::UART(SerialPorts Port)
    {
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
        outportb(s_cst(uint16_t, Port + 1), 0x00);                  // Disable all interrupts
        outportb(s_cst(uint16_t, Port + 3), SERIAL_ENABLE_DLAB);    // Enable DLAB (set baud rate divisor)
        outportb(s_cst(uint16_t, Port + 0), SERIAL_RATE_115200_LO); // Set divisor to 1 (lo byte) 115200 baud
        outportb(s_cst(uint16_t, Port + 1), SERIAL_RATE_115200_HI); //                  (hi byte)
        outportb(s_cst(uint16_t, Port + 3), 0x03);                  // 8 bits, no parity, one stop bit
        outportb(s_cst(uint16_t, Port + 2), 0xC7);                  // Enable FIFO, clear them, with 14-byte threshold
        outportb(s_cst(uint16_t, Port + 4), 0x0B);                  // IRQs enabled, RTS/DSR set

        // Check if the serial port is faulty.
        if (inportb(s_cst(uint16_t, Port + 0)) != 0xAE)
        {
            static int once = 0;
            if (!once++)
                warn("Serial port %#llx is faulty.", Port);
            // serialports[Port] = false; // ignore for now
            // return;
        }

        // Set to normal operation mode.
        outportb(s_cst(uint16_t, Port + 4), 0x0F);
        serialports[PortNumber] = true;
    }

    SafeFunction UART::~UART() {}

    SafeFunction void UART::Write(uint8_t Char)
    {
        while ((inportb(s_cst(uint16_t, Port + 5)) & SERIAL_BUFFER_EMPTY) == 0)
            ;
        outportb(Port, Char);
    }

    SafeFunction uint8_t UART::Read()
    {
        while ((inportb(s_cst(uint16_t, Port + 5)) & 1) == 0)
            ;
        return inportb(Port);
    }
}
