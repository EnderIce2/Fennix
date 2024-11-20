#include <net/net.hpp>

uint16_t CalculateChecksum(uint16_t *Data, uint64_t Length)
{
    uint16_t *Data16 = (uint16_t *)Data;
    uint64_t Checksum = 0;
    for (uint64_t i = 0; i < Length / 2; i++)
        Checksum += ((Data16[i] & 0xFF00) >> 8) | ((Data16[i] & 0x00FF) << 8);
    if (Length % 2)
        Checksum += ((uint16_t)((char *)Data16)[Length - 1]) << 8;
    while (Checksum & 0xFFFF0000)
        Checksum = (Checksum & 0xFFFF) + (Checksum >> 16);
    return (uint16_t)(((~Checksum & 0xFF00) >> 8) | ((~Checksum & 0x00FF) << 8));
}
