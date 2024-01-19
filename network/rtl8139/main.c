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

#include <base.h>

#include "rtl8139.hpp"

int DriverEntry() { return cxx_Initialize(); }
int DriverFinal() { return cxx_Finalize(); }
int DriverPanic() { return cxx_Panic(); }
int DriverProbe() { return cxx_Probe(); }

DriverInfo("rtl8139",
		   "Realtek RTL8139 Network Driver",
		   "EnderIce2",
		   "0.1",
		   "GPLv3");
