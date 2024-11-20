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

#include <bitmap.hpp>

bool Bitmap::Get(uint64_t index)
{
	if (index > Size * 8)
		return false;

	uint64_t byteIndex = index / 8;
	uint8_t bitIndex = index % 8;
	uint8_t bitIndexer = 0b10000000 >> bitIndex;

	if ((Buffer[byteIndex] & bitIndexer) > 0)
		return true;

	return false;
}

bool Bitmap::Set(uint64_t index, bool value)
{
	if (index > Size * 8)
		return false;

	uint64_t byteIndex = index / 8;
	uint8_t bitIndex = index % 8;
	uint8_t bitIndexer = 0b10000000 >> bitIndex;

	Buffer[byteIndex] &= ~bitIndexer;
	if (value)
		Buffer[byteIndex] |= bitIndexer;

	return true;
}

bool Bitmap::operator[](uint64_t index) { return this->Get(index); }
