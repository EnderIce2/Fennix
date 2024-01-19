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

#ifndef __FENNIX_DRIVER_AIP_H__
#define __FENNIX_DRIVER_AIP_H__

#include <types.h>
#include <aip.h>
#include <regs.h>

extern uint8_t Device1ID[];
extern uint8_t Device2ID[];

void PS2KbdInterruptHandler(TrapFrame *);
int InitializeKeyboard();
int FinalizeKeyboard();
int DetectPS2Keyboard();

void PS2MouseInterruptHandler(TrapFrame *);
int InitializeMouse();
int FinalizeMouse();
int DetectPS2Mouse();

#endif // !__FENNIX_DRIVER_AIP_H__
