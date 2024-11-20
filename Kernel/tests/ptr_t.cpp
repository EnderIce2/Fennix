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
#include <debug.h>
#include <assert.h>

class __ptr_t_demo
{
public:
	ptr_t<void *> Address;

	__ptr_t_demo(ptr_t<void *> Address, ptr_t<void *> Address2 = nullptr)
	{
		this->Address = Address;
	}

	~__ptr_t_demo() {}

	void *test1()
	{
		return Address + 0xdeadbeef;
	}

	uintptr_t test2()
	{
		return Address + 0xdeadbeef;
	}

	void *Map(ptr_t<void *> VirtualAddress, ptr_t<void *> &PhysicalAddress, size_t Length)
	{
		ptr_t<void *> Final;
		for (size_t i = 0; i < Length; i++)
			Final = VirtualAddress + (i * 0x1000);

		PhysicalAddress = 0;
		return Final;
	}
};

__constructor void TestPtr_T()
{
	__ptr_t_demo demo(nullptr);
	assert(demo.test1() == (void *)0xdeadbeef);
	assert(demo.test2() == 0xdeadbeef);

	ptr_t<void *> VirtualAddress = 0x1000;
	ptr_t<void *> PhysicalAddress = 0x2000;
	size_t Length = 4;

	void *ret = demo.Map(VirtualAddress, PhysicalAddress, Length);
	assert(ret == (void *)0x4000);

	VirtualAddress -= 0x1000;
	assert(PhysicalAddress == 0);

	VirtualAddress = 0x1;
	assert(VirtualAddress > PhysicalAddress);
	assert(VirtualAddress > 0);

	PhysicalAddress = 0x2;
	assert(VirtualAddress < PhysicalAddress);
	PhysicalAddress = 0;
	assert(!(PhysicalAddress < 0));

	PhysicalAddress = 0x1;
	assert(VirtualAddress >= PhysicalAddress);
	assert(VirtualAddress >= 0);

	VirtualAddress = 0;
	PhysicalAddress = 0;
	assert(VirtualAddress <= PhysicalAddress);
	assert(PhysicalAddress <= 0);

	VirtualAddress = 0x1000;
	assert(VirtualAddress == 0x1000);
	assert(VirtualAddress != 0x2000);

	PhysicalAddress = 0x1000;
	assert(PhysicalAddress == 0x1000);
	assert(PhysicalAddress != 0x2000);

	ptr_t<void *> v;
	v += 1;
	assert(v == 1);
	v -= 1;
	assert(v == 0);
	v -= 1;
	assert(v == (void *)-1);
}

#endif // DEBUG
