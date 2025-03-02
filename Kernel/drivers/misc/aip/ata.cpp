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

#include <driver.hpp>
#include <cpu.hpp>
#include <pci.hpp>
#include <io.h>

extern Driver::Manager *DriverManager;
extern PCI::Manager *PCIManager;
namespace Driver::AdvancedIntegratedPeripheral
{
	extern dev_t DriverID;

	const struct InodeOperations opsATA = {
		.Lookup = nullptr,
		.Create = nullptr,
		.Remove = nullptr,
		.Rename = nullptr,
		.Read = nullptr,
		.Write = nullptr,
		.Truncate = nullptr,
		.Open = nullptr,
		.Close = nullptr,
		.Ioctl = nullptr,
		.ReadDir = nullptr,
		.MkDir = nullptr,
		.RmDir = nullptr,
		.SymLink = nullptr,
		.ReadLink = nullptr,
		.Seek = nullptr,
		.Stat = nullptr,
	};

	void MasterInterruptHandler(CPU::TrapFrame *)
	{
	}

	void SlaveInterruptHandler(CPU::TrapFrame *)
	{
	}

	int InitializeATA()
	{
		v0::RegisterInterruptHandler(DriverID, 14, (void *)MasterInterruptHandler);
		v0::RegisterInterruptHandler(DriverID, 15, (void *)SlaveInterruptHandler);
		return 0;
	}

	int FinalizeATA()
	{
		v0::UnregisterInterruptHandler(DriverID, 14, (void *)MasterInterruptHandler);
		v0::UnregisterInterruptHandler(DriverID, 15, (void *)SlaveInterruptHandler);
		return 0;
	}
}
