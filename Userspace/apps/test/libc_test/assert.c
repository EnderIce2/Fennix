/*
	This file is part of Fennix Userspace.

	Fennix Userspace is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Userspace is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Userspace. If not, see <https://www.gnu.org/licenses/>.
*/

#include <assert.h>

/* https://pubs.opengroup.org/onlinepubs/9799919799/functions/assert.html */

int test_assert(void)
{
	assert(1 == 1);
#define A 1
#define B 2
	assert(A != B);
	assert((A == 1) && (B == 2));
	assert(1);
	assert(!0);
	return 0;
}
