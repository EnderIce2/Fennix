#ifndef __FENNIX_KERNEL_ICMPv6_H__
#define __FENNIX_KERNEL_ICMPv6_H__

#include <net/nc.hpp>
#include <types.h>

namespace NetworkICMPv6
{
    struct ICMPHeader
    {
        uint8_t Type;
        uint8_t Code;
        uint16_t Checksum;
        uint16_t Identifier;
        uint16_t SequenceNumber;
    };

    struct ICMPPacket
    {
        ICMPHeader Header;
        uint8_t Data[];
    };

    class ICMPv6
    {
    private:
        NetworkInterfaceManager::DeviceInterface *Interface;

    public:
        NetworkInterfaceManager::DeviceInterface *GetInterface() { return this->Interface; }

        ICMPv6(NetworkInterfaceManager::DeviceInterface *Interface);
        ~ICMPv6();
        void Send(uint8_t *Data, uint64_t Length);
        void Receive(uint8_t *Data);
    };
}

#endif // !__FENNIX_KERNEL_ICMPv6_H__
