#include <net/udp.hpp>
#include <debug.h>

#include "../kernel.h"

namespace NetworkUDP
{
    struct EventInfo
    {
        Socket *UDPSocket;
        uint16_t Port;
    };
    Vector<EventInfo> RegisteredEvents;

    UDPEvents::UDPEvents() {}

    UDPEvents::~UDPEvents() {}

    /* -------------------------------------------------------------------------------------------------------------------------------- */

    UDP::UDP(NetworkIPv4::IPv4 *ipv4, NetworkInterfaceManager::DeviceInterface *Interface) : NetworkIPv4::IPv4Events(NetworkIPv4::PROTOCOL_UDP)
    {
        this->ipv4 = ipv4;
        this->Interface = Interface;
    }

    UDP::~UDP() {}

    uint16_t UsablePort = 0x200;

    Socket *UDP::Connect(InternetProtocol IP, uint16_t Port)
    {
        netdbg("Connecting to %s", IP.v4.ToStringLittleEndian(), Port);
        Socket *socket = new Socket(this);
        socket->RemoteIP = IP;
        socket->RemotePort = Port;
        socket->LocalPort = UsablePort++; // TODO: track ports
        socket->LocalIP = Interface->IP;
        socket->LocalPort = b16(socket->LocalPort);
        socket->RemotePort = b16(socket->RemotePort);
        RegisteredEvents.push_back({.UDPSocket = socket, .Port = socket->LocalPort});
        return socket;
    }

    Socket *UDP::Listen(uint16_t Port)
    {
        fixme("Not implemented.");
        return nullptr;
    }

    void UDP::Disconnect(Socket *Socket)
    {
        fixme("Not implemented.");
    }

    void UDP::Send(Socket *Socket, uint8_t *Data, uint64_t Length)
    {
        netdbg("Sending %d bytes to %s", Length, Socket->RemoteIP.v4.ToStringLittleEndian(), Socket->RemotePort);
        uint16_t TotalLength = Length + sizeof(UDPHeader);
        UDPPacket *packet = (UDPPacket *)kmalloc(TotalLength);
        packet->Header.SourcePort = Socket->LocalPort;
        packet->Header.DestinationPort = Socket->RemotePort;
        packet->Header.Length = b16(TotalLength);
        memcpy(packet->Data, Data, Length);
        packet->Header.Checksum = 0; // I totally should do this. Some devices may require it.
        // packet->Header.Checksum = CalculateChecksum((uint16_t *)packet, TotalLength);
        this->ipv4->Send((uint8_t *)packet, TotalLength, 0x11, Socket->RemoteIP);
        kfree(packet);
    }

    void UDP::Bind(Socket *Socket, UDPEvents *EventHandler)
    {
        netdbg("Binding socket to %s", Socket->LocalIP.v4.ToStringLittleEndian(), Socket->LocalPort);
        Socket->EventHandler = EventHandler;
    }

    bool UDP::OnIPv4PacketReceived(InternetProtocol SourceIP, InternetProtocol DestinationIP, uint8_t *Data, uint64_t Length)
    {
        netdbg("Received %d bytes from %s", Length, SourceIP.v4.ToStringLittleEndian());
        if (Length < sizeof(UDPHeader))
            return false;

        UDPHeader *udp = (UDPHeader *)Data;

        netdbg("SP:%d | DP:%d | L:%d | CHK:%#x", b16(udp->SourcePort), b16(udp->DestinationPort), b16(udp->Length), b16(udp->Checksum));

        Socket *GoodSocket = nullptr;

        foreach (auto &var in RegisteredEvents)
        {
            netdbg("UDP->SKT[]: LP:%d | LIP:%s | RP:%d | RIP:%s | LST:%d",
                   b16(var.UDPSocket->LocalPort),
                   var.UDPSocket->LocalIP.v4.ToStringLittleEndian(),
                   b16(var.UDPSocket->RemotePort),
                   var.UDPSocket->RemoteIP.v4.ToStringLittleEndian(),
                   b16(var.UDPSocket->Listening));
            if (var.UDPSocket->LocalPort == udp->DestinationPort &&
                var.UDPSocket->LocalIP.v4 == DestinationIP.v4 &&
                var.UDPSocket->Listening == true)
            {
                var.UDPSocket->Listening = false;
                var.UDPSocket->RemotePort = b16(udp->SourcePort);
                var.UDPSocket->RemoteIP = SourceIP;
                netdbg("E1");
                return true;
            }

            GoodSocket = var.UDPSocket;
        }
        if (GoodSocket)
            GoodSocket->EventHandler->OnUDPPacketReceived(GoodSocket, ((UDPPacket *)Data)->Data, Length);

        netdbg("E0 (Success)");
        return false;
    }

    /* -------------------------------------------------------------------------------------------------------------------------------- */

    Socket::Socket(UDP *_UDP) { this->SocketUDP = _UDP; }

    Socket::~Socket() {}
}
