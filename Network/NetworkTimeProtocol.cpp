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

#include <net/ntp.hpp>
#include <debug.h>

#include "../kernel.h"

namespace NetworkNTP
{
    void NTP::OnUDPPacketReceived(NetworkUDP::Socket *Socket, uint8_t *Data, size_t Length)
    {
        UNUSED(Socket);
        UNUSED(Length);
        this->NTPPacket = *(NTPHeader *)Data;
        this->TimeReceived = true;
        netdbg("Received UDP packet for NTP.");
    }

    NTP::NTP(NetworkUDP::Socket *Socket) : NetworkUDP::UDPEvents()
    {
        debug("NTP interface %#lx created.", this);
        this->UDPSocket = Socket;
    }

    NTP::~NTP()
    {
        debug("NTP interface %#lx destroyed.", this);
    }

    int NTP::ReadTime()
    {
        netdbg("Requesting time");
        this->TimeReceived = false;

        NTPHeader ReqPacket;                      // This may require physical memory allocation but Ethernet already has this job.
        memset(&ReqPacket, 0, sizeof(NTPHeader)); // Zero out the packet
        *((char *)&ReqPacket) = 0x1b;             // byteswap nightmare, this is the code below but in little endian
        // ReqPacket.LeapIndicator = 0;
        // ReqPacket.VersionNumber = 3;
        // ReqPacket.Mode = 3;
        UDPSocket->SocketUDP->Send(UDPSocket, (uint8_t *)&ReqPacket, sizeof(NTPHeader));

        debug("Waiting for response...");
        int RequestTimeout = 20;
        while (!this->TimeReceived)
        {
            if (--RequestTimeout == 0)
            {
                warn("Request timeout.");
                return 0;
            }
            netdbg("Still waiting...");
            TaskManager->Sleep(1000);
        }

        uint64_t UnixTimestamp = (b32(this->NTPPacket.TransmitTimestamp[0])) - 2208988800;
        debug("Unix time: %d", UnixTimestamp);
        return s_cst(int, UnixTimestamp);
    }
}
