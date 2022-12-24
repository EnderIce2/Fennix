#include <net/ntp.hpp>
#include <debug.h>

#include "../kernel.h"

namespace NetworkNTP
{
    NTP::NTP(NetworkUDP::Socket *Socket) : NetworkUDP::UDPEvents()
    {
        this->Socket = Socket;
    }

    NTP::~NTP()
    {
    }

    void ReadTime()
    {
        fixme("ReadTime()");
    }
}
