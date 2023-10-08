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

#include <net/icmpv4.hpp>
#include <net/icmpv6.hpp>
#include <debug.h>

#include "../kernel.h"

namespace NetworkICMPv4
{
    ICMPv4::ICMPv4(NetworkInterfaceManager::DeviceInterface *Interface)
    {
        debug("ICMPv4 interface %#lx created.", this);
        this->Interface = Interface;
    }

    ICMPv4::~ICMPv4()
    {
        debug("ICMPv4 interface %#lx destroyed.", this);
    }

    void ICMPv4::Send(/* ???? */)
    {
        fixme("Unimplemented");
    }

    void ICMPv4::Receive(ICMPPacket *Packet)
    {
        if (Packet->Header.Type == ICMPv4Type::TYPE_ECHO)
        {
            // TODO: This probably doesn't work
            netdbg("Echo Request");
            Packet->Header.Type = ICMPv4Type::TYPE_ECHO_REPLY;
            Packet->Header.Code = 0x0;
            Packet->Header.Checksum = CalculateChecksum((uint16_t *)Packet, sizeof(ICMPHeader));
            NIManager->Send(this->Interface, (uint8_t *)Packet, sizeof(ICMPHeader) + 0);
        }
        else
        {
            netdbg("Unknown type %d", Packet->Header.Type);
        }
    }
}

namespace NetworkICMPv6
{
    ICMPv6::ICMPv6(NetworkInterfaceManager::DeviceInterface *Interface) { this->Interface = Interface; }
    ICMPv6::~ICMPv6() {}

    void ICMPv6::Send(uint8_t *Data, size_t Length)
    {
        UNUSED(Data);
        UNUSED(Length);
        fixme("Unimplemented");
    }

    void ICMPv6::Receive(uint8_t *Data)
    {
        UNUSED(Data);
        fixme("Unimplemented");
    }
}
