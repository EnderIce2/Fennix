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

#ifndef __FENNIX_KERNEL_POWER_H__
#define __FENNIX_KERNEL_POWER_H__

#include <types.h>

namespace Power
{
	class Power
	{
	private:
		void *acpi = nullptr;
		void *dsdt = nullptr;
		void *madt = nullptr;

	public:
		/**
		 * @brief Get Advanced Configuration and Power Interface. (Available only on x32 and x64)
		 *
		 * @return void* (ACPI::ACPI *)
		 */
		void *GetACPI() { return this->acpi; }

		/**
		 * @brief Get Differentiated System Description Table. (Available only on x32 and x64)
		 *
		 * @return void* (ACPI::DSDT *)
		 */
		void *GetDSDT() { return this->dsdt; }

		/**
		 * @brief Get Multiple APIC Description Table. (Available only on x32 and x64)
		 *
		 * @return void* (ACPI::MADT *)
		 */
		void *GetMADT() { return this->madt; }

		/**
		 * @brief Reboot the system.
		 */
		void Reboot();

		/**
		 * @brief Shutdown the system.
		 */
		void Shutdown();

		void InitDSDT();
		Power();
	};
}

#endif // !__FENNIX_KERNEL_POWER_H__
