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

#ifndef __FENNIX_KERNEL_NETWORK_ETHERNET_H__
#define __FENNIX_KERNEL_NETWORK_ETHERNET_H__

#include <types.h>
#include <net/nc.hpp>

namespace NetworkEthernet
{
	enum FrameType
	{
		TYPE_IPV4 = 0x0800,
		TYPE_ARP = 0x0806,
		TYPE_RARP = 0x8035,
		TYPE_IPV6 = 0x86DD
	};

	struct EthernetHeader
	{
		uint48_t DestinationMAC : 48;
		uint48_t SourceMAC : 48;
		uint16_t Type;
	} __packed;

	struct EthernetPacket
	{
		EthernetHeader Header;
		uint8_t Data[];
	};

	class EthernetEvents
	{
	private:
		FrameType FType;

	protected:
		EthernetEvents(FrameType Type);
		~EthernetEvents();

	public:
		FrameType GetFrameType() { return FType; }
		virtual void OnEthernetPacketSent(EthernetPacket *Packet)
		{
			UNUSED(Packet);
			netdbg("Event not handled. [%p]", Packet);
		}

		virtual bool OnEthernetPacketReceived(uint8_t *Data, size_t Length)
		{
			UNUSED(Data);
			UNUSED(Length);
			netdbg("Event not handled. [%p, %d]", Data, Length);
			return false;
		}
	};

	class Ethernet : public NetworkInterfaceManager::Events
	{
	private:
		NetworkInterfaceManager::DeviceInterface *Interface;
		void Receive(uint8_t *Data, size_t Length);
		void OnInterfaceReceived(NetworkInterfaceManager::DeviceInterface *Interface, uint8_t *Data, size_t Length);

	public:
		/** @brief Get driver interface
		 * @return Driver interface
		 */
		NetworkInterfaceManager::DeviceInterface *GetInterface()
		{
			netdbg("Interface: %#lx (MAC: %s; IPv4: %s; IPv6: %s)", this->Interface,
				   this->Interface->MAC.ToString(),
				   this->Interface->IP.v4.ToStringLittleEndian(),
				   this->Interface->IP.v6.ToStringLittleEndian());
			return this->Interface;
		}

		Ethernet(NetworkInterfaceManager::DeviceInterface *Interface);
		~Ethernet();

		/**
		 * @brief Send an Ethernet packet.
		 *
		 * @param MAC The MAC address of the destination. (Big-endian)
		 * @param Type The type of the packet.
		 * @param Data The data to send.
		 * @param Length The length of the data.
		 */
		void Send(MediaAccessControl MAC, FrameType Type, uint8_t *Data, size_t Length);
	};
}

#endif // !__FENNIX_KERNEL_NETWORK_ETHERNET_H__
