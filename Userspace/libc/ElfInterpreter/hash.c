#include "ld.h"

uint32_t ElfHash(const unsigned char *Name)
{
	uint32_t i = 0, j;
	while (*Name)
	{
		i = (i << 4) + *Name++;
		if ((j = i & 0xF0000000) != 0)
			i ^= j >> 24;
		i &= ~j;
	}
	return i;
}
