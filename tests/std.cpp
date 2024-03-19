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
#include <unordered_map>
#include <vector>
#include <atomic>
#include <bitset>
#include <list>

void Test_std()
{
	std::atomic_int a = 0;
	a++;
	assert(a == 1);

	int b = a.exchange(2);
	assert(b == 1);
	assert(a == 2);

	/* ---------------------------- */

	std::vector<int> intVector;
	intVector.push_back(10);
	intVector.push_back(20);
	intVector.push_back(30);
	assert(intVector.size() == 3);

	assert(intVector[0] == 10);
	assert(intVector[1] == 20);
	assert(intVector[2] == 30);

	intVector.pop_back();
	assert(intVector.size() == 2);

	intVector.clear();
	assert(intVector.empty());

	intVector.push_back(1);
	intVector.push_back(1);
	intVector.push_back(1);
	intVector.push_back(1);
	intVector.push_back(1);

	intVector.erase(intVector.end() - 1);
	assert(intVector.size() == 4);
	debug("0: %#lx", intVector[0]);
	debug("1: %#lx", intVector[1]);
	debug("2: %#lx", intVector[2]);
	debug("3: %#lx", intVector[3]);
	debug("4: %#lx", intVector[4]);

	/* ---------------------------- */

	std::list<int> intList;
	intList.push_back(10);
	intList.push_back(20);
	intList.push_back(30);
	assert(intList.size() == 3);

	assert(intList.front() == 10);
	assert(intList.back() == 30);

	intList.pop_back();
	assert(intList.size() == 2);

	intList.clear();
	assert(intList.empty());

	/* ---------------------------- */

	std::unordered_map<int, int> intMap;
	intMap[1] = 10;
	intMap[2] = 20;
	intMap[3] = 30;
	assert(intMap.size() == 3);

	assert(intMap[1] == 10);
	assert(intMap[2] == 20);
	assert(intMap[3] == 30);

	intMap.erase(1);
	assert(intMap.size() == 2);

	intMap.clear();
	assert(intMap.empty());

	std::unordered_map<const char *, int> strMap;
	strMap["hello"] = 10;
	strMap["world"] = 20;
	strMap["foo"] = 30;
	assert(strMap.size() == 3);

	assert(strMap["hello"] == 10);
	assert(strMap["world"] == 20);
	assert(strMap["foo"] == 30);

	strMap.erase("hello");
	assert(strMap.size() == 2);

	strMap.clear();
	assert(strMap.empty());

	/* ---------------------------- */

	std::hash<int> intHash;

	size_t a0 = intHash(0xdeadbeef);
	size_t a1 = intHash(0xdeadbeef);
	size_t a2 = intHash(1);
	size_t a3 = intHash(2);

	debug("a0: %#lx", a0);
	debug("a1: %#lx", a1);
	debug("a2: %#lx", a2);
	debug("a3: %#lx", a3);

	assert(a0 == a1);
	assert(a2 != a3);

	/* ---------------------------- */

	/* https://en.cppreference.com/w/cpp/utility/bitset */

	typedef std::size_t length_t, position_t;
	constexpr std::bitset<4> bs1;
	constexpr std::bitset<4> bs2{0xA};
	std::bitset<4> bs3{"0011"};
	std::bitset<8> bs4{"ABBA", length_t(4), 'A', 'B'};

	debug("bs1: %s; bs2: %s; bs3: %s; bs4: %s",
		  bs1.to_string().c_str(), bs2.to_string().c_str(),
		  bs3.to_string().c_str(), bs4.to_string().c_str());

	assert(bs1 == 0b0000);
	assert(bs2 == 0b1010);
	assert(bs3 == 0b0011);
	assert(bs4 == 0b00000110);

	bs3 |= 0b0100;
	assert(bs3 == 0b0111);
	bs3 &= 0b0011;
	assert(bs3 == 0b0011);
	bs3 ^= std::bitset<4>{0b1100};
	assert(bs3 == 0b1111);

	bs3.reset();
	assert(bs3 == 0);
	bs3.set();
	assert(bs3 == 0b1111);
	bool all = bs3.all();
	bool any = bs3.any();
	bool none = bs3.none();
	debug("all: %d; any: %d; none: %d", all, any, none);
	assert(all && any && !none);
	bs3.flip();
	assert(bs3 == 0);

	bs3.set(position_t(1), true);
	assert(bs3 == 0b0010);
	bs3.set(position_t(1), false);
	assert(bs3 == 0);
	bs3.flip(position_t(2));
	assert(bs3 == 0b0100);
	bs3.reset(position_t(2));
	assert(bs3 == 0);

	bs3[2] = true;
	assert(true == bs3[2]);

	assert(bs3.count() == 1);
	assert(bs3.size() == 4);
	assert(bs3.to_ullong() == 0b0100ULL);
	assert(bs3.to_string() == "0100");

	/* ---------------------------- */

	debug("std: OK");
}

#endif // DEBUG
