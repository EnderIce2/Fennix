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

#ifndef __FENNIX_KERNEL_ICMPv4_H__
#define __FENNIX_KERNEL_ICMPv4_H__

#include <net/nc.hpp>
#include <types.h>

namespace NetworkICMPv4
{
    enum ICMPv4Type
    {
        TYPE_ECHO_REPLY = 0,
        TYPE_DESTINATION_UNREACHABLE = 3,
        TYPE_SOURCE_QUENCH = 4,
        TYPE_REDIRECT = 5,
        TYPE_ECHO = 8,
        TYPE_ROUTER_ADVERTISEMENT = 9,
        TYPE_ROUTER_SELECTION = 10,
        TYPE_TIME_EXCEEDED = 11,
        TYPE_PARAMETER_PROBLEM = 12,
        TYPE_TIMESTAMP = 13,
        TYPE_TIMESTAMP_REPLY = 14
    };

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

    class ICMPv4
    {
    private:
        NetworkInterfaceManager::DeviceInterface *Interface;

    public:
        NetworkInterfaceManager::DeviceInterface *GetInterface() { return this->Interface; }

        ICMPv4(NetworkInterfaceManager::DeviceInterface *Interface);
        ~ICMPv4();
        void Send(/* ???? */);
        void Receive(ICMPPacket *Packet);
    };
}

#endif // !__FENNIX_KERNEL_ICMPv4_H__
