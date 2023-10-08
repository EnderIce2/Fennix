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

#ifndef __FENNIX_KERNEL_DHCP_H__
#define __FENNIX_KERNEL_DHCP_H__

#include <net/ipv4.hpp>
#include <net/udp.hpp>
#include <net/nc.hpp>
#include <types.h>

namespace NetworkDHCP
{
    struct DHCPHeader
    {
        uint8_t Opcode;
        uint8_t HardwareType;
        uint8_t HardwareAddressLength;
        uint8_t Hops;
        uint32_t TransactionID;
        uint16_t Seconds;
        uint16_t Flags;
        uint32_t ClientIP;
        uint32_t YourIP;
        uint32_t ServerIP;
        uint32_t GatewayIP;
        uint8_t ClientHardwareAddress[16];
        uint8_t ServerHostName[64];
        uint8_t BootFileName[128];
        uint8_t Options[64];
    } __packed;

    enum DHCPOperation
    {
        DHCP_OP_BOOTREQUEST = 1,
        DHCP_OP_BOOTREPLY = 2
    };

    /* TODO: Complete list from https://en.wikipedia.org/wiki/Dynamic_Host_Configuration_Protocol#Options */
    enum DHCPOption
    {
        DHCP_OPTION_PAD = 0,
        DHCP_OPTION_SUBNETMASK = 1,
        DHCP_OPTION_TIME_OFFSET = 2,
        DHCP_OPTION_ROUTER = 3,
        DHCP_OPTION_TIME_SERVER = 4,
        DHCP_OPTION_NAME_SERVER = 5,
        DHCP_OPTION_DOMAIN_NAME_SERVER = 6,
        DHCP_OPTION_LOG_SERVER = 7,
        DHCP_OPTION_COOKIE_SERVER = 8,
        DHCP_OPTION_LPR_SERVER = 9,
        DHCP_OPTION_IMPRESS_SERVER = 10,
        DHCP_OPTION_RESOURCE_LOCATION_SERVER = 11,
        DHCP_OPTION_HOST_NAME = 12,
        DHCP_OPTION_BOOT_FILE_SIZE = 13,
        DHCP_OPTION_MERIT_DUMP_FILE = 14,
        DHCP_OPTION_DOMAIN_NAME = 15,
        DHCP_OPTION_SWAP_SERVER = 16,
        DHCP_OPTION_ROOT_PATH = 17,
        DHCP_OPTION_EXTENSION_PATH = 18,

        DHCP_OPTION_IP_FORWARDING = 19,
        DHCP_OPTION_NON_LOCAL_SOURCE_ROUTING = 20,
        DHCP_OPTION_POLICY_FILTER = 21,
        DHCP_OPTION_MAX_DATAGRAM_REASSEMBLY_SIZE = 22,
        DHCP_OPTION_DEFAULT_IP_TTL = 23,
        DHCP_OPTION_PATH_MTU_AGING_TIMEOUT = 24,
        DHCP_OPTION_PATH_MTU_PLATEAU_TABLE = 25,

        DHCP_OPTION_INTERFACE_MTU = 26,
        DHCP_OPTION_ALL_SUBNETS_ARE_LOCAL = 27,
        DHCP_OPTION_BROADCAST_ADDRESS = 28,
        DHCP_OPTION_PERFORM_MASK_DISCOVERY = 29,
        DHCP_OPTION_MASK_SUPPLIER = 30,
        DHCP_OPTION_ROUTER_DISCOVERY = 31,
        DHCP_OPTION_ROUTER_SOLICITATION_ADDRESS = 32,
        DHCP_OPTION_STATIC_ROUTE = 33,

        DHCP_OPTION_TRAILER_ENCAPSULATION = 34,
        DHCP_OPTION_ARP_CACHE_TIMEOUT = 35,
        DHCP_OPTION_ETHERNET_ENCAPSULATION = 36,

        DHCP_OPTION_DEFAULT_TCP_TTL = 37,
        DHCP_OPTION_TCP_KEEPALIVE_INTERVAL = 38,
        DHCP_OPTION_TCP_KEEPALIVE_GARBAGE = 39,

        DHCP_OPTION_NIS_DOMAIN = 40,
        DHCP_OPTION_NIS_SERVERS = 41,
        DHCP_OPTION_NTP_SERVERS = 42,
        DHCP_OPTION_VENDOR_SPECIFIC = 43,
        DHCP_OPTION_NETBIOS_NAME_SERVERS = 44,
        DHCP_OPTION_NETBIOS_DD_SERVER = 45,
        DHCP_OPTION_NETBIOS_NODE_TYPE = 46,
        DHCP_OPTION_NETBIOS_SCOPE = 47,
        DHCP_OPTION_X_FONT_SERVERS = 48,
        DHCP_OPTION_X_DISPLAY_MANAGER = 49,

        DHCP_OPTION_REQUESTED_IP = 50,
        DHCP_OPTION_IP_LEASE_TIME = 51,
        DHCP_OPTION_OPTION_OVERLOAD = 52,
        DHCP_OPTION_MESSAGE_TYPE = 53,
        DHCP_OPTION_SERVER_IDENTIFIER = 54,
        DHCP_OPTION_PARAMETER_REQUEST_LIST = 55,
        DHCP_OPTION_MESSAGE = 56,
        DHCP_OPTION_MAX_MESSAGE_SIZE = 57,
        DHCP_OPTION_T1_TIMEOUT = 58,
        DHCP_OPTION_T2_TIMEOUT = 59,
        DHCP_OPTION_VENDOR_CLASS_IDENTIFIER = 60,
        DHCP_OPTION_CLIENT_IDENTIFIER = 61,

        DHCP_OPTION_NETWORK_TIME_SERVER = 62,

        DHCP_OPTION_END = 255
    };

    enum DHCPMessageType
    {
        DHCP_MESSAGE_TYPE_DISCOVER = 1,
        DHCP_MESSAGE_TYPE_OFFER = 2,
        DHCP_MESSAGE_TYPE_REQUEST = 3,
        DHCP_MESSAGE_TYPE_DECLINE = 4,
        DHCP_MESSAGE_TYPE_ACK = 5,
        DHCP_MESSAGE_TYPE_NAK = 6,
        DHCP_MESSAGE_TYPE_RELEASE = 7,
        DHCP_MESSAGE_TYPE_INFORM = 8,
        DHCP_MESSAGE_TYPE_FORCERENEW = 9,
        DHCP_MESSAGE_TYPE_LEASEQUERY = 10,
        DHCP_MESSAGE_TYPE_LEASEUNASSIGNED = 11,
        DHCP_MESSAGE_TYPE_LEASEUNKNOWN = 12,
        DHCP_MESSAGE_TYPE_LEASEACTIVE = 13,
        DHCP_MESSAGE_TYPE_BULKLEASEQUERY = 14,
        DHCP_MESSAGE_TYPE_LEASEQUERYDONE = 15,
        DHCP_MESSAGE_TYPE_ACTIVELEASEQUERY = 16,
        DHCP_MESSAGE_TYPE_LEASEQUERYSTATUS = 17,
        DHCP_MESSAGE_TYPE_DHCPTLS = 18
    };

#define DHCP_TRANSACTION_ID 0xFE2EC005

    class DHCP : public NetworkUDP::UDPEvents
    {
    private:
        NetworkUDP::Socket *UDPSocket;
        NetworkInterfaceManager::DeviceInterface *Interface;
        bool Received = false;

        void CreatePacket(DHCPHeader *Packet, uint8_t MessageType, uint32_t RequestIP);
        void *GetOption(DHCPHeader *Packet, uint8_t Type);
        void OnUDPPacketReceived(NetworkUDP::Socket *Socket, uint8_t *Data, size_t Length);

    public:
        /** @brief IP address (Little-endian) */
        InternetProtocol IP = {};
        /** @brief Gateway address (Little-endian) */
        InternetProtocol Gateway = {};
        /** @brief Subnet mask (Little-endian) */
        InternetProtocol SubNetworkMask = {};
        /** @brief DNS server address (Little-endian) */
        InternetProtocol DomainNameSystem = {};

        DHCP(NetworkUDP::Socket *Socket, NetworkInterfaceManager::DeviceInterface *Interface);
        ~DHCP();
        void Request();
        void Request(InternetProtocol IP);
    };
}

#endif // !__FENNIX_KERNEL_DHCP_H__
