/*
	This file is part of Fennix Kernel.

	Fennix Kernel is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Kernel is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Kernel. If not, see <https://www.gnu.org/licenses/>.
*/

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
    } __packed;

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
        virtual void OnUDPPacketReceived(Socket *Socket, uint8_t *Data, size_t Length)
        {
            UNUSED(Socket);
            UNUSED(Data);
            UNUSED(Length);
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

        virtual Socket *Connect(InternetProtocol IP, uint16_t Port);
        virtual Socket *Listen(uint16_t Port);
        virtual void Disconnect(Socket *Socket);
        virtual void Send(Socket *Socket, uint8_t *Data, size_t Length);
        virtual void Bind(Socket *Socket, UDPEvents *EventHandler);

        virtual bool OnIPv4PacketReceived(InternetProtocol SourceIP, InternetProtocol DestinationIP, uint8_t *Data, size_t Length);
    };

    class Socket
    {
    public:
        InternetProtocol LocalIP;
        uint16_t LocalPort = 0;
        InternetProtocol RemoteIP;
        uint16_t RemotePort = 0;
        bool Listening = false;
        UDPEvents *EventHandler = nullptr;
        UDP *SocketUDP = nullptr;

        Socket(UDP *_UDP);
        ~Socket();
    };
}

#endif // !__FENNIX_KERNEL_UDP_H__
