/*
    This file is part of Fennix Drivers.

    Fennix Drivers is free software: you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    Fennix Drivers is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Fennix Drivers. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef __FENNIX_API_NETWORK_UTILS_H__
#define __FENNIX_API_NETWORK_UTILS_H__

#include <types.h>

typedef __UINT64_TYPE__ uint48_t;

struct MediaAccessControl
{
    uint8_t Address[6];

    inline bool operator==(const MediaAccessControl &lhs) const
    {
        return lhs.Address[0] == this->Address[0] &&
               lhs.Address[1] == this->Address[1] &&
               lhs.Address[2] == this->Address[2] &&
               lhs.Address[3] == this->Address[3] &&
               lhs.Address[4] == this->Address[4] &&
               lhs.Address[5] == this->Address[5];
    }

    inline bool operator==(const uint48_t &lhs) const
    {
        MediaAccessControl MAC;
        MAC.Address[0] = (uint8_t)((lhs >> 40) & 0xFF);
        MAC.Address[1] = (uint8_t)((lhs >> 32) & 0xFF);
        MAC.Address[2] = (uint8_t)((lhs >> 24) & 0xFF);
        MAC.Address[3] = (uint8_t)((lhs >> 16) & 0xFF);
        MAC.Address[4] = (uint8_t)((lhs >> 8) & 0xFF);
        MAC.Address[5] = (uint8_t)(lhs & 0xFF);
        return MAC.Address[0] == this->Address[0] &&
               MAC.Address[1] == this->Address[1] &&
               MAC.Address[2] == this->Address[2] &&
               MAC.Address[3] == this->Address[3] &&
               MAC.Address[4] == this->Address[4] &&
               MAC.Address[5] == this->Address[5];
    }

    inline bool operator!=(const MediaAccessControl &lhs) const { return !(*this == lhs); }
    inline bool operator!=(const uint48_t &lhs) const { return !(*this == lhs); }

    inline uint48_t ToHex()
    {
        return ((uint48_t)this->Address[0] << 40) |
               ((uint48_t)this->Address[1] << 32) |
               ((uint48_t)this->Address[2] << 24) |
               ((uint48_t)this->Address[3] << 16) |
               ((uint48_t)this->Address[4] << 8) |
               ((uint48_t)this->Address[5]);
    }

    inline MediaAccessControl FromHex(uint48_t Hex)
    {
        this->Address[0] = (uint8_t)((Hex >> 40) & 0xFF);
        this->Address[1] = (uint8_t)((Hex >> 32) & 0xFF);
        this->Address[2] = (uint8_t)((Hex >> 24) & 0xFF);
        this->Address[3] = (uint8_t)((Hex >> 16) & 0xFF);
        this->Address[4] = (uint8_t)((Hex >> 8) & 0xFF);
        this->Address[5] = (uint8_t)(Hex & 0xFF);
        return *this;
    }

    inline bool Valid()
    {
        // TODO: More complex MAC validation
        return (this->Address[0] != 0 ||
                this->Address[1] != 0 ||
                this->Address[2] != 0 ||
                this->Address[3] != 0 ||
                this->Address[4] != 0 ||
                this->Address[5] != 0) &&
               (this->Address[0] != 0xFF ||
                this->Address[1] != 0xFF ||
                this->Address[2] != 0xFF ||
                this->Address[3] != 0xFF ||
                this->Address[4] != 0xFF ||
                this->Address[5] != 0xFF);
    }
};

struct InternetProtocol4
{
    uint8_t Address[4];

    inline bool operator==(const InternetProtocol4 &lhs) const
    {
        return lhs.Address[0] == this->Address[0] &&
               lhs.Address[1] == this->Address[1] &&
               lhs.Address[2] == this->Address[2] &&
               lhs.Address[3] == this->Address[3];
    }

    inline bool operator==(const uint32_t &lhs) const
    {
        InternetProtocol4 IP;
        IP.Address[0] = (uint8_t)((lhs >> 24) & 0xFF);
        IP.Address[1] = (uint8_t)((lhs >> 16) & 0xFF);
        IP.Address[2] = (uint8_t)((lhs >> 8) & 0xFF);
        IP.Address[3] = (uint8_t)(lhs & 0xFF);

        return IP.Address[0] == this->Address[0] &&
               IP.Address[1] == this->Address[1] &&
               IP.Address[2] == this->Address[2] &&
               IP.Address[3] == this->Address[3];
    }

    inline bool operator!=(const InternetProtocol4 &lhs) const { return !(*this == lhs); }
    inline bool operator!=(const uint32_t &lhs) const { return !(*this == lhs); }

    inline uint32_t ToHex()
    {
        return ((uint64_t)this->Address[0] << 24) |
               ((uint64_t)this->Address[1] << 16) |
               ((uint64_t)this->Address[2] << 8) |
               ((uint64_t)this->Address[3]);
    }

    inline InternetProtocol4 FromHex(uint32_t Hex)
    {
        this->Address[0] = (uint8_t)((Hex >> 24) & 0xFF);
        this->Address[1] = (uint8_t)((Hex >> 16) & 0xFF);
        this->Address[2] = (uint8_t)((Hex >> 8) & 0xFF);
        this->Address[3] = (uint8_t)(Hex & 0xFF);
        return *this;
    }
};

#endif // !__FENNIX_API_NETWORK_UTILS_H__
