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
