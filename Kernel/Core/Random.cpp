#include <rand.hpp>

namespace Random
{
    static uint64_t Seed = 0xdeadbeef;

    uint16_t rand16()
    {
        Seed = Seed * 1103515245 + 12345;
        return (uint16_t)(Seed / 65536) % __UINT16_MAX__;
    }

    uint32_t rand32()
    {
        Seed = Seed * 1103515245 + 12345;
        return (uint32_t)(Seed / 65536) % __UINT32_MAX__;
    }

    uint64_t rand64()
    {
        Seed = Seed * 1103515245 + 12345;
        return (uint64_t)(Seed / 65536) % __UINT64_MAX__;
    }

    void changeseed(uint64_t CustomSeed) { Seed = CustomSeed; }

}