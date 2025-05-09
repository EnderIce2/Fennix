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

#include <functional>
#include <assert.h>
#include <memory>

struct Base
{
	virtual ~Base() = default;
	virtual int id() const { return 1; }
};
struct Derived : Base
{
	int id() const override { return 2; }
};

struct Deleter
{
	bool *flag;
	Deleter(bool *f) : flag(f) {}
	void operator()(int *p) const
	{
		*flag = true;
		delete p;
	}
};

void test_shared_ptr_bitset()
{
	debug("std::shared_ptr  ...");

	/* 1. constructor default */

	std::shared_ptr<int> p1;
	assert(!p1);
	assert(p1.use_count() == 0);

	/* 2. constructor from raw ptr, get(), *, use_count(), operator bool */

	auto p2 = std::shared_ptr<int>(new int(5));
	assert(p2);
	assert(p2.use_count() == 1);
	assert(*p2 == 5);
	assert(p2.get() && *p2 == 5);

	/* 3. copy/move ctor */

	auto p3 = p2;
	assert(p2.use_count() == 2 && p3.use_count() == 2);
	auto p4 = std::move(p3);
	assert(p4.use_count() == 2 && p3.use_count() == 0);

	/* 4. copy/move assign */

	std::shared_ptr<int> p5;
	p5 = p4;
	assert(p5.use_count() == 3);
	std::shared_ptr<int> p6;
	p6 = std::move(p5);
	assert(p6.use_count() == 3 && p5.use_count() == 0);

	/* 5. reset() */

	p6.reset();
	assert(!p6 && p6.use_count() == 0);
	p4.reset(new int(10));
	assert(*p4 == 10);

	/* 6. swap() member and std::swap */

	p4.swap(p2);
	assert(*p2 == 10 && *p4 == 5);
	std::swap(p2, p4);
	assert(*p4 == 10 && *p2 == 5);

	/* 7. observers: ->, *, get() already checked */

	struct S
	{
		int x;
	};
	auto ps = std::make_shared<S>();
	ps->x = 7;
	assert((*ps).x == 7 && ps.get()->x == 7);

	/* 8. use_count(), operator bool already checked */

	/* 9. owner_before() */

	auto pa = std::make_shared<int>(1);
	auto pb = std::make_shared<int>(1);
	assert(pa.owner_before(pb) || pb.owner_before(pa));

	/* 10. make_shared, allocate_shared */

	auto pm = std::make_shared<int>(42);
	assert(pm && *pm == 42);
	std::allocator<int> allo;
	auto pa2 = std::allocate_shared<int>(allo, 99);
	assert(*pa2 == 99);

	/* 11. static_pointer_cast, dynamic_pointer_cast, const_pointer_cast, reinterpret_pointer_cast */

	debug("skipping static_pointer_cast, dynamic_pointer_cast, const_pointer_cast, reinterpret_pointer_cast");

	// auto pd = std::make_shared<Derived>();
	// std::shared_ptr<Base> pb2 = std::static_pointer_cast<Base>(pd);
	// assert(pb2->id() == 2);

	// std::shared_ptr<Base> base = std::make_shared<Derived>();
	// auto dyn = std::dynamic_pointer_cast<Derived>(base);
	// assert(dyn && dyn->id() == 2);
	// std::shared_ptr<Base> base2 = std::make_shared<Base>();
	// auto dyn2 = std::dynamic_pointer_cast<Derived>(base2);
	// assert(!dyn2);

	// auto pc = std::make_shared<const int>(123);
	// auto pnc = std::const_pointer_cast<int>(pc);
	// assert(*pnc == 123);
	// *pnc = 124;
	// assert(*pnc == 124);

	// auto pint = std::make_shared<int>(65);
	// auto pchar = std::reinterpret_pointer_cast<char>(pint);
	// assert(pchar.get() == reinterpret_cast<char *>(pint.get()));

	/* 12. comparison operators ==, !=, <, <=, >, >= */

	debug("skipping comparison operators ==, !=, <, <=, >, >=");

	// auto c1 = std::make_shared<int>(1);
	// auto c2 = std::make_shared<int>(1);
	// auto c3 = c1;
	// assert(c1 == c3);
	// assert(c1 != c2);
	// assert((c2 < c1) == (c2.get() < c1.get()));
	// assert(c1 <= c3);
	// assert(c1 >= c3);

	/* 13. operator<< */

	debug("skipping operator<<");

	// std::ostringstream oss;
	// std::shared_ptr<int> pnull;
	// oss << pnull;
	// assert(oss.str() == \"0\");

	/* 14. get_deleter */

	debug("skipping get_deleter");

	// bool flag = false;
	// std::shared_ptr<int> pd1(new int(7), Deleter(&flag));
	// auto d = std::get_deleter<Deleter>(pd1);
	// assert(d && d->flag == &flag);
	// pd1.reset();
	// assert(flag);

	/* 15. std::hash<shared_ptr> */

	debug("skipping std::hash<shared_ptr>");

	// std::hash<std::shared_ptr<int>> h;
	// assert(h(c1) == std::hash<int *>()(c1.get()));

	debug("std::shared_ptr  OK");
}

#endif // DEBUG
