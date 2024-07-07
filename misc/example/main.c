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
	 * This function should be used to initialize the PCI device
	 * and allocate resources.
	 */

	/* Print a message to the kernel terminal */
	KernelPrint("Hello World from Example Driver!");

	/* Print a message to the kernel log */
	KernelLog("Hello World from Example Driver!");

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
	 * sending interrupts or anything else that is not
	 * safe to do when the kernel panics.
	 */

	return 0;
}

int DriverProbe()
{
	/** This is the first function that is called when the
	 * driver is loaded.
	 *
	 * This function is to test if the driver is compatible
	 * with the hardware.
	 * Example: Like if there is a PCI device that the driver
	 * is for, or a CPU feature that etc.
	 *
	 * Return 0 if the driver is compatible with the hardware.
	 * Otherwise, return a value that is not 0.
	 *
	 * Note: In this function you cannot use variables that
	 * have constructors or destructors. Before DriverEntry,
	 * the constructors are called and after DriverFinalize,
	 * the destructors are called.
	 */

	return 0;
}

/** DriverInfo() is a macro that is used to define the
 * driver's information.
 *
 * The parameters are:
 * - Name: Lowercase name
 * - Description: A short description
 * - Author: The author
 * - Version: The version
 * - License: The license
 */
DriverInfo("example",
		   "Example Driver",
		   "EnderIce2",
		   0, 0, 1,
		   "GPLv3");
