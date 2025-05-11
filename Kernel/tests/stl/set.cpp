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
// #include <set>
#include <cassert>
#include <algorithm>
// #include <numeric>

void test_stl_set()
{
	debug("std::set  ...");

// 	auto expect = false;

// 	std::set<int> a{1, 2, 3};
// 	std::set<int> b;
// 	b = a;
// 	expect = b == a;
// 	assert(expect);

// 	auto alloc = a.get_allocator();
// 	int *ptr = alloc.allocate(1);
// 	alloc.deallocate(ptr, 1);

// 	expect = *a.begin() == 1;
// 	assert(expect);

// 	expect = *a.cbegin() == 1;
// 	assert(expect);

// 	expect = *a.rbegin() == 3;
// 	assert(expect);

// 	expect = *a.crbegin() == 3;
// 	assert(expect);

// 	expect = std::next(a.begin(), 3) == a.end();
// 	assert(expect);

// 	expect = std::next(a.cbegin(), 3) == a.cend();
// 	assert(expect);

// 	expect = std::next(a.rbegin(), 3) == a.rend();
// 	assert(expect);

// 	expect = std::next(a.crbegin(), 3) == a.crend();
// 	assert(expect);

// 	assert(!a.empty());
// 	assert(a.size() == 3);
// 	assert(a.max_size() > 3);

// 	std::set<int> c;
// 	c.insert(10);
// 	c.insert({20, 30});
// 	assert(c.size() == 3);

// 	c.emplace(40);
// 	c.emplace_hint(c.begin(), 5);
// 	assert(c.count(5) == 1);
// 	assert(c.count(40) == 1);

// 	auto it = c.find(20);
// 	c.erase(it);
// 	assert(c.count(20) == 0);

// 	c.clear();
// 	assert(c.empty());

// 	std::set<int> d{1, 2, 3};
// 	std::set<int> e{4, 5};
// 	d.swap(e);
// 	assert(d.count(4) == 1 && e.count(1) == 1);

// #if __cpp_lib_node_extract >= 201606L
// 	std::set<int> f{1, 2, 3};
// 	auto node = f.extract(2);
// 	assert(node.value() == 2);
// 	assert(f.count(2) == 0);
// 	std::set<int> g{10, 11};
// 	g.insert(std::move(node));
// 	assert(g.count(2) == 1);

// 	std::set<int> h{1, 2};
// 	std::set<int> i{2, 3};
// 	h.merge(i);
// 	assert(h.count(3) == 1);
// 	assert(i.count(3) == 0);
// #endif

// 	std::set<int> s{10, 20, 30};
// 	assert(s.count(20) == 1);
// 	assert(s.find(20) != s.end());
// 	assert(s.contains(10));
// 	auto range = s.equal_range(20);
// 	assert(range.first == s.find(20) && range.second == std::next(s.find(20)));
// 	assert(*s.lower_bound(15) == 20);
// 	assert(*s.upper_bound(20) == 30);

// 	auto kc = s.key_comp();
// 	assert(kc(10, 20));
// 	auto vc = s.value_comp();
// 	assert(vc(10, 20));

// 	std::set<int> x{1, 2};
// 	std::set<int> y{1, 2};
// 	std::set<int> z{3, 4};
// 	assert(x == y);
// 	assert(x != z);

// 	expect = x < z;
// 	assert(expect);

// 	expect = z > x;
// 	assert(expect);

// 	expect = x <= y;
// 	assert(expect);

// 	expect = z >= y;
// 	assert(expect);

// #if __cpp_impl_three_way_comparison >= 201907L
// 	expect = (x <=> y) == std::strong_ordering::equal;
// 	assert(expect);
// #endif

// 	std::set<int> q{5, 6};
// 	std::set<int> r{7, 8};
// 	std::swap(q, r);
// 	assert(q.count(7) == 1 && r.count(5) == 1);

// #if __cpp_lib_erase_if >= 202002L
// 	std::set<int> m{1, 2, 3, 4};
// 	auto erased = std::erase_if(m, [](int v)
// 								{ return v % 2 == 0; });
// 	assert(erased == 2);
// 	assert(m == std::set<int>({1, 3}));
// #endif

	debug("std::set  OK");
}

#endif // DEBUG
