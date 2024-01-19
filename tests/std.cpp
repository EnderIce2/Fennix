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

	debug("std: OK");
}

#endif // DEBUG
