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
#include <cpu.hpp>

namespace Driver::AdvancedIntegratedPeripheral
{
	extern uint8_t Device1ID[];
	extern uint8_t Device2ID[];

	void PS2KbdInterruptHandler(CPU::TrapFrame *);
	int InitializeKeyboard();
	int FinalizeKeyboard();
	int DetectPS2Keyboard();

	void PS2MouseInterruptHandler(CPU::TrapFrame *);
	int InitializeMouse();
	int FinalizeMouse();
	int DetectPS2Mouse();
	int DetectUART();

	void MasterInterruptHandler(CPU::TrapFrame *);
	void SlaveInterruptHandler(CPU::TrapFrame *);
	int InitializeATA();
	int FinalizeATA();
}
