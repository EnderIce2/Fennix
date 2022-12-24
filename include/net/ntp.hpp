#ifndef __FENNIX_KERNEL_NTP_H__
#define __FENNIX_KERNEL_NTP_H__

#include <net/udp.hpp>
#include <types.h>

namespace NetworkNTP
{
    struct NTPHeader
    {
        uint8_t LIv;
        uint8_t VN;
        uint8_t Mode;
        uint8_t Stratum;
        uint8_t Poll;
        uint8_t Precision;
        uint32_t RootDelay;
        uint32_t RootDispersion;
        uint32_t ReferenceID;
        uint32_t ReferenceTimestamp;
        uint32_t OriginateTimestamp;
        uint32_t ReceiveTimestamp;
        uint32_t TransmitTimestamp;
    } __attribute__((packed));

    class NTP : public NetworkUDP::UDPEvents
    {
    private:
        NetworkUDP::Socket *Socket;

    public:
        NTP(NetworkUDP::Socket *Socket);
        ~NTP();

        void ReadTime();
    };
}

#endif // !__FENNIX_KERNEL_NTP_H__
