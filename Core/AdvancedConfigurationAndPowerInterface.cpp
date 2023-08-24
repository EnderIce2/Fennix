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

#include <acpi.hpp>

#include <debug.h>
#include <io.h>

#include "../kernel.h"

namespace ACPI
{
	__no_sanitize("alignment") void *ACPI::FindTable(ACPI::ACPIHeader *ACPIHeader, char *Signature)
	{
		for (uint64_t t = 0; t < ((ACPIHeader->Length - sizeof(ACPI::ACPIHeader)) / (XSDTSupported ? 8 : 4)); t++)
		{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"

			// TODO: Should I be concerned about unaligned memory access?
			ACPI::ACPIHeader *SDTHdr = nullptr;
			if (XSDTSupported)
				SDTHdr = (ACPI::ACPIHeader *)(*(uint64_t *)((uint64_t)ACPIHeader + sizeof(ACPI::ACPIHeader) + (t * 8)));
			else
				SDTHdr = (ACPI::ACPIHeader *)(*(uint32_t *)((uint64_t)ACPIHeader + sizeof(ACPI::ACPIHeader) + (t * 4)));

#pragma GCC diagnostic pop

			for (short i = 0; i < 4; i++)
			{
				if (SDTHdr->Signature[i] != Signature[i])
					break;
				if (i == 3)
				{
					trace("%s found at address %p", Signature, (uintptr_t)SDTHdr);
					return SDTHdr;
				}
			}
		}
		// warn("%s not found!", Signature);
		return nullptr;
	}

	void ACPI::SearchTables(ACPIHeader *Header)
	{
		if (!Header)
			return;

		HPET = (HPETHeader *)FindTable(Header, (char *)"HPET");
		FADT = (FADTHeader *)FindTable(Header, (char *)"FACP");
		MCFG = (MCFGHeader *)FindTable(Header, (char *)"MCFG");
		BGRT = (BGRTHeader *)FindTable(Header, (char *)"BGRT");
		SRAT = (SRATHeader *)FindTable(Header, (char *)"SRAT");
		TPM2 = (TPM2Header *)FindTable(Header, (char *)"TPM2");
		TCPA = (TCPAHeader *)FindTable(Header, (char *)"TCPA");
		WAET = (WAETHeader *)FindTable(Header, (char *)"WAET");
		MADT = (MADTHeader *)FindTable(Header, (char *)"APIC");
		HEST = (HESTHeader *)FindTable(Header, (char *)"HEST");
		FindTable(Header, (char *)"BERT");
		FindTable(Header, (char *)"CPEP");
		FindTable(Header, (char *)"DSDT");
		FindTable(Header, (char *)"ECDT");
		FindTable(Header, (char *)"EINJ");
		FindTable(Header, (char *)"ERST");
		FindTable(Header, (char *)"FACS");
		FindTable(Header, (char *)"MSCT");
		FindTable(Header, (char *)"MPST");
		FindTable(Header, (char *)"OEMx");
		FindTable(Header, (char *)"PMTT");
		FindTable(Header, (char *)"PSDT");
		FindTable(Header, (char *)"RASF");
		FindTable(Header, (char *)"RSDT");
		FindTable(Header, (char *)"SBST");
		FindTable(Header, (char *)"SLIT");
		FindTable(Header, (char *)"SSDT");
		FindTable(Header, (char *)"XSDT");
		FindTable(Header, (char *)"DRTM");
		FindTable(Header, (char *)"FPDT");
		FindTable(Header, (char *)"GTDT");
		FindTable(Header, (char *)"PCCT");
		FindTable(Header, (char *)"S3PT");
		FindTable(Header, (char *)"MATR");
		FindTable(Header, (char *)"MSDM");
		FindTable(Header, (char *)"WPBT");
		FindTable(Header, (char *)"OSDT");
		FindTable(Header, (char *)"RSDP");
		FindTable(Header, (char *)"NFIT");
		FindTable(Header, (char *)"ASF!");
		FindTable(Header, (char *)"BOOT");
		FindTable(Header, (char *)"CSRT");
		FindTable(Header, (char *)"DBG2");
		FindTable(Header, (char *)"DBGP");
		FindTable(Header, (char *)"DMAR");
		FindTable(Header, (char *)"IBFT");
		FindTable(Header, (char *)"IORT");
		FindTable(Header, (char *)"IVRS");
		FindTable(Header, (char *)"LPIT");
		FindTable(Header, (char *)"MCHI");
		FindTable(Header, (char *)"MTMR");
		FindTable(Header, (char *)"SLIC");
		FindTable(Header, (char *)"SPCR");
		FindTable(Header, (char *)"SPMI");
		FindTable(Header, (char *)"UEFI");
		FindTable(Header, (char *)"VRTC");
		FindTable(Header, (char *)"WDAT");
		FindTable(Header, (char *)"WDDT");
		FindTable(Header, (char *)"WDRT");
		FindTable(Header, (char *)"ATKG");
		FindTable(Header, (char *)"GSCI");
		FindTable(Header, (char *)"IEIT");
		FindTable(Header, (char *)"HMAT");
		FindTable(Header, (char *)"CEDT");
		FindTable(Header, (char *)"AEST");
	}

	ACPI::ACPI()
	{
		trace("Initializing ACPI");
		if (bInfo.RSDP->Revision >= 2 && bInfo.RSDP->XSDTAddress)
		{
			debug("XSDT supported");
			XSDTSupported = true;
			XSDT = (ACPIHeader *)(bInfo.RSDP->XSDTAddress);
		}
		else
		{
			debug("RSDT supported");
			XSDT = (ACPIHeader *)(uintptr_t)bInfo.RSDP->RSDTAddress;
		}

		if (!Memory::Virtual().Check(XSDT))
		{
			warn("%s is not mapped!",
				 XSDTSupported ? "XSDT" : "RSDT");
			debug("XSDT: %p", XSDT);
			Memory::Virtual().Map(XSDT, XSDT, Memory::RW);
		}

		this->SearchTables(XSDT);

		if (FADT)
		{
			outb(s_cst(uint16_t, FADT->SMI_CommandPort), FADT->AcpiEnable);
			while (!(inw(s_cst(uint16_t, FADT->PM1aControlBlock)) & 1))
				;
		}
	}

	ACPI::~ACPI()
	{
	}
}
