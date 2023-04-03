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
#include <memory.hpp>
#include <debug.h>

__constructor void TestMacros()
{
    {
        int a = TO_PAGES(4096);
        int b = FROM_PAGES(2);

        debug("a: 4096 -> %d", a);
        debug("b:   a  -> %d", b);

        if (a != 2)
        {
            error("TO_PAGES is not equal to 2");
            while (1)
                ;
        }

        if (b != 8192)
        {
            error("FROM_PAGES is not equal to 8192");
            while (1)
                ;
        }
    }

    debug("-------------------------");

    {
        uint64_t actual = PAGE_SIZE;
        uint64_t expected = 1;

        for (int i = 0; i < 128; i++)
        {
            uint64_t a = TO_PAGES(actual);
            uint64_t b = FROM_PAGES(expected);

            /* TODO: This is a workaround for now. */
            if (a != expected + 1)
            {
                error("TO_PAGES is not equal to %d (actual: %d)", expected, a);
                while (1)
                    ;
            }

            if (b != actual)
            {
                error("FROM_PAGES is not equal to %d (actual: %d)", actual, b);
                while (1)
                    ;
            }

            actual += PAGE_SIZE;
            expected++;
        }
    }
}