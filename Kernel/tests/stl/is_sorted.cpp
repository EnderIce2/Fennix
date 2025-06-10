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
#include <algorithm>

void test_stl_is_sorted()
{
	int a[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	assert(std::is_sorted(a, a + (sizeof(a) / sizeof(int))));

	int b[] = {1, 2, 3, 4, 5, 6, 7, 8, 10, 9};
	assert(!std::is_sorted(b, b + (sizeof(b) / sizeof(int))));
}

#endif
