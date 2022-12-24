#ifndef __FENNIX_KERNEL_NETWORK_H__
#define __FENNIX_KERNEL_NETWORK_H__

#include <types.h>

// #define DEBUG_NETWORK 1

#ifdef DEBUG_NETWORK
#define netdbg(m, ...) debug(m, ##__VA_ARGS__)
void DbgNetwork();
void DbgDumpData(const char *Description, void *Address, unsigned long Length);
#else
#define netdbg(m, ...)
static inline void DbgNetwork() { return; }
static inline void DbgDumpData(const char *Description, void *Address, unsigned long Length) { return; }
#endif

// TODO: How i would do this?
typedef __UINT64_TYPE__ uint48_t;

#define b4(x) ((x & 0x0F) << 4 | (x & 0xF0) >> 4)
#define b8(x) ((x)&0xFF)
#define b16(x) __builtin_bswap16(x)
#define b32(x) __builtin_bswap32(x)
#define b48(x) (((((x)&0x0000000000ff) << 40) | (((x)&0x00000000ff00) << 24) | (((x)&0x000000ff0000) << 8) | (((x)&0x0000ff000000) >> 8) | (((x)&0x00ff00000000) >> 24) | (((x)&0xff0000000000) >> 40)))
#define b64(x) __builtin_bswap64(x)

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

uint16_t CalculateChecksum(uint16_t *Data, uint64_t Length);

#endif // !__FENNIX_KERNEL_NETWORK_H__
