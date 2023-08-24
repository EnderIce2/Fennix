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

#include <vm.hpp>

#include <acpi.hpp>
#include <cpu.hpp>
#include <debug.h>

#include "../kernel.h"

bool DetectByHypervisor()
{
	const char *Hypervisor = CPU::Hypervisor();
	if (strcmp(Hypervisor, x86_CPUID_VENDOR_VMWARE) == 0)
		goto Yes;
	else if (strcmp(Hypervisor, x86_CPUID_VENDOR_XENHVM) == 0)
		goto Yes;
	else if (strcmp(Hypervisor, x86_CPUID_VENDOR_MICROSOFT_HV) == 0)
		goto Yes;
	else if (strcmp(Hypervisor, x86_CPUID_VENDOR_MICROSOFT_XTA) == 0)
		goto Yes;
	else if (strcmp(Hypervisor, x86_CPUID_VENDOR_PARALLELS) == 0)
		goto Yes;
	else if (strcmp(Hypervisor, x86_CPUID_VENDOR_KVM) == 0)
		goto Yes;
	else if (strcmp(Hypervisor, x86_CPUID_VENDOR_VIRTUALBOX) == 0)
		goto Yes;
	else if (strcmp(Hypervisor, x86_CPUID_VENDOR_TCG) == 0)
		goto Yes;
	else if (strcmp(Hypervisor, x86_CPUID_VENDOR_BHYVE) == 0)
		goto Yes;
	else if (strcmp(Hypervisor, x86_CPUID_VENDOR_ACRN) == 0)
		goto Yes;
	else if (strcmp(Hypervisor, x86_CPUID_VENDOR_QNX) == 0)
		goto Yes;
	else if (strcmp(Hypervisor, x86_CPUID_VENDOR_APPLE) == 0)
		goto Yes;
	return false;

Yes:
	debug("Hypervisor: %s", Hypervisor);
	return true;
}

bool IsVMwareBackdoorAvailable()
{
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
	} cmd{};

#define VMWARE_MAGIC 0x564D5868 /* hXMV */
#define VMWARE_PORT 0x5658
#define CMD_GETVERSION 0xA

	cmd.bx = ~VMWARE_MAGIC;
	cmd.command = CMD_GETVERSION;
	cmd.magic = VMWARE_MAGIC;
	cmd.port = VMWARE_PORT;

	asmv("in %%dx, %0"
		 : "+a"(cmd.ax),
		   "+b"(cmd.bx),
		   "+c"(cmd.cx),
		   "+d"(cmd.dx),
		   "+S"(cmd.si),
		   "+D"(cmd.di));

	if (cmd.bx != VMWARE_MAGIC || cmd.ax == 0xFFFFFFFF)
		return false;

	debug("VMware backdoor version: %d.%d.%d",
		  cmd.ax >> 16,
		  (cmd.ax >> 8) & 0xFF,
		  cmd.ax & 0xFF);

	return true;
}

bool DetectByCPUID()
{
	bool IsVM = false;
	if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_INTEL) == 0)
	{
		CPU::x86::Intel::CPUID0x00000001 cpuid00000001;
		cpuid00000001.Get();

		if (cpuid00000001.ECX.Hypervisor == 1)
		{
			debug("Intel: Hypervisor: %d",
				  cpuid00000001.ECX.Hypervisor);
			IsVM = true;
		}
	}
	else if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_AMD) == 0)
	{
		CPU::x86::AMD::CPUID0x00000001 cpuid00000001;
		cpuid00000001.Get();

		if (cpuid00000001.ECX.Hypervisor == 1)
		{
			debug("AMD: Hypervisor: %d",
				  cpuid00000001.ECX.Hypervisor);
			IsVM = true;
		}
	}
	return IsVM;
}

bool DetectByHPET()
{
	void *acpi = PowerManager->GetACPI();
	if (!acpi)
		return false;

	void *hpet = ((ACPI::ACPI *)acpi)->HPET;
	if (!hpet)
		return false;

	ACPI::ACPI::HPETHeader *HPET = (ACPI::ACPI::HPETHeader *)hpet;

	if (strstr((const char *)HPET->Header.OEMID, "BOCHS BXPC") != NULL)
		goto Yes;

	if (strstr((const char *)HPET->Header.OEMID, "VBOX") != NULL)
		goto Yes;

	if (strstr((const char *)HPET->Header.OEMID, "VMWAREVMW") != NULL)
		goto Yes;

	return false;

Yes:
	debug("HPET: %s", HPET->Header.OEMID);
	return true;
}

bool IsVirtualizedEnvironment()
{
	bool IsVM = false;

	debug("Detecting virtualized environment...");

	if (DetectByHypervisor())
		IsVM = true;

	if (IsVMwareBackdoorAvailable())
		IsVM = true;

	if (DetectByCPUID())
		IsVM = true;

	if (DetectByHPET())
		IsVM = true;

	/* TODO: Add more detection methods */

	debug("Virtualized environment: %s",
		  IsVM ? "Yes" : "No");
	return IsVM;
}
