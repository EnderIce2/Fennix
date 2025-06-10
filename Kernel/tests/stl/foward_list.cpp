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

#include <foward_list>
#include <iostream>
#include <cassert>
#include <algorithm>

void test_stl_forward_list()
{
	debug("Running forward_list tests...");

	std::forward_list<int> fl1(3, 7);
	assert(std::distance(fl1.begin(), fl1.end()) == 3);
	for (int v : fl1)
		assert(v == 7);

	fl1.assign({1, 2, 3});
	assert(std::distance(fl1.begin(), fl1.end()) == 3);
	assert(*fl1.begin() == 1);

	auto alloc = fl1.get_allocator();
	int *test = alloc.allocate(1);
	alloc.deallocate(test, 1);

	assert(fl1.front() == 9 || fl1.front() == 1);

	auto before = fl1.before_begin();
	auto begin = fl1.begin();
	auto end = fl1.end();
	assert(std::distance(begin, end) >= 0);

	assert(!fl1.empty());
	assert(fl1.max_size() > 0);

	fl1.insert_after(before, 42);
	fl1.emplace_after(fl1.begin(), 99);

	fl1.erase_after(fl1.begin());

	fl1.emplace_front(88);
	fl1.pop_front();

	fl1.resize(5, -1);
	fl1.resize(2);

	std::forward_list<int> fl2 = {100, 200};
	fl1.swap(fl2);
	assert(*fl1.begin() == 100);

	std::forward_list<int> a = {1, 3, 5};
	std::forward_list<int> b = {2, 4, 6};
	a.merge(b);
	assert(std::is_sorted(a.begin(), a.end()));

	std::forward_list<int> src = {9, 8};
	a.splice_after(a.before_begin(), src);
	assert(*a.begin() == 9);

	a.remove(3);
	a.remove_if([](int x)
				{ return x == 5; });

	a.reverse();

	a.push_front(2);
	a.push_front(2);
	a.unique();

	a.sort();
	assert(std::is_sorted(a.begin(), a.end()));

	debug("All forward_list tests passed.");
}

#endif
