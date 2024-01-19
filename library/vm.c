/*
	This file is part of Fennix Drivers.

	Fennix Drivers is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Drivers is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Drivers. If not, see <https://www.gnu.org/licenses/>.
*/

#include <vm.h>

int __strcmp(const char *l, const char *r)
{
	for (; *l == *r && *l; l++, r++)
		;

	return *(unsigned char *)l - *(unsigned char *)r;
}

void __cpuid(uint32_t Function,
			 uint32_t *eax, uint32_t *ebx,
			 uint32_t *ecx, uint32_t *edx)
{
	asmv("cpuid"
		 : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
		 : "a"(Function));
}

bool __CheckHypervisorBit()
{
	uint32_t eax, ebx, ecx, edx;
	__cpuid(0x1, &eax, &ebx, &ecx, &edx);
	if (!(ecx & (1 << 31)))
		return false; /* Hypervisor not detected */
	return true;
}

bool __VMwareBackdoorHypervisors()
{
	const char hv[13] = {0};
	uint32_t eax, ebx, ecx, edx;
	__cpuid(0x40000000, &eax, &ebx, &ecx, &edx);

	*(uint32_t *)hv = ebx;
	*(uint32_t *)(hv + 4) = ecx;
	*(uint32_t *)(hv + 8) = edx;

	if (__strcmp(hv, "VMwareVMware") != 0 &&
		__strcmp(hv, "KVMKVMKVM") != 0 &&
		__strcmp(hv, "TCGTCGTCGTCG") != 0)
	{
		return false;
	}
	return true;
}

bool IsVMwareBackdoorAvailable()
{
	if (!__CheckHypervisorBit())
		return false;

	if (!__VMwareBackdoorHypervisors())
		return false;

	struct
	{
		union
		{
			uint32_t ax;
			uint32_t magic;
		};
		union
		{
			uint32_t bx;
			size_t size;
		};
		union
		{
			uint32_t cx;
			uint16_t command;
		};
		union
		{
			uint32_t dx;
			uint16_t port;
		};
		uint32_t si;
		uint32_t di;
	} cmd;

	cmd.si = cmd.di = 0;
	cmd.bx = ~0x564D5868;
	cmd.command = 0xA;
	cmd.magic = 0x564D5868;
	cmd.port = 0x5658;

	asmv("in %%dx, %0"
		 : "+a"(cmd.ax), "+b"(cmd.bx),
		   "+c"(cmd.cx), "+d"(cmd.dx),
		   "+S"(cmd.si), "+D"(cmd.di));

	if (cmd.bx != 0x564D5868 ||
		cmd.ax == 0xFFFFFFFF)
		return false;
	return true;
}
