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

#ifndef __FENNIX_KERNEL_NETWORK_ARP_H__
#define __FENNIX_KERNEL_NETWORK_ARP_H__

#include <net/eth.hpp>
#include <net/nc.hpp>
#include <types.h>
#include <vector>

namespace NetworkARP
{
    enum ARPOperation
    {
        REQUEST = 0x1,
        REPLY = 0x2
    };

    enum ARPHardwareType
    {
        HTYPE_ETHERNET = 1,
        HTYPE_802_3 = 6,
        HTYPE_ARCNET = 7,
        HTYPE_FRAME_RELAY = 15,
        HTYPE_ATM = 16,
        HTYPE_HDLC = 17,
        HTYPE_FIBRE_CHANNEL = 18,
        HTYPE_ATM_2 = 19,
        HTYPE_SERIAL_LINE = 20
    };

    struct ARPHeader
    {
        uint16_t HardwareType;
        uint16_t ProtocolType;
        uint8_t HardwareSize;
        uint8_t ProtocolSize;
        uint16_t Operation;
        uint48_t SenderMAC : 48;
        uint32_t SenderIP;
        uint48_t TargetMAC : 48;
        uint32_t TargetIP;
    } __packed;

    struct DiscoveredAddress
    {
        MediaAccessControl MAC;
        InternetProtocol IP;
    };

    class ARP : public NetworkEthernet::EthernetEvents
    {
    private:
        NetworkEthernet::Ethernet *Ethernet;

        enum DAType
        {
            DA_ADD = 1,
            DA_DEL = 2,
            DA_SEARCH = 3,
            DA_UPDATE = 4
        };

        std::vector<NetworkARP::DiscoveredAddress *> DiscoveredAddresses;
        DiscoveredAddress *ManageDiscoveredAddresses(DAType Type, InternetProtocol IP, MediaAccessControl MAC);
        DiscoveredAddress *Search(InternetProtocol TargetIP);
        DiscoveredAddress *Update(InternetProtocol TargetIP, MediaAccessControl TargetMAC);
        bool OnEthernetPacketReceived(uint8_t *Data, size_t Length);

    public:
        ARP(NetworkEthernet::Ethernet *Ethernet);
        ~ARP();

        /**
         * @brief Resolve an IP address to a MAC address.
         *
         * @param IP The IP address to resolve. (Little-endian)
         * @return uint48_t The MAC address of the IP address.
         */
        uint48_t Resolve(InternetProtocol IP);

        /**
         * @brief Broadcast an ARP packet.
         *
         * @param IP The IP address to broadcast.
         */
        void Broadcast(InternetProtocol IP);
    };
}

#endif // !__FENNIX_KERNEL_NETWORK_ARP_H__
