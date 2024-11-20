#ifndef __FENNIX_KERNEL_CRC32_H__
#define __FENNIX_KERNEL_CRC32_H__

#include <types.h>

EXTERNC uint32_t crc32(const uint8_t *Buffer, int Length);

#endif // !__FENNIX_KERNEL_CRC32_H__
