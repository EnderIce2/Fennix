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

#include "../cmds.hpp"

#include <fs/vfs.hpp>
#include <acpi.hpp>

#include "../../kernel.h"

using namespace vfs;

void cmd_lsacpi(const char *)
{
	ACPI::ACPI *acpi = (ACPI::ACPI *)PowerManager->GetACPI();
	for (auto Table : acpi->Tables)
	{
		printf("%#lx: %.4s [%.6s:%.8s] %d bytes\n",
			   (uintptr_t)Table.second,
			   Table.second->Signature,
			   Table.second->OEMID,
			   Table.second->OEMTableID,
			   Table.second->Length);
	}
}
