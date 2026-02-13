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

#include <quirks.hpp>

namespace hwquirks
{
	struct
	{
		int16_t Class;
		int16_t Subclass;
		int16_t ProgIF;
		int16_t RevisionID;
		int32_t VendorID;
		int32_t DeviceID;
		void (*Apply)(PCI::PCIDevice Device);
	} quirks[] = {
#define ANY -1
		{0x0C, 0x03, 0x30, ANY, PCI::IntelCorporation, ANY, nullptr},
	};

	void ApplyQuirks(PCI::PCIDevice Device)
	{
		for (auto &&quirk : quirks)
		{
			if ((quirk.Class == ANY || quirk.Class == Device.Header->Class) &&
				(quirk.Subclass == ANY || quirk.Subclass == Device.Header->Subclass) &&
				(quirk.ProgIF == ANY || quirk.ProgIF == Device.Header->ProgIF) &&
				(quirk.RevisionID == ANY || quirk.RevisionID == Device.Header->RevisionID) &&
				(quirk.VendorID == ANY || quirk.VendorID == Device.GetVendorID()) &&
				(quirk.DeviceID == ANY || quirk.DeviceID == Device.GetDeviceID()))
			{
				if (quirk.Apply != nullptr)
					quirk.Apply(Device);
			}
		}
	}
#undef ANY

	bool IsQEMU()
	{
		static bool result = false;

		static int once = 0;
		if (!once++)
		{
			if (strcmp(CPU::Hypervisor(), x86_CPUID_VENDOR_KVM) == 0)
				result = true;
			else if (strcmp(CPU::Hypervisor(), x86_CPUID_VENDOR_TCG) == 0)
				result = true;
		}

		return result;
	}
}
