#include <types.h>
#include <memory.hpp>
#include <convert.h>

extern bool EnableExternalMemoryTracer;
extern bool DebuggerIsAttached;

__constructor void TestMemoryOperations()
{
    if (EnableExternalMemoryTracer || DebuggerIsAttached)
    {
        debug("The test is disabled when the external memory tracer or a debugger is enabled.");
        return;
    }

    int arr1[5] = {1, 2, 3, 4, 5};
    int arr2[5] = {0, 0, 0, 0, 0};
    char str1[] = "Hello";
    char str2[] = "World";

    memcpy_unsafe(arr2, arr1, sizeof(arr1));
    debug("memcpy: arr2[0]=%d, arr2[1]=%d, arr2[2]=%d, arr2[3]=%d, arr2[4]=%d\n",
          arr2[0], arr2[1], arr2[2], arr2[3], arr2[4]);
    if (memcmp(arr1, arr2, sizeof(arr1)) != 0)
    {
        error("memcpy failed!\n");
        while (1)
            ;
    }

    memset_unsafe(arr2, 0, sizeof(arr2));
    debug("memset: arr2[0]=%d, arr2[1]=%d, arr2[2]=%d, arr2[3]=%d, arr2[4]=%d\n",
          arr2[0], arr2[1], arr2[2], arr2[3], arr2[4]);
    if (memcmp(arr1, arr2, sizeof(arr1)) == 0)
    {
        error("memset failed!\n");
        while (1)
            ;
    }

    memmove_unsafe(str1 + 3, str1, strlen(str1) + 1);
    debug("memmove: str1=%s\n", str1);
    if (strcmp(str1, "HelHello") != 0)
    {
        error("memmove failed!\n");
        while (1)
            ;
    }

    char carr[1024];
    char carrFull[1024];
    char carrTo[16];

    for (size_t i = 0; i < 512; i++)
    {
        for (size_t i = 0; i < 16; i++)
            carrTo[i] = 'a';

        for (size_t i = 0; i < 1024; i += 16)
            memcpy_unsafe(carr + i, carrTo, 16);

        for (size_t i = 0; i < 1024; i++)
        {
            if (carr[i] != 'a')
            {
                error("memcpy failed!\n");
                while (1)
                    ;
            }
        }

        for (size_t i = 0; i < 1024; i++)
            carrFull[i] = 'b';

        memcpy_unsafe(carr, carrFull, 1024);

        for (size_t i = 0; i < 1024; i++)
        {
            if (carr[i] != 'b')
            {
                error("memcpy failed!\n");
                while (1)
                    ;
            }
        }
    }

    for (size_t i = 0; i < 512; i++)
    {
        for (size_t i = 0; i < 1024; i += 16)
            memset_unsafe(carr + i, 'c', 16);

        for (size_t i = 0; i < 1024; i++)
        {
            if (carr[i] != 'c')
            {
                error("memcpy failed!\n");
                while (1)
                    ;
            }
        }
    }

    for (size_t i = 0; i < 512; i++)
    {
        memset_unsafe(carr, 'd', 1024);

        for (size_t i = 0; i < 1024; i++)
        {
            if (carr[i] != 'd')
            {
                error("memset failed!\n");
                while (1)
                    ;
            }
        }
    }

    debug("Memory operations test passed");
}
