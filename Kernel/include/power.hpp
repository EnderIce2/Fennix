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

#pragma once

#include <types.h>

namespace Power
{
	class Power
	{
	private:
		void arch_init();
		void arch_reboot();
		void arch_shutdown();

		void UnloadKernelObjects();

	public:
		/** Reboot the system */
		void Reboot();

		/** Shutdown the system */
		void Shutdown();

		Power();
		~Power() = delete;
	};
}
