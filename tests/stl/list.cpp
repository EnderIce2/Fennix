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
#include <iterator>
#include <list>

void __stl_list_front_back_push_pop_begin_end()
{
	std::list<int> l = {7, 5, 16, 8};
	l.push_front(25);
	l.push_back(13);

	auto it = std::find(l.begin(), l.end(), 16);
	if (it != l.end())
		l.insert(it, 42);

	// test if l = { 25, 7, 5, 42, 16, 8, 13, };
	assert(l.front() == 25);
	assert(l.back() == 13);

	l.pop_front();
	l.pop_back();

	// test if l = { 7, 5, 42, 16, 8, };
	assert(l.front() == 7);
	assert(l.back() == 8);

	l.pop_front();
	l.pop_back();

	// test if l = { 5, 42, 16, };
	assert(l.front() == 5);
	assert(l.back() == 16);

	l.pop_front();
	l.pop_back();

	// test if l = { 42, };
	assert(l.front() == 42);
	assert(l.back() == 42);
}

void __stl_list_assign()
{
	std::list<char> characters;

	characters.assign(5, 'a');
	assert(characters.size() == 5);

	for (auto it = characters.begin(); it != characters.end(); ++it)
		assert(*it == 'a');
}

void __stl_list_clear_insert_emplace_erase_resize_swap()
{
	std::list<int> l = {1, 2, 3, 4, 5};

	l.clear();
	assert(l.empty());

	l.insert(l.begin(), 42);
	assert(l.size() == 1);
	assert(l.front() == 42);

	l.emplace(l.begin(), 13);
	assert(l.size() == 2);
	assert(l.front() == 13);

	l.erase(l.begin());
	assert(l.size() == 1);
	assert(l.front() == 42);

	l.resize(5, 100);
	assert(l.size() == 5);
	assert(l.back() == 100);

	std::list<int> l2 = {10, 20, 30};
	l.swap(l2);
	assert(l.size() == 3);
	assert(l2.size() == 5);
}

void __stl_list_merge_splice_remove_remove_if_reverse_unique_sort()
{
	{
		std::list<int> l1 = {1, 3, 5};
		std::list<int> l2 = {2, 4, 6};

		l1.merge(l2);
		assert(l1.size() == 6);
		assert(l2.empty());
	}

	{
		std::list<int> l3 = {1, 2, 3, 4, 5};
		std::list<int> l4 = {10, 20, 30};

		auto it = l3.begin();
		std::advance(it, 3);
		l3.splice(it, l4);
		assert(l3.size() == 8);
		assert(l4.empty());

		l3.remove(3);
		assert(l3.size() == 7);

		l3.remove_if([](int n)
					 { return n < 5; });
		assert(l3.size() == 4);
	}

	{
		std::list<int> l5 = {1, 2, 3, 4, 5};
		l5.reverse();
		assert(l5.front() == 5);
		assert(l5.back() == 1);

		l5.push_back(1);
		l5.remove_if([](int n)
					 { return n == 3; });

		l5.unique();
		assert(l5.size() == 4);

		l5.sort();
		assert(l5.front() == 1);
		assert(l5.back() == 5);
	}
}

void test_stl_list()
{
	debug("std::list  ...");
	debug("std::list  front, back, push_front, push_back, pop_front, pop_back, begin, end");
	__stl_list_front_back_push_pop_begin_end();

	debug("std::list  assign");
	__stl_list_assign();

	debug("std::list  clear, insert, emplace, erase, resize, swap");
	__stl_list_clear_insert_emplace_erase_resize_swap();

	debug("std::list  merge, splice, remove, remove_if, reverse, unique, sort");
	__stl_list_merge_splice_remove_remove_if_reverse_unique_sort();

	debug("std::list  OK");
}

#endif // DEBUG
