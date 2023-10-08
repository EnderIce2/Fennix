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
#include <convert.h>


extern bool DebuggerIsAttached;
extern Memory::MemoryAllocatorType AllocatorType;

__constructor void TestMemoryOperations()
{
	if (DebuggerIsAttached)
	{
		debug("The test is disabled when the debugger is enabled.");
		return;
	}

	if (AllocatorType == Memory::MemoryAllocatorType::Pages)
	{
		debug("The test is disabled when the allocator is set to pages.");
		return;
	}

	int arr1[5] = {1, 2, 3, 4, 5};
	int arr2[5] = {0, 0, 0, 0, 0};
	char str1[] = "Hello";
	char str2[] = "World";

	memcpy_unsafe(arr2, arr1, sizeof(arr1));
	debug("memcpy: arr2[0]=%d, arr2[1]=%d, arr2[2]=%d, arr2[3]=%d, arr2[4]=%d",
		  arr2[0], arr2[1], arr2[2], arr2[3], arr2[4]);
	if (memcmp(arr1, arr2, sizeof(arr1)) != 0)
	{
		error("memcpy failed!");
		inf_loop;
	}

	memset_unsafe(arr2, 0, sizeof(arr2));
	debug("memset: arr2[0]=%d, arr2[1]=%d, arr2[2]=%d, arr2[3]=%d, arr2[4]=%d",
		  arr2[0], arr2[1], arr2[2], arr2[3], arr2[4]);
	if (memcmp(arr1, arr2, sizeof(arr1)) == 0)
	{
		error("memset failed!");
		inf_loop;
	}

	memmove_unsafe(str1 + 3, str1, strlen(str1) + 1);
	debug("memmove: str1=%s", str1);
	if (strcmp(str1, "HelHello") != 0)
	{
		error("memmove failed!");
		inf_loop;
	}

	char carr[512];
	char carrTo[16];

	for (size_t i = 0; i < 512; i++)
	{
		for (size_t i = 0; i < 16; i++)
			carrTo[i] = 'a';

		for (size_t i = 0; i < 512; i += 16)
			memcpy_unsafe(carr + i, carrTo, 16);

		for (size_t i = 0; i < 512; i++)
		{
			if (carr[i] != 'a')
			{
				error("memcpy failed!");
				while (1)
					;
			}
		}

		{
			char carrFull[512];
			for (size_t i = 0; i < 512; i++)
				carrFull[i] = 'b';

			memcpy_unsafe(carr, carrFull, 512);

			for (size_t i = 0; i < 512; i++)
			{
				if (carr[i] != 'b')
				{
					error("memcpy failed!");
					while (1)
						;
				}
			}
		}
	}

	for (size_t i = 0; i < 512; i++)
	{
		for (size_t i = 0; i < 512; i += 16)
			memset_unsafe(carr + i, 'c', 16);

		for (size_t i = 0; i < 512; i++)
		{
			if (carr[i] != 'c')
			{
				error("memcpy failed!");
				while (1)
					;
			}
		}
	}

	for (size_t i = 0; i < 512; i++)
	{
		memset_unsafe(carr, 'd', 512);

		for (size_t i = 0; i < 512; i++)
		{
			if (carr[i] != 'd')
			{
				error("memset failed!");
				while (1)
					;
			}
		}
	}

	debug("Memory operations test passed");
}

#endif
