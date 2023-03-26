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
        uintptr_t actual = PAGE_SIZE;
        int expected = 1;

        for (int i = 0; i < 128; i++)
        {
            int a = TO_PAGES(actual);
            uintptr_t b = FROM_PAGES(expected);

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