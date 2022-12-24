#ifndef __FENNIX_KERNEL_UDP_H__
#define __FENNIX_KERNEL_UDP_H__

#include <net/ipv4.hpp>
#include <net/udp.hpp>
#include <net/nc.hpp>
#include <types.h>

namespace NetworkUDP
{
    struct UDPHeader
    {
        uint16_t SourcePort;
        uint16_t DestinationPort;
        uint16_t Length;
        uint16_t Checksum;
    } __attribute__((packed));

    struct UDPPacket
    {
        UDPHeader Header;
        uint8_t Data[];
    };

    class Socket;

    class UDPEvents
    {
    protected:
        UDPEvents();
        ~UDPEvents();

    public:
        virtual void OnUDPPacketReceived(Socket *Socket, uint8_t *Data, uint64_t Length)
        {
            warn("Not implemented.");
        }
    };

    class UDP : public NetworkIPv4::IPv4Events
    {
    private:
        NetworkIPv4::IPv4 *ipv4;
        NetworkInterfaceManager::DeviceInterface *Interface;

    public:
        NetworkInterfaceManager::DeviceInterface *GetInterface() { return this->Interface; }

        UDP(NetworkIPv4::IPv4 *ipv4, NetworkInterfaceManager::DeviceInterface *Interface);
        ~UDP();

        virtual Socket *Connect(InternetProtocol4 IP, uint16_t Port);
        virtual Socket *Listen(uint16_t Port);
        virtual void Disconnect(Socket *Socket);
        virtual void Send(Socket *Socket, uint8_t *Data, uint64_t Length);
        virtual void Bind(Socket *Socket, UDPEvents *EventHandler);

        virtual bool OnIPv4PacketReceived(InternetProtocol4 SourceIP, InternetProtocol4 DestinationIP, uint8_t *Data, uint64_t Length);
    };

    class Socket
    {
    public:
        InternetProtocol4 LocalIP = {.Address = {0xFF, 0xFF, 0xFF, 0xFF}};
        uint16_t LocalPort = 0;
        InternetProtocol4 RemoteIP = {.Address = {0xFF, 0xFF, 0xFF, 0xFF}};
        uint16_t RemotePort = 0;
        bool Listening = false;
        UDPEvents *EventHandler = nullptr;
        UDP *SocketUDP = nullptr;

        Socket(UDP *_UDP);
        ~Socket();
    };
}

#endif // !__FENNIX_KERNEL_UDP_H__
