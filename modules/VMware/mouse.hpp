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

#ifndef __FENNIX_KERNEL_VMWARE_MOUSE_H__
#define __FENNIX_KERNEL_VMWARE_MOUSE_H__

#include <types.h>
#include "../../mapi.hpp"

namespace VMwareMouse
{
#define VMWARE_MAGIC 0x564D5868 /* hXMV */
#define VMWARE_PORT 0x5658

#define CMD_GETVERSION 0xA
#define CMD_ABSPOINTER_DATA 0x27
#define CMD_ABSPOINTER_STATUS 0x28
#define CMD_ABSPOINTER_COMMAND 0x29

#define ABSPOINTER_ENABLE 0x45414552 /* Q E A E */
#define ABSPOINTER_RELATIVE 0xF5
#define ABSPOINTER_ABSOLUTE 0x53424152 /* R A B S */

	typedef struct
	{
		union
		{
			uint32_t ax;
			uint32_t magic;
		};
		union
		{
			uint32_t bx;
			size_t size;
		};
		union
		{
			uint32_t cx;
			uint16_t command;
		};
		union
		{
			uint32_t dx;
			uint16_t port;
		};
		uint32_t si;
		uint32_t di;
	} VMwareCommand;

	int DriverEntry(void *);
	int CallbackHandler(KernelCallback *);
	int InterruptCallback(CPURegisters *);
}

#endif // !__FENNIX_KERNEL_VMWARE_MOUSE_H__
