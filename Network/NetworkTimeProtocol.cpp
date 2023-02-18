#include <net/ntp.hpp>
#include <debug.h>

#include "../kernel.h"

namespace NetworkNTP
{
    void NTP::OnUDPPacketReceived(NetworkUDP::Socket *Socket, uint8_t *Data, uint64_t Length)
    {
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

        NTPHeader ReqPacket;                      // This may require phyiscal memory allocation but Ethernet already has this job.
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

        int UnixTimestamp = b32(this->NTPPacket.TransmitTimestamp[0]) - 2208988800;
        debug("Unix time: %d", UnixTimestamp);
        return UnixTimestamp;
    }
}
