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

#include <types.h>
#include <printf.h>
#include <uart.hpp>

#include "../kernel.h"

#if BITS_PER_LONG >= 64
typedef long gcov_type;
#else
typedef long long gcov_type;
#endif

struct gcov_fn_info
{
	unsigned int ident;
	unsigned int checksum;
	unsigned int n_ctrs[0];
};

struct gcov_ctr_info
{
	unsigned int num;
	gcov_type *values;
	void (*merge)(gcov_type *, unsigned int);
};

struct gcov_info
{
	unsigned int version;
	struct gcov_info *next;
	unsigned int stamp;
	const char *filename;
	unsigned int n_functions;
	const struct gcov_fn_info *functions;
	unsigned int ctr_mask;
	struct gcov_ctr_info counts[0];
};

static inline nsa NIF void gcov_uart_wrapper(char c, void *unused)
{
	UNUSED(c);
	UNUSED(unused);
}

// TODO: Implement

EXTERNC nsa NIF void __gcov_init(gcov_info *p __unused)
{
}

EXTERNC nsa NIF void __gcov_exit(void)
{
}

EXTERNC nsa NIF void __gcov_flush(void)
{
}

EXTERNC nsa NIF void __gcov_merge_add(gcov_type *counters, unsigned int n_counters)
{
	UNUSED(counters);
	UNUSED(n_counters);
}

EXTERNC nsa NIF void __gcov_merge_single(gcov_type *counters, unsigned int n_counters)
{
	UNUSED(counters);
	UNUSED(n_counters);
}
