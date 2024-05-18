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
#include <vector>

void __stl_vector_constructor_destructor_operator_equal_assign()
{
	std::vector<int> v1;
	assert(v1.empty());
	assert(v1.size() == 0);
	assert(v1.capacity() == 0);

	std::vector<int> v2(10);
	assert(!v2.empty());
	assert(v2.size() == 10);
	assert(v2.capacity() == 10);

	std::vector<int> v3(10, 5);
	assert(!v3.empty());
	assert(v3.size() == 10);
	assert(v3.capacity() == 10);

	std::vector<int> v4(v3);
	assert(!v4.empty());
	assert(v4.size() == 10);
	assert(v4.capacity() == 10);

	std::vector<int> v5 = v4;
	assert(!v5.empty());
	assert(v5.size() == 10);
	assert(v5.capacity() == 10);

	std::vector<int> v6;
	v6 = v5;
	assert(!v6.empty());
	assert(v6.size() == 10);
	assert(v6.capacity() == 10);

	std::vector<int> v7;
	v7.assign(v6.begin(), v6.end());
	assert(!v7.empty());
	assert(v7.size() == 10);
	assert(v7.capacity() == 10);
}

void __stl_vector_at_operator_array_front_back_data()
{
	std::vector<int> v1(10, 5);
	assert(v1.at(0) == 5);
	assert(v1.at(9) == 5);
	assert(v1[0] == 5);
	assert(v1[9] == 5);
	assert(v1.front() == 5);
	assert(v1.back() == 5);
	assert(v1.data() != nullptr);
}

void __stl_vector_begin_cbegin_end_cend()
{
	std::vector<int> v1(10, 5);
	assert(*v1.begin() == 5);
	assert(*v1.cbegin() == 5);
	assert(*v1.end() == 0);
	assert(*v1.cend() == 0);
}

void __stl_vector_clear_insert_emplace_erase_push_back_emplace_back_pop_back_resize_swap()
{
	std::vector<int> v1(10, 5);
	v1.clear();
	assert(v1.empty());
	assert(v1.size() == 0);
	assert(v1.capacity() == 10);

	v1.insert(v1.begin(), 5);
	assert(v1.size() == 1);
	assert(v1[0] == 5);

	v1.emplace(v1.begin(), 10);
	assert(v1.size() == 2);
	assert(v1[0] == 10);
	assert(v1[1] == 5);

	v1.erase(v1.begin());
	assert(v1.size() == 1);
	assert(v1[0] == 5);

	v1.push_back(10);
	assert(v1.size() == 2);
	assert(v1[1] == 10);

	v1.emplace_back(15);
	assert(v1.size() == 3);
	assert(v1[2] == 15);

	v1.pop_back();
	assert(v1.size() == 2);
	assert(v1[1] == 10);

	v1.resize(5);
	assert(v1.size() == 5);
	assert(v1[1] == 10);

	std::vector<int> v2(10, 5);
	v1.swap(v2);
	assert(v1.size() == 10);
	assert(v2.size() == 5);
}

void __stl_vector_reverse_iterators()
{
	std::vector<int> v1 = {1, 2, 3, 4, 5};

	// Test rbegin and rend
	std::vector<int>::reverse_iterator rit;
	std::vector<int>::const_reverse_iterator crit;

	int i = 0;
	for (rit = v1.rbegin(); rit != v1.rend(); ++rit)
	{
		assert(*rit == 5 - i);
		i++;
	}

	int j = 0;
	for (crit = v1.crbegin(); crit != v1.crend(); ++crit)
	{
		assert(*crit == 5 - j);
		j++;
	}
}

void test_stl_vector()
{
	debug("std::vector  ...");

	debug("std::vector  constructor, destructor, operator=, assign");
	__stl_vector_constructor_destructor_operator_equal_assign();

	debug("std::vector  at, operator[], front, back, data");
	__stl_vector_at_operator_array_front_back_data();

	debug("std::vector  begin, cbegin, end, cend");
	__stl_vector_begin_cbegin_end_cend();

	debug("std::vector  clear, insert, emplace, erase, push_back, emplace_back, pop_back, resize, swap");
	__stl_vector_clear_insert_emplace_erase_push_back_emplace_back_pop_back_resize_swap();

	debug("std::vector  OK");
}

#endif // DEBUG
