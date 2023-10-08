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

#include <net/dhcp.hpp>
#include <debug.h>

#include "../kernel.h"

namespace NetworkDHCP
{
    DHCP::DHCP(NetworkUDP::Socket *Socket, NetworkInterfaceManager::DeviceInterface *Interface)
    {
        debug("DHCP interface %#lx created.", this);
        this->UDPSocket = Socket;
        this->Interface = Interface;
        Socket->LocalPort = b16(68);

        InternetProtocol::Version4 DefaultIPv4 = {.Address = {0x0, 0x0, 0x0, 0x0}};
        InternetProtocol::Version6 DefaultIPv6 = {.Address = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}};

        this->IP.v4 = DefaultIPv4;
        this->IP.v6 = DefaultIPv6;

        this->Gateway.v4 = DefaultIPv4;
        this->Gateway.v6 = DefaultIPv6;

        this->SubNetworkMask.v4 = DefaultIPv4;
        this->SubNetworkMask.v6 = DefaultIPv6;

        this->DomainNameSystem.v4 = DefaultIPv4;
        this->DomainNameSystem.v6 = DefaultIPv6;
    }

    DHCP::~DHCP()
    {
        debug("DHCP interface %#lx destroyed.", this);
    }

    __no_sanitize("alignment") void DHCP::CreatePacket(DHCPHeader *Packet, uint8_t MessageType, uint32_t RequestIP)
    {
        Packet->Opcode = b8(DHCP_OP_BOOTREQUEST);
        Packet->HardwareType = b8(1);
        Packet->HardwareAddressLength = b8(6);
        Packet->Hops = b8(0);
        Packet->TransactionID = b32(DHCP_TRANSACTION_ID);
        Packet->Flags = b16(0x40);
        uint48_t InterfaceMAC = b48(Interface->MAC.ToHex());
        memcpy(Packet->ClientHardwareAddress, &InterfaceMAC, sizeof(InterfaceMAC));

        uint8_t *Ptr = Packet->Options;
        *((uint32_t *)(Ptr)) = b32(0x63825363); // Magic Cookie
        Ptr += 4;

        *(Ptr++) = DHCP_OPTION_MESSAGE_TYPE;
        *(Ptr++) = DHCP_MESSAGE_TYPE_DISCOVER;
        *(Ptr++) = MessageType;

        *(Ptr++) = DHCP_OPTION_CLIENT_IDENTIFIER;
        *(Ptr++) = 0x07;
        *(Ptr++) = 0x01;
        memcpy(Ptr, &InterfaceMAC, sizeof(InterfaceMAC));
        Ptr += 6;

        *(Ptr++) = DHCP_OPTION_REQUESTED_IP;
        *(Ptr++) = 0x04;
        *((uint32_t *)(Ptr)) = b32(0x0a00020e);
        memcpy((uint32_t *)(Ptr), &RequestIP, 4);
        Ptr += 4;

        *(Ptr++) = DHCP_OPTION_HOST_NAME;
        char *HostName = (char *)KERNEL_NAME;
        *(Ptr++) = s_cst(uint8_t, 1 + strlen(HostName));
        memcpy(Ptr, HostName, strlen(HostName));
        Ptr += strlen(HostName);

        *(Ptr++) = DHCP_OPTION_PAD;

        *(Ptr++) = DHCP_OPTION_PARAMETER_REQUEST_LIST;
        *(Ptr++) = DHCP_OPTION_COOKIE_SERVER;
        *(Ptr++) = DHCP_OPTION_SUBNETMASK;
        *(Ptr++) = DHCP_OPTION_ROUTER;
        *(Ptr++) = DHCP_OPTION_DOMAIN_NAME_SERVER;
        *(Ptr++) = DHCP_OPTION_DOMAIN_NAME;
        *(Ptr++) = DHCP_OPTION_NETBIOS_NAME_SERVERS;
        *(Ptr++) = DHCP_OPTION_NETBIOS_NODE_TYPE;
        *(Ptr++) = DHCP_OPTION_NETBIOS_SCOPE;
        *(Ptr++) = DHCP_OPTION_MAX_MESSAGE_SIZE;

        *(Ptr++) = DHCP_OPTION_END;
    }

    void DHCP::Request()
    {
        netdbg("Requesting IP address");
        DHCPHeader packet;
        memset(&packet, 0, sizeof(DHCPHeader));

        CreatePacket(&packet, DHCP_MESSAGE_TYPE_DISCOVER, 0x00000000);
        this->UDPSocket->SocketUDP->Send(this->UDPSocket, (uint8_t *)&packet, sizeof(DHCPHeader));

        debug("Waiting for response...");
        int RequestTimeout = 20;
        while (!Received)
        {
            if (--RequestTimeout == 0)
            {
                warn("Request timeout.");
                break;
            }
            netdbg("Still waiting...");
            TaskManager->Sleep(1000);
        }
    }

    void DHCP::Request(InternetProtocol IP)
    {
        netdbg("Requesting IP address %s", IP.v4.ToStringLittleEndian());
        DHCPHeader packet;
        memset(&packet, 0, sizeof(DHCPHeader));

        /* CreatePacket() accepts IP as MSB */
        CreatePacket(&packet, DHCP_MESSAGE_TYPE_REQUEST, b32(IP.v4.ToHex()));
        UDPSocket->SocketUDP->Send(UDPSocket, (uint8_t *)&packet, sizeof(DHCPHeader));
    }

    void *DHCP::GetOption(DHCPHeader *Packet, uint8_t Type)
    {
        uint8_t *Option = Packet->Options + 4;
        uint8_t Current = *Option;
        while (Current != 0xFF)
        {
            uint8_t OptionLength = *(Option + 1);
            if (Current == Type)
                return Option + 2;
            Option += (2 + OptionLength);
            Current = *Option;
        }
        warn("Option %#x not found", Type);
        return nullptr;
    }

    __no_sanitize("alignment") void DHCP::OnUDPPacketReceived(NetworkUDP::Socket *Socket, uint8_t *Data, size_t Length)
    {
        UNUSED(Socket);
        UNUSED(Length);
        DHCPHeader *Packet = (DHCPHeader *)Data;
        uint8_t *MessageType = (uint8_t *)GetOption(Packet, DHCP_OPTION_MESSAGE_TYPE);

        switch (*MessageType)
        {
        case DHCP_OPTION_TIME_OFFSET:
        {
            InternetProtocol ReqIP;
            ReqIP.v4.FromHex(b32(Packet->YourIP));
            netdbg("Received DHCP offer for IP %s", ReqIP.v4.ToStringLittleEndian());
            this->Request(ReqIP);
            break;
        }
        case DHCP_OPTION_NAME_SERVER:
            this->IP.v4.FromHex(b32(Packet->YourIP));
            this->Gateway.v4.FromHex(b32((uint32_t)(*(uintptr_t *)GetOption(Packet, DHCP_OPTION_ROUTER))));
            this->DomainNameSystem.v4.FromHex(b32((uint32_t)(*(uintptr_t *)GetOption(Packet, DHCP_OPTION_DOMAIN_NAME_SERVER))));
            this->SubNetworkMask.v4.FromHex(b32((uint32_t)(*(uintptr_t *)GetOption(Packet, DHCP_OPTION_SUBNETMASK))));
            this->Received = true;
            netdbg("Received DHCP ACK for IP %s", this->IP.v4.ToStringLittleEndian());
            break;
        default:
            warn("Received unknown message type %#x", *MessageType);
            break;
        }
    }
}
