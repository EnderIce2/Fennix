#ifndef __FENNIX_KERNEL_IPv6_H__
#define __FENNIX_KERNEL_IPv6_H__

#include <types.h>

namespace NetworkIPv6
{
    struct IPv6Header
    {
        uint32_t Version;
        uint8_t TrafficClass;
        uint16_t FlowLabel;
        uint16_t PayloadLength;
        uint8_t NextHeader;
        uint8_t HopLimit;
        uint32_t SourceIP;
        uint32_t DestinationIP;
    };

    struct IPv6Packet
    {
        IPv6Header Header;
        uint8_t Data[];
    };
}

#endif // !__FENNIX_KERNEL_IPv6_H__
