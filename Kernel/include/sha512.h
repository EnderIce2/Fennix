/*
	This file is part of Fennix Kernel.

	Fennix Kernel is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Kernel is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Kernel. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef __FENNIX_KERNEL_SHA512_H__
#define __FENNIX_KERNEL_SHA512_H__

#include <types.h>

#define SHA512_BLOCK_SIZE 128
#define SHA512_DIGEST_SIZE 64

typedef struct
{
	uint64_t state[8];
	uint64_t bitlen;
	uint8_t data[SHA512_BLOCK_SIZE];
	size_t datalen;
} SHA512_CTX;

START_EXTERNC

void sha512_transform(SHA512_CTX *ctx, const uint8_t *data);
void sha512_init(SHA512_CTX *ctx);
void sha512_update(SHA512_CTX *ctx, const uint8_t *data, size_t len);
void sha512_final(SHA512_CTX *ctx, uint8_t *hash);
uint8_t *sha512_sum(const uint8_t *input, size_t len);

END_EXTERNC

#endif // !__FENNIX_KERNEL_SHA512_H__
