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

#include <net/eth.hpp>
#include <debug.h>

#include "../kernel.h"

/* conversion from 'uint48_t' {aka 'long unsigned int'} to 'long unsigned int:48' may change value */
#pragma GCC diagnostic ignored "-Wconversion"

namespace NetworkEthernet
{
	struct EthernetEventHelperStruct
	{
		EthernetEvents *Ptr;
		uint16_t Type;
	};

	std::vector<EthernetEventHelperStruct> RegisteredEvents;

	Ethernet::Ethernet(NetworkInterfaceManager::DeviceInterface *Interface) : NetworkInterfaceManager::Events(Interface)
	{
		debug("Ethernet interface %#lx created.", this);
		this->Interface = Interface;
	}

	Ethernet::~Ethernet()
	{
		debug("Ethernet interface %#lx destroyed.", this);
	}

	void Ethernet::Send(MediaAccessControl MAC, FrameType Type, uint8_t *Data, size_t Length)
	{
		netdbg("Sending frame type %#x to %s", Type, MAC.ToString());
		size_t PacketLength = sizeof(EthernetHeader) + Length;
		EthernetPacket *Packet = (EthernetPacket *)kmalloc(PacketLength);

		Packet->Header.DestinationMAC = b48(MAC.ToHex());
		Packet->Header.SourceMAC = b48(this->Interface->MAC.ToHex());
		Packet->Header.Type = b16(Type);

		memcpy(Packet->Data, Data, Length);
		/* Network Interface Manager takes care of physical allocation.
		   So basically, we allocate here and then it allocates again but 1:1 mapped. */
		NIManager->Send(Interface, (uint8_t *)Packet, PacketLength);
		kfree(Packet);
	}

	void Ethernet::Receive(uint8_t *Data, size_t Length)
	{
		EthernetPacket *Packet = (EthernetPacket *)Data;
		size_t PacketLength = Length - sizeof(EthernetHeader);
		/* TODO: I have to do checks here to be sure that PacketLength is right */
		UNUSED(PacketLength);

		MediaAccessControl SourceMAC;
		SourceMAC.FromHex(b48(Packet->Header.SourceMAC));
		MediaAccessControl DestinationMAC;
		DestinationMAC.FromHex(b48(Packet->Header.DestinationMAC));

		netdbg("Received frame type %#x from %s to %s", b16(Packet->Header.Type), SourceMAC.ToString(), DestinationMAC.ToString());

		/*                 Byte-swapped               little-endian                   */
		if (b48(Packet->Header.DestinationMAC) == 0xFFFFFFFFFFFF ||
			/*                 Byte-swapped           Driver interface has little-endian order */
			b48(Packet->Header.DestinationMAC) == this->Interface->MAC.ToHex())
		/* This is true only if the packet is for us (Interface MAC or broadcast) */
		{
			netdbg("Received data from %s [Type %#x]", SourceMAC.ToString(), b16(Packet->Header.Type));
			bool Reply = false;

			switch (b16(Packet->Header.Type))
			{
			case TYPE_IPV4:
				foreach (auto e in RegisteredEvents)
					if (e.Type == TYPE_IPV4)
						Reply = e.Ptr->OnEthernetPacketReceived((uint8_t *)Packet->Data, Length);
				break;
			case TYPE_ARP:
				foreach (auto e in RegisteredEvents)
					if (e.Type == TYPE_ARP)
						Reply = e.Ptr->OnEthernetPacketReceived((uint8_t *)Packet->Data, Length);
				break;
			case TYPE_RARP:
				foreach (auto e in RegisteredEvents)
					if (e.Type == TYPE_RARP)
						Reply = e.Ptr->OnEthernetPacketReceived((uint8_t *)Packet->Data, Length);
				break;
			case TYPE_IPV6:
				foreach (auto e in RegisteredEvents)
					if (e.Type == TYPE_IPV6)
						Reply = e.Ptr->OnEthernetPacketReceived((uint8_t *)Packet->Data, Length);
				break;
			default:
				warn("Unknown packet type %#lx", Packet->Header.Type);
				break;
			}
			if (Reply)
			{
				/* FIXME: I should reply, right? I have to do more research here... */
				// Packet->Header.DestinationMAC = Packet->Header.SourceMAC;
				// Packet->Header.SourceMAC = b48(this->Interface->MAC.ToHex());
				fixme("Replying to %s [%s]=>[%s]", SourceMAC.ToString(),
					  this->Interface->MAC.ToString(), DestinationMAC.ToString());
			}
		}
		else
		{
			netdbg("Packet not for us [%s]=>[%s]", this->Interface->MAC.ToString(), DestinationMAC.ToString());
		}
	}

	void Ethernet::OnInterfaceReceived(NetworkInterfaceManager::DeviceInterface *Interface, uint8_t *Data, size_t Length)
	{
		if (Interface == this->Interface)
			this->Receive(Data, Length);
	}

	EthernetEvents::EthernetEvents(FrameType Type)
	{
		this->FType = Type;
		RegisteredEvents.push_back({.Ptr = this, .Type = (uint16_t)Type});
	}

	EthernetEvents::~EthernetEvents()
	{
		forItr(itr, RegisteredEvents)
		{
			if (itr->Ptr == this)
			{
				RegisteredEvents.erase(itr);
				return;
			}
		}
	}
}
