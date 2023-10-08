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

#include <std.hpp>
#include <assert.h>
#include <vector>

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

	debug("std: OK");
}

#endif // DEBUG
