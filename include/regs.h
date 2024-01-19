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

#ifndef __FENNIX_API_REGISTERS_H__
#define __FENNIX_API_REGISTERS_H__

#include <types.h>

#if defined(__amd64__)
typedef struct
{
	uint64_t r15; // General purpose
	uint64_t r14; // General purpose
	uint64_t r13; // General purpose
	uint64_t r12; // General purpose
	uint64_t r11; // General purpose
	uint64_t r10; // General purpose
	uint64_t r9;  // General purpose
	uint64_t r8;  // General purpose

	uint64_t rbp; // Base Pointer (meant for stack frames)
	uint64_t rdi; // Destination index for string operations
	uint64_t rsi; // Source index for string operations
	uint64_t rdx; // Data (commonly extends the A register)
	uint64_t rcx; // Counter
	uint64_t rbx; // Base
	uint64_t rax; // Accumulator

	uint64_t InterruptNumber; // Interrupt Number
	uint64_t ErrorCode;		  // Error code

	uint64_t rip;	 // Instruction Pointer
	uint64_t cs;	 // Code Segment
	uint64_t rflags; // Register Flags
	uint64_t rsp;	 // Stack Pointer
	uint64_t ss;	 // Stack Segment
} TrapFrame;
#elif defined(__i386__)
typedef struct TrapFrame
{
	uint32_t edi; // Destination index for string operations
	uint32_t esi; // Source index for string operations
	uint32_t ebp; // Base Pointer (meant for stack frames)
	uint32_t esp; // Stack Pointer
	uint32_t ebx; // Base
	uint32_t edx; // Data (commonly extends the A register)
	uint32_t ecx; // Counter
	uint32_t eax; // Accumulator

	uint32_t InterruptNumber; // Interrupt Number
	uint32_t ErrorCode;		  // Error code

	uint32_t eip;	 // Instruction Pointer
	uint32_t cs;	 // Code Segment
	uint32_t eflags; // Register Flags

	uint32_t r3_esp; // Stack Pointer
	uint32_t r3_ss;	 // Stack Segment
} TrapFrame;
#elif defined(__aarch64__)
typedef struct TrapFrame
{
	uint64_t x19; // General purpose
	uint64_t x20; // General purpose
	uint64_t x21; // General purpose
	uint64_t x22; // General purpose
	uint64_t x23; // General purpose
	uint64_t x24; // General purpose
	uint64_t x25; // General purpose
	uint64_t x26; // General purpose

	uint64_t x27; // Stack frame pointer
	uint64_t x28; // Link register
	uint64_t x29; // Frame pointer
	uint64_t x30; // Program counter

	uint64_t sp_el0;				  // Stack pointer
	uint64_t elr_el1;				  // Exception Link Register
	uint64_t spsr_el1;				  // Saved Program Status Register
	uint64_t ErrorCode /* esr_el1 */; // Exception Syndrome Register

	uint64_t InterruptNumber /* iar_el1 */; // Interrupt Acknowledge Register
} TrapFrame;
#endif

#endif // !__FENNIX_API_REGISTERS_H__
