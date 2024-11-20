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

#ifdef DEBUG

#include <assert.h>
#include <bitset>

void __stl_bitset_test()
{
	/* Code from: https://en.cppreference.com/w/cpp/utility/bitset */

	typedef std::size_t length_t, position_t; // the hints

	// constructors:
	constexpr std::bitset<4> b1;
	constexpr std::bitset<4> b2{0xA};								// == 0B1010
	std::bitset<4> b3{"0011"};										// can also be constexpr since C++23
	std::bitset<8> b4{"ABBA", length_t(4), /*0:*/ 'A', /*1:*/ 'B'}; // == 0B0000'0110

	// bitsets can be printed out to a stream:
	// std::cout << "b1:" << b1 << "; b2:" << b2 << "; b3:" << b3 << "; b4:" << b4 << '\n';
	debug("b1:%s; b2:%s; b3:%s; b4:%s", b1.to_string().c_str(), b2.to_string().c_str(), b3.to_string().c_str(), b4.to_string().c_str());
	/* Expected output: b1:0000; b2:1010; b3:0011; b4:00000110 */

	// bitset supports bitwise operations:
	b3 |= 0b0100;
	assert(b3 == 0b0111);
	b3 &= 0b0011;
	assert(b3 == 0b0011);
	b3 ^= std::bitset<4>{0b1100};
	assert(b3 == 0b1111);

	// operations on the whole set:
	b3.reset();
	assert(b3 == 0);
	b3.set();
	assert(b3 == 0b1111);
	assert(b3.all() && b3.any() && !b3.none());
	b3.flip();
	assert(b3 == 0);

	// operations on individual bits:
	b3.set(position_t(1), true);
	assert(b3 == 0b0010);
	b3.set(position_t(1), false);
	assert(b3 == 0);
	b3.flip(position_t(2));
	assert(b3 == 0b0100);
	b3.reset(position_t(2));
	assert(b3 == 0);

	// subscript operator[] is supported:
	b3[2] = true;
	assert(true == b3[2]);

	// other operations:
	assert(b3.count() == 1);
	assert(b3.size() == 4);
	assert(b3.to_ullong() == 0b0100ULL);
	assert(b3.to_string() == "0100");
}

void test_stl_bitset()
{
	debug("std::bitset  ...");

	__stl_bitset_test();

	debug("std::bitset  OK");
}

#endif // DEBUG
