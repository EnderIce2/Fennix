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

namespace Driver::FileAllocationTable
{
	dev_t DriverID;

	int Entry() { return 0; }
	int Final() { return 0; }
	int Panic() { return 0; }
	int Probe() { return 0; }

	REGISTER_BUILTIN_DRIVER(fat,
							"File Allocation Table Driver",
							"enderice2",
							1, 0, 0,
							Entry,
							Final,
							Panic,
							Probe);
}
