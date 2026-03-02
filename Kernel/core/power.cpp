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

#include <power.hpp>

#include <memory.hpp>
#include <acpi.hpp>
#include <debug.h>
#include <io.h>

#include "../kernel.h"

void KernelExit();

namespace Power
{
	void Power::UnloadKernelObjects()
	{
		klog("Unloading filesystems");
		if (fs)
			delete fs, fs = nullptr;

		klog("Unloading all drivers");
		if (DriverManager)
			DriverManager->UnloadAllDrivers();

		klog("Stopping scheduling");
		if (TaskManager && !TaskManager->IsPanic())
		{
			TaskManager->SignalShutdown();
			delete TaskManager, TaskManager = nullptr;
		}

		klog("Stopping USB Manager");
		if (usb)
			delete usb, usb = nullptr;

		KernelExit();
	}

	void Power::Reboot()
	{
		klog("Rebooting");

		UnloadKernelObjects();
		arch_reboot();
		CPU::Stop();
	}

	void Power::Shutdown()
	{
		klog("Shutting down");

		UnloadKernelObjects();
		arch_shutdown();
		CPU::Stop();
	}
}
