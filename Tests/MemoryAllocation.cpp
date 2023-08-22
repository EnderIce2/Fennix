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

#include <memory.hpp>
#include <debug.h>
#include <string>

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

extern bool DebuggerIsAttached;

void TestMemoryAllocation()
{
#ifdef a32
	return; /* Not ready for now. */
#endif
	if (DebuggerIsAttached)
	{
		debug("The test is disabled when the debugger is enabled.");
		return;
	}

	void *tmpAlloc1 = kmalloc(176);
	void *tmpAlloc2 = kmalloc(511);
	void *tmpAlloc3 = kmalloc(1027);
	void *tmpAlloc4 = kmalloc(1569);
	for (int repeat = 0; repeat < 4; repeat++)
	{
		debug("---------------[TEST %d]---------------\n", repeat);

		debug("Single Page Request Test");
		{
			uintptr_t prq1 = (uintptr_t)KernelAllocator.RequestPage();
			KernelAllocator.FreePage((void *)prq1);

			for (size_t i = 0; i < MEMTEST_ITERATIONS; i++)
				KernelAllocator.FreePage(KernelAllocator.RequestPage());

			uintptr_t prq2 = (uintptr_t)KernelAllocator.RequestPage();
			KernelAllocator.FreePage((void *)prq2);

			debug(" Result:\t\t1-[%#lx]; 2-[%#lx]", (void *)prq1, (void *)prq2);
			assert(prq1 == prq2);
		}

		debug("Multiple Page Request Test");
		{
			uintptr_t prq1 = (uintptr_t)KernelAllocator.RequestPages(10);
			KernelAllocator.FreePages((void *)prq1, 10);

			for (size_t i = 0; i < MEMTEST_ITERATIONS; i++)
				KernelAllocator.FreePages(KernelAllocator.RequestPages(20), 20);

			uintptr_t prq2 = (uintptr_t)KernelAllocator.RequestPages(10);
			KernelAllocator.FreePages((void *)prq2, 10);

			debug(" Result:\t\t1-[%#lx]; 2-[%#lx]", (void *)prq1, (void *)prq2);
			assert(prq1 == prq2);
		}

		debug("Multiple Fixed Malloc Test");
		{
			uintptr_t prq1 = (uintptr_t)kmalloc(0x1000);
			kfree((void *)prq1);

			for (size_t i = 0; i < MEMTEST_ITERATIONS; i++)
			{
				kfree(kmalloc(0x10000));
				kfree(kmalloc(0x1000));
				kfree(kmalloc(0x100));
				kfree(kmalloc(0x10));
				kfree(kmalloc(0x1));
			}

			uintptr_t prq2 = (uintptr_t)kmalloc(0x1000);
			kfree((void *)prq2);

			debug(" Result:\t\t1-[%#lx]; 2-[%#lx]", (void *)prq1, (void *)prq2);
			assert(prq1 == prq2);
		}

		debug("Multiple Dynamic Malloc Test");
		{
			uintptr_t prq1 = (uintptr_t)kmalloc(0x1000);
			kfree((void *)prq1);

			for (size_t i = 1; i < MEMTEST_ITERATIONS; i++)
				kfree(kmalloc(i));

			uintptr_t prq2 = (uintptr_t)kmalloc(0x1000);
			kfree((void *)prq2);

			debug(" Result:\t1-[%#lx]; 2-[%#lx]", (void *)prq1, (void *)prq2);
			assert(prq1 == prq2);
		}

		debug("New/Delete Test");
		{
			uintptr_t prq1 = (uintptr_t)kmalloc(0x1000);
			kfree((void *)prq1);

			for (size_t i = 0; i < MEMTEST_ITERATIONS; i++)
			{
				test_mem_new_delete *t = new test_mem_new_delete();
				delete t;
			}

			uintptr_t prq2 = (uintptr_t)kmalloc(0x1000);
			kfree((void *)prq2);

			debug(" Result:               \t1-[%#lx]; 2-[%#lx]", (void *)prq1, (void *)prq2);
			assert(prq1 == prq2);
		}

		debug("New/Delete Fixed Array Test");
		{
			uintptr_t prq1 = (uintptr_t)kmalloc(0x1000);
			kfree((void *)prq1);

			for (size_t i = 0; i < MEMTEST_ITERATIONS; i++)
			{
				char *t = new char[128];
				delete[] t;
			}

			uintptr_t prq2 = (uintptr_t)kmalloc(0x1000);
			kfree((void *)prq2);

			debug(" Result:    \t1-[%#lx]; 2-[%#lx]", (void *)prq1, (void *)prq2);
			assert(prq1 == prq2);
		}

		debug("New/Delete Dynamic Array Test");
		{
			uintptr_t prq1 = (uintptr_t)kmalloc(0x1000);
			kfree((void *)prq1);

			for (size_t i = 0; i < MEMTEST_ITERATIONS; i++)
			{
				if (i == 0)
					continue;
				char *t = new char[i];
				delete[] t;
			}

			uintptr_t prq2 = (uintptr_t)kmalloc(0x1000);
			kfree((void *)prq2);

			debug(" Result:\t1-[%#lx]; 2-[%#lx]", (void *)prq1, (void *)prq2);
			assert(prq1 == prq2);
		}

		debug("calloc Test");
		{
			uintptr_t prq1 = (uintptr_t)kmalloc(0x1000);
			kfree((void *)prq1);

			for (size_t i = 0; i < MEMTEST_ITERATIONS; i++)
			{
				char *t = (char *)kcalloc(128, 1);
				kfree(t);
			}

			uintptr_t prq2 = (uintptr_t)kmalloc(0x1000);
			kfree((void *)prq2);

			debug(" Result:\t1-[%#lx]; 2-[%#lx]", (void *)prq1, (void *)prq2);
			assert(prq1 == prq2);
		}

		debug("realloc Test");
		{
			uintptr_t prq1 = (uintptr_t)kmalloc(0x1000);
			kfree((void *)prq1);

			for (size_t i = 0; i < MEMTEST_ITERATIONS; i++)
			{
				char *t = (char *)kmalloc(128);
				t = (char *)krealloc(t, 256);
				kfree(t);
			}

			uintptr_t prq2 = (uintptr_t)kmalloc(0x1000);
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
	for (size_t i = 1; i < 0x1000; i++)
		kfree(kmalloc(i));

	debug("Invalid Usage Test");
	kfree(tmpAlloc1);
	kfree(tmpAlloc2);
	kfree(tmpAlloc3);
	kfree(tmpAlloc4);

	/* Allocation functions have assertions to check for invalid usage */

	// void *InvMlc = kmalloc(0);
	// assert(InvMlc == nullptr);
	// krealloc(InvMlc, 0);
	// assert(InvMlc == nullptr);
	// kcalloc(0, 0);
	// assert(InvMlc == nullptr);
	// kcalloc(1, 0);
	// assert(InvMlc == nullptr);
	// kcalloc(0, 1);
	// assert(InvMlc == nullptr);
	// kfree(InvMlc);

	debug("Memory Test Complete");
}

#endif // DEBUG
