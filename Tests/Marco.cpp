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
#include <memory.hpp>
#include <debug.h>

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
}

#endif // DEBUG
