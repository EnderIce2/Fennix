#ifdef DEBUG

#include <string.hpp>
#include <memory.hpp>
#include <debug.h>

/* Originally from: https://github.com/EnderIce2/FennixProject/blob/main/kernel/test.cpp */

#define MEMTEST_ITERATIONS 1024

class test_mem_new_delete
{
public:
    test_mem_new_delete();
    ~test_mem_new_delete();
};

test_mem_new_delete::test_mem_new_delete()
{
    for (char i = 0; i < 2; i++)
        ;
}

test_mem_new_delete::~test_mem_new_delete()
{
    for (char i = 0; i < 2; i++)
        ;
}

void TestMemoryAllocation()
{
    void *tmpAlloc1 = kmalloc(176);
    void *tmpAlloc2 = kmalloc(511);
    void *tmpAlloc3 = kmalloc(1027);
    void *tmpAlloc4 = kmalloc(1569);
    for (int repeat = 0; repeat < 4; repeat++)
    {
        debug("---------------[TEST %d]---------------\n", repeat);

        debug("Single Page Request Test");
        {
            uint64_t prq1 = (uint64_t)KernelAllocator.RequestPage();
            KernelAllocator.FreePage((void *)prq1);

            for (uint64_t i = 0; i < MEMTEST_ITERATIONS; i++)
                KernelAllocator.FreePage(KernelAllocator.RequestPage());

            uint64_t prq2 = (uint64_t)KernelAllocator.RequestPage();
            KernelAllocator.FreePage((void *)prq2);

            debug(" Result:\t\t1-[%#lx]; 2-[%#lx]", (void *)prq1, (void *)prq2);
            assert(prq1 == prq2);
        }

        debug("Multiple Page Request Test");
        {
            uint64_t prq1 = (uint64_t)KernelAllocator.RequestPages(10);
            KernelAllocator.FreePages((void *)prq1, 10);

            for (uint64_t i = 0; i < MEMTEST_ITERATIONS; i++)
                KernelAllocator.FreePages(KernelAllocator.RequestPages(20), 20);

            uint64_t prq2 = (uint64_t)KernelAllocator.RequestPages(10);
            KernelAllocator.FreePages((void *)prq2, 10);

            debug(" Result:\t\t1-[%#lx]; 2-[%#lx]", (void *)prq1, (void *)prq2);
            assert(prq1 == prq2);
        }

        debug("Multiple Fixed Malloc Test");
        {
            uint64_t prq1 = (uint64_t)kmalloc(0x1000);
            kfree((void *)prq1);

            for (uint64_t i = 0; i < MEMTEST_ITERATIONS; i++)
                kfree(kmalloc(0x10000));

            uint64_t prq2 = (uint64_t)kmalloc(0x1000);
            kfree((void *)prq2);

            debug(" Result:\t\t1-[%#lx]; 2-[%#lx]", (void *)prq1, (void *)prq2);
            assert(prq1 == prq2);
        }

        debug("Multiple Dynamic Malloc Test");
        {
            uint64_t prq1 = (uint64_t)kmalloc(0x1000);
            kfree((void *)prq1);

            for (uint64_t i = 0; i < MEMTEST_ITERATIONS; i++)
                kfree(kmalloc(i));

            uint64_t prq2 = (uint64_t)kmalloc(0x1000);
            kfree((void *)prq2);

            debug(" Result:\t1-[%#lx]; 2-[%#lx]", (void *)prq1, (void *)prq2);
            assert(prq1 == prq2);
        }

        debug("New/Delete Test");
        {
            uint64_t prq1 = (uint64_t)kmalloc(0x1000);
            kfree((void *)prq1);

            for (uint64_t i = 0; i < MEMTEST_ITERATIONS; i++)
            {
                test_mem_new_delete *t = new test_mem_new_delete();
                delete t;
            }

            uint64_t prq2 = (uint64_t)kmalloc(0x1000);
            kfree((void *)prq2);

            debug(" Result:               \t1-[%#lx]; 2-[%#lx]", (void *)prq1, (void *)prq2);
            assert(prq1 == prq2);
        }

        debug("New/Delete Fixed Array Test");
        {
            uint64_t prq1 = (uint64_t)kmalloc(0x1000);
            kfree((void *)prq1);

            for (uint64_t i = 0; i < MEMTEST_ITERATIONS; i++)
            {
                char *t = new char[128];
                delete[] t;
            }

            uint64_t prq2 = (uint64_t)kmalloc(0x1000);
            kfree((void *)prq2);

            debug(" Result:    \t1-[%#lx]; 2-[%#lx]", (void *)prq1, (void *)prq2);
            assert(prq1 == prq2);
        }

        debug("New/Delete Dynamic Array Test");
        {
            uint64_t prq1 = (uint64_t)kmalloc(0x1000);
            kfree((void *)prq1);

            for (uint64_t i = 0; i < MEMTEST_ITERATIONS; i++)
            {
                if (i == 0)
                    continue;
                char *t = new char[i];
                delete[] t;
            }

            uint64_t prq2 = (uint64_t)kmalloc(0x1000);
            kfree((void *)prq2);

            debug(" Result:\t1-[%#lx]; 2-[%#lx]\n", (void *)prq1, (void *)prq2);
            assert(prq1 == prq2);
        }

        debug("calloc Test");
        {
            uint64_t prq1 = (uint64_t)kmalloc(0x1000);
            kfree((void *)prq1);

            for (uint64_t i = 0; i < MEMTEST_ITERATIONS; i++)
            {
                char *t = (char *)kcalloc(128, 1);
                kfree(t);
            }

            uint64_t prq2 = (uint64_t)kmalloc(0x1000);
            kfree((void *)prq2);

            debug(" Result:\t1-[%#lx]; 2-[%#lx]", (void *)prq1, (void *)prq2);
            assert(prq1 == prq2);
        }

        debug("realloc Test");
        {
            uint64_t prq1 = (uint64_t)kmalloc(0x1000);
            kfree((void *)prq1);

            for (uint64_t i = 0; i < MEMTEST_ITERATIONS; i++)
            {
                char *t = (char *)kmalloc(128);
                t = (char *)krealloc(t, 256);
                kfree(t);
            }

            uint64_t prq2 = (uint64_t)kmalloc(0x1000);
            kfree((void *)prq2);

            debug(" Result:\t1-[%#lx]; 2-[%#lx]", (void *)prq1, (void *)prq2);
            assert(prq1 == prq2);
        }
    }

    kfree(tmpAlloc1);
    kfree(tmpAlloc2);
    kfree(tmpAlloc3);
    kfree(tmpAlloc4);

    debug("Memory Stress Test");
    for (size_t i = 0; i < 0x1000; i++)
        kfree(kmalloc(i));

    debug("Invalid Usage Test");
    kfree(tmpAlloc1);
    kfree(tmpAlloc2);
    kfree(tmpAlloc3);
    kfree(tmpAlloc4);

    void *InvMlc = kmalloc(0);
    assert(InvMlc == nullptr);
    krealloc(InvMlc, 0);
    assert(InvMlc == nullptr);
    kcalloc(0, 0);
    assert(InvMlc == nullptr);
    kcalloc(1, 0);
    assert(InvMlc == nullptr);
    kcalloc(0, 1);
    assert(InvMlc == nullptr);
    kfree(InvMlc);

    debug("Memory Test Complete\n");
}

#endif // DEBUG
