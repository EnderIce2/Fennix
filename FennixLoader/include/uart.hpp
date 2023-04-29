#ifndef __FENNIX_KERNEL_UART_H__
#define __FENNIX_KERNEL_UART_H__

#include <types.h>

namespace UniversalAsynchronousReceiverTransmitter
{
    enum SerialPorts
    {
        COMNULL = 0,
        COM1 = 0x3F8,
        COM2 = 0x2F8,
        COM3 = 0x3E8,
        COM4 = 0x2E8,
        COM5 = 0x5F8,
        COM6 = 0x4F8,
        COM7 = 0x5E8,
        COM8 = 0x4E8
    };

    class UART
    {
    private:
        SerialPorts Port;

    public:
        UART(SerialPorts Port = COMNULL);
        ~UART();
        void Write(uint8_t Char);
        uint8_t Read();
    };
}

#endif // !__FENNIX_KERNEL_UART_H__
