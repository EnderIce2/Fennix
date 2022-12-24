#ifndef __FENNIX_KERNEL_IPv4_H__
#define __FENNIX_KERNEL_IPv4_H__

#include <net/ipv4.hpp>
#include <net/arp.hpp>
#include <net/nc.hpp>
#include <types.h>

namespace NetworkIPv4
{
    struct IPv4Header
    {
        uint8_t IHL : 4;
        uint8_t Version : 4;
        uint8_t TypeOfService;
        uint16_t TotalLength;
        uint16_t Identification;
        uint16_t FlagsAndFragmentOffset;
        uint8_t TimeToLive;
        uint8_t Protocol;
        uint16_t HeaderChecksum;
        uint32_t SourceIP;
        uint32_t DestinationIP;

        /* On wikipedia page we have this: https://en.wikipedia.org/wiki/File:IPv4_Packet-en.svg
           but only the code above works... */
        // uint8_t Version : 4;
        // uint8_t IHL : 4;
        // uint16_t TypeOfService : 8;
        // uint16_t TotalLength : 12;
        // uint16_t Identification : 16;
        // uint16_t Flags : 3;
        // uint16_t FragmentOffset : 13;
        // uint8_t TimeToLive : 8;
        // uint8_t Protocol : 8;
        // uint16_t HeaderChecksum;
        // uint32_t SourceIP;
        // uint32_t DestinationIP;
    };

    struct IPv4Packet
    {
        IPv4Header Header;
        uint8_t Data[];
    };

    enum IPv4Protocols
    {
        PROTOCOL_ICMP = 1,
        PROTOCOL_IGMP = 2,
        PROTOCOL_TCP = 6,
        PROTOCOL_UDP = 17,
        PROTOCOL_IPV6 = 41,
        PROTOCOL_ROUTING = 43,
        PROTOCOL_FRAGMENT = 44,
        PROTOCOL_ESP = 50,
        PROTOCOL_AH = 51,
        PROTOCOL_ICMPV6 = 58,
        PROTOCOL_NONE = 59,
        PROTOCOL_DSTOPTS = 60,
        PROTOCOL_ND = 77,
        PROTOCOL_ICLFXBM = 78,
        PROTOCOL_PIM = 103,
        PROTOCOL_COMP = 108,
        PROTOCOL_SCTP = 132,
        PROTOCOL_UDPLITE = 136,
        PROTOCOL_RAW = 255
    };

    class IPv4 : public NetworkEthernet::EthernetEvents
    {
    private:
        NetworkARP::ARP *ARP;
        NetworkEthernet::Ethernet *Ethernet;

        virtual bool OnEthernetPacketReceived(uint8_t *Data, uint64_t Length);

    public:
        InternetProtocol4 GatewayIP = {.Address = {0xFF, 0xFF, 0xFF, 0xFF}};
        InternetProtocol4 SubNetworkMaskIP = {.Address = {0xFF, 0xFF, 0xFF, 0xFF}};
        IPv4(NetworkARP::ARP *ARP, NetworkEthernet::Ethernet *Ethernet);
        ~IPv4();

        /**
         * @brief Send an IPv4 packet.
         *
         * @param Data The data to send.
         * @param Length The length of the data.
         * @param Protocol The protocol of the packet.
         * @param DestinationIP The IP address of the destination. (Big-endian)
         */
        void Send(uint8_t *Data, uint64_t Length, uint8_t Protocol, InternetProtocol4 DestinationIP);
    };

    class IPv4Events
    {
    private:
        uint8_t Protocol;

    protected:
        IPv4Events(IPv4Protocols Protocol);
        ~IPv4Events();

    public:
        uint8_t GetProtocol() { return Protocol; }

        virtual bool OnIPv4PacketReceived(InternetProtocol4 SourceIP, InternetProtocol4 DestinationIP, uint8_t *Data, uint64_t Length)
        {
            warn("Not implemented.");
            return false;
        }
    };
}

#endif // !__FENNIX_KERNEL_IPv4_H__
