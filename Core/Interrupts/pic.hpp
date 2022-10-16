#ifndef __FENNIX_KERNEL_8259PIC_H__
#define __FENNIX_KERNEL_8259PIC_H__

#include <types.h>

namespace PIC
{
    class PIC
    {
    private:
        uint8_t MasterCommandPort;
        uint8_t MasterDataPort;
        uint8_t SlaveCommandPort;
        uint8_t SlaveDataPort;
        uint8_t MasterOffset;
        uint8_t SlaveOffset;
        uint8_t MasterMask;
        uint8_t SlaveMask;

    public:
        PIC(uint8_t MasterCommandPort, uint8_t MasterDataPort, uint8_t SlaveCommandPort, uint8_t SlaveDataPort, uint8_t MasterOffset, uint8_t SlaveOffset);
        ~PIC();
        void Mask(uint8_t IRQ);
        void Unmask(uint8_t IRQ);
        void SendEOI(uint8_t IRQ);
    };

    class PIT
    {
    private:
        uint16_t Port;
        uint16_t Frequency;

    public:
        PIT(uint16_t Port, uint16_t Frequency);
        ~PIT();
        void PrepareSleep(uint32_t Milliseconds);
        void PerformSleep();
    };
}

#endif // !__FENNIX_KERNEL_8259PIC_H__
