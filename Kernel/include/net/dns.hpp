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

#ifndef __FENNIX_KERNEL_DNS_H__
#define __FENNIX_KERNEL_DNS_H__

#include <net/udp.hpp>
#include <types.h>

namespace NetworkDNS
{
	class DNS : public NetworkUDP::UDPEvents
	{
	private:
		NetworkUDP::Socket *UDPSocket;

	public:
		DNS(NetworkUDP::Socket *Socket);
		~DNS();
	};
}

#endif // !__FENNIX_KERNEL_DNS_H__
