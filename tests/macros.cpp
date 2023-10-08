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

#include <types.h>
#include <memory/macro.hpp>
#include <memory/vma.hpp>
#include <assert.h>
#include <debug.h>

#include <filesystem.hpp>
#include "../syscalls.h"

/* static assert, no constructor needed */

#ifdef a64
#if UINTPTR_MAX != UINT64_MAX
#error "uintptr_t is not 64-bit!"
#endif // UINTPTR_MAX != UINT64_MAX
#endif // a64

#ifdef a32
#if UINTPTR_MAX != UINT32_MAX
#error "uintptr_t is not 32-bit!"
#endif // UINTPTR_MAX != UINT32_MAX
#endif // a32

#ifdef aa64
#if UINTPTR_MAX != UINT64_MAX
#error "uintptr_t is not 64-bit!"
#endif // UINTPTR_MAX != UINT64_MAX
#endif // aa64

void TestSeekMacros()
{
	static_assert(sc_SEEK_SET == SEEK_SET);
	static_assert(sc_SEEK_CUR == SEEK_CUR);
	static_assert(sc_SEEK_END == SEEK_END);
}

__constructor void TestMacros()
{
	{
		int a = TO_PAGES(4096);
		int b = FROM_PAGES(1);

		debug("a: 4096 -> %d", a);
		debug("b:   a  -> %d", b);

		if (a != 1)
		{
			error("t1: TO_PAGES is not equal to 1");
			inf_loop;
		}

		if (b != 4096)
		{
			error("t1: FROM_PAGES is not equal to 4096");
			inf_loop;
		}
	}

	{
		int a = TO_PAGES(4097);
		int b = FROM_PAGES(2);

		debug("a: 4097 -> %d", a);
		debug("b:   a  -> %d", b);

		if (a != 2)
		{
			error("t2: TO_PAGES is not equal to 2");
			inf_loop;
		}

		if (b != 8192)
		{
			error("t2: FROM_PAGES is not equal to 8192");
			inf_loop;
		}
	}

	{
		int a = 10;
		assert(a == 10);

		const char *str = "Hello";
		assert(str != nullptr && str[0] == 'H');

		bool flag = false;
		assert(!flag);
	}

	debug("-------------------------");

	{
		uint64_t bytes = PAGE_SIZE;
		uint64_t pgs = 1;

		for (int i = 0; i < 128; i++)
		{
			uint64_t cnv_to_pgs = TO_PAGES(bytes);
			uint64_t cnv_from_pgs = FROM_PAGES(pgs);

			if (cnv_to_pgs != pgs)
			{
				error("TO_PAGES is not equal to %d (pages: %d)", pgs, cnv_to_pgs);
				inf_loop;
			}

			if (cnv_from_pgs != bytes)
			{
				error("FROM_PAGES is not equal to %d (bytes: %d)", bytes, cnv_from_pgs);
				inf_loop;
			}

			bytes += PAGE_SIZE;
			pgs++;
		}
	}

	{
		debug("Testing ROUND_UP and ROUND_DOWN");
		int x = 0x101;
		int y = 0x100;
		int result;

		result = ROUND_UP(x, y);
		if (result != 0x200)
		{
			error("ERROR: ROUND_UP failed: %d != 0x200", result);
			inf_loop;
		}

		result = ROUND_DOWN(x, y);
		if (result != 0x100)
		{
			error("ERROR: ROUND_DOWN failed: %d != 0x100", result);
			inf_loop;
		}
	}
}

#endif // DEBUG
