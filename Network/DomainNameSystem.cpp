#include <net/dns.hpp>
#include <debug.h>

#include "../kernel.h"

namespace NetworkDNS
{
    DNS::DNS(NetworkUDP::Socket *Socket) : NetworkUDP::UDPEvents()
    {
        this->UDPSocket = Socket;
    }

    DNS::~DNS()
    {
    }
}
