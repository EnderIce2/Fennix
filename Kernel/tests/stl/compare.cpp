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
#include <compare>
#include <string>
#include <vector>

void test_stl_compare()
{
	debug("std::compare  ...");

	assert(std::strong_ordering::equal == (5 <=> 5));
	assert(std::strong_ordering::less == (3 <=> 5));
	assert(std::strong_ordering::greater == (7 <=> 5));

	assert(std::weak_ordering::equivalent == (5.0 <=> 5.0));
	assert(std::weak_ordering::less == (3.0 <=> 5.0));
	assert(std::weak_ordering::greater == (7.0 <=> 5.0));

	assert(std::partial_ordering::equivalent == (5.0 <=> 5.0));
	assert(std::partial_ordering::less == (3.0 <=> 5.0));
	assert(std::partial_ordering::greater == (7.0 <=> 5.0));
	assert(std::partial_ordering::unordered == (std::numeric_limits<double>::quiet_NaN() <=> 5.0));

	assert(std::strong_ordering::equal == (std::string("abc") <=> std::string("abc")));
	assert(std::strong_ordering::less == (std::string("abc") <=> std::string("bcd")));
	assert(std::strong_ordering::greater == (std::string("bcd") <=> std::string("abc")));

	assert(std::strong_ordering::equal == (std::vector<int>{1, 2, 3} <=> std::vector<int>{1, 2, 3}));
	assert(std::strong_ordering::less == (std::vector<int>{1, 2, 3} <=> std::vector<int>{1, 2, 4}));
	assert(std::strong_ordering::greater == (std::vector<int>{1, 2, 4} <=> std::vector<int>{1, 2, 3}));

	debug("std::compare  OK");
}

#endif // DEBUG
