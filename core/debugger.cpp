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

#include <debug.h>

#include <printf.h>
#include <lock.hpp>
#include <io.h>

NewLock(DebuggerLock);

extern bool serialports[8];

static inline NIF void uart_wrapper(char c, void *unused)
{
	static int once = 0;
	if (unlikely(!once++))
	{
		uint8_t com = inb(0x3F8);
		if (com != 0xFF)
		{
			outb(s_cst(uint16_t, 0x3F8 + 1), 0x00); // Disable all interrupts
			outb(s_cst(uint16_t, 0x3F8 + 3), 0x80); // Enable DLAB (set baud rate divisor)
			outb(s_cst(uint16_t, 0x3F8 + 0), 0x1);	// Set divisor to 1 (lo byte) 115200 baud
			outb(s_cst(uint16_t, 0x3F8 + 1), 0x0);	//                  (hi byte)
			outb(s_cst(uint16_t, 0x3F8 + 3), 0x03); // 8 bits, no parity, one stop bit
			outb(s_cst(uint16_t, 0x3F8 + 2), 0xC7); // Enable FIFO, clear them, with 14-byte threshold
			outb(s_cst(uint16_t, 0x3F8 + 4), 0x0B); // IRQs enabled, RTS/DSR set

			/* FIXME  https://wiki.osdev.org/Serial_Ports */
			// outb(s_cst(uint16_t, 0x3F8 + 0), 0x1E);
			// outb(s_cst(uint16_t, 0x3F8 + 0), 0xAE);
			// Check if the serial port is faulty.
			// if (inb(s_cst(uint16_t, 0x3F8 + 0)) != 0xAE)
			// {
			//     static int once = 0;
			//     if (!once++)
			//         warn("Serial port %#llx is faulty.", 0x3F8);
			//     // serialports[0x3F8] = false; // ignore for now
			//     // return;
			// }

			// Set to normal operation mode.
			outb(s_cst(uint16_t, 0x3F8 + 4), 0x0F);
			serialports[0] = true;
		}
	}

	if (likely(serialports[0]))
	{
		while ((inb(s_cst(uint16_t, 0x3F8 + 5)) & 0x20) == 0)
			;
		outb(0x3F8, c);
	}
	UNUSED(unused);
}

static inline NIF bool WritePrefix(DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, va_list args)
{
	const char *DbgLvlString;
	switch (Level)
	{
	case DebugLevelError:
		DbgLvlString = "ERROR";
		break;
	case DebugLevelWarning:
		DbgLvlString = "WARN ";
		break;
	case DebugLevelInfo:
		DbgLvlString = "INFO ";
		break;
	case DebugLevelDebug:
		DbgLvlString = "DEBUG";
		break;
	case DebugLevelTrace:
		DbgLvlString = "TRACE";
		break;
	case DebugLevelFixme:
		DbgLvlString = "FIXME";
		break;
	case DebugLevelStub:
		fctprintf(uart_wrapper, nullptr, "STUB | %s>%s() is stub\n", File, Function);
		return false;
	case DebugLevelFunction:
		fctprintf(uart_wrapper, nullptr, "FUNC | %s>%s( ", File, Function);
		vfctprintf(uart_wrapper, nullptr, Format, args);
		fctprintf(uart_wrapper, nullptr, " )\n");
		return false;
	case DebugLevelUbsan:
	{
		DbgLvlString = "UBSAN";
		fctprintf(uart_wrapper, nullptr, "%s| ", DbgLvlString);
		return true;
	}
	default:
		DbgLvlString = "UNKNW";
		break;
	}
	fctprintf(uart_wrapper, nullptr, "%s| %s>%s:%d: ", DbgLvlString, File, Function, Line);
	return true;
}

namespace SysDbg
{
	NIF void Write(DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...)
	{
		va_list args;
		va_start(args, Format);
		if (!WritePrefix(Level, File, Line, Function, Format, args))
		{
			va_end(args);
			return;
		}
		vfctprintf(uart_wrapper, nullptr, Format, args);
		va_end(args);
	}

	NIF void WriteLine(DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...)
	{
		va_list args;
		va_start(args, Format);
		if (!WritePrefix(Level, File, Line, Function, Format, args))
		{
			va_end(args);
			return;
		}
		vfctprintf(uart_wrapper, nullptr, Format, args);
		va_end(args);
		uart_wrapper('\n', nullptr);
	}

	NIF void LockedWrite(DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...)
	{
		SmartTimeoutLock(DebuggerLock, 1000);
		va_list args;
		va_start(args, Format);
		if (!WritePrefix(Level, File, Line, Function, Format, args))
		{
			va_end(args);
			return;
		}
		vfctprintf(uart_wrapper, nullptr, Format, args);
		va_end(args);
	}

	NIF void LockedWriteLine(DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...)
	{
		SmartTimeoutLock(DebuggerLock, 1000);
		va_list args;
		va_start(args, Format);
		if (!WritePrefix(Level, File, Line, Function, Format, args))
		{
			va_end(args);
			return;
		}
		vfctprintf(uart_wrapper, nullptr, Format, args);
		va_end(args);
		uart_wrapper('\n', nullptr);
	}
}

// C compatibility
extern "C" NIF void SysDbgWrite(enum DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...)
{
	va_list args;
	va_start(args, Format);
	if (!WritePrefix(Level, File, Line, Function, Format, args))
	{
		va_end(args);
		return;
	}
	vfctprintf(uart_wrapper, nullptr, Format, args);
	va_end(args);
}

// C compatibility
extern "C" NIF void SysDbgWriteLine(enum DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...)
{
	va_list args;
	va_start(args, Format);
	if (!WritePrefix(Level, File, Line, Function, Format, args))
	{
		va_end(args);
		return;
	}
	vfctprintf(uart_wrapper, nullptr, Format, args);
	va_end(args);
	uart_wrapper('\n', nullptr);
}

// C compatibility
extern "C" NIF void SysDbgLockedWrite(enum DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...)
{
	SmartTimeoutLock(DebuggerLock, 1000);
	va_list args;
	va_start(args, Format);
	if (!WritePrefix(Level, File, Line, Function, Format, args))
	{
		va_end(args);
		return;
	}
	vfctprintf(uart_wrapper, nullptr, Format, args);
	va_end(args);
}

// C compatibility
extern "C" NIF void SysDbgLockedWriteLine(enum DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...)
{
	SmartTimeoutLock(DebuggerLock, 1000);
	va_list args;
	va_start(args, Format);
	if (!WritePrefix(Level, File, Line, Function, Format, args))
	{
		va_end(args);
		return;
	}
	vfctprintf(uart_wrapper, nullptr, Format, args);
	va_end(args);
	uart_wrapper('\n', nullptr);
}
