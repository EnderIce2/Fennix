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

int DriverEntry()
{
	/** This is the main function of the driver.
	 * This function is called when the driver is loaded.
	 * This function should be used to initialize the PCI device
	 * and allocate resources.
	 */

	/* Print a message to the screen */
	KPrint("Hello World from Example Driver!");

	/* Print a message to the kernel log */
	Log("Hello World from Example Driver!");

	/* Print a message only if DEBUG is set */
	DebugLog("Hello World from Example Driver!");

	/* Return 0 to indicate success */
	return 0;
}

int DriverFinal()
{
	/** This function is called when the driver is unloaded.
	 * This function should be used to stop the PCI device and
	 * free any resources.
	 */

	return 0;
}

int DriverPanic()
{
	/** This function is called when the kernel panics.
	 * This function should be used to stop the driver from
	 * receiving interrupts or anything else that is not
	 * safe to do when the kernel panics.
	 */

	return 0;
}

int DriverProbe()
{
	/** This is the first function that is called when the
	 * driver is loaded.
	 * We can use this function to test if the driver is
	 * compatible with the hardware.
	 * Like if we have a specific PCI device or if we have
	 * a specific CPU feature.
	 *
	 * Return 0 if the driver is compatible with the hardware.
	 * Otherwise, we return a value from the errno.h header.
	 *
	 * Note: In this function you cannot use variables that
	 * have constructors or destructors. Before DriverEntry,
	 * the constructors are called and after DriverFinalize,
	 * the destructors are called.
	 */

	return 0;
}

DriverInfo("example",
		   "Example Driver",
		   "EnderIce2",
		   "0.1",
		   "GPLv3");
