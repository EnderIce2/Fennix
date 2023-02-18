#include <net/dns.hpp>
#include <debug.h>

#include "../kernel.h"

namespace NetworkDNS
{
    DNS::DNS(NetworkUDP::Socket *Socket) : NetworkUDP::UDPEvents()
    {
        debug("DNS interface %#lx created.", this);
        this->UDPSocket = Socket;
    }

    DNS::~DNS()
    {
        debug("DNS interface %#lx destroyed.", this);
    }
}
