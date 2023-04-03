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
