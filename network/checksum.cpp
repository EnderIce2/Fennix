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

#include <net/net.hpp>

uint16_t CalculateChecksum(uint16_t *Data, size_t Length)
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
