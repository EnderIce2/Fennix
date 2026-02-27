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

#include <log.hpp>

#include "../kernel.h"

namespace Log
{
	void uart_wrapper(char c, void *)
	{
		uart.DebugWrite(c);
	}

	void UARTWrite(const Log::LogRecord *record)
	{
		const char *DbgLvlString;
		switch (record->Level)
		{
		case LogLevelError:
			DbgLvlString = "ERROR";
			break;
		case LogLevelWarning:
			DbgLvlString = "WARN ";
			break;
		case LogLevelInfo:
			DbgLvlString = "INFO ";
			break;
		case LogLevelDebug:
			DbgLvlString = "DEBUG";
			break;
		// case DebugLevelTrace:
		// 	DbgLvlString = "TRACE";
		// 	break;
		case LogLevelFixme:
			DbgLvlString = "FIXME";
			break;
		case LogLevelStub:
			fctprintf(uart_wrapper, nullptr, "STUB | %s>%s() is stub\n", record->File, record->Function);
			return;
		case LogLevelFunction:
			fctprintf(uart_wrapper, nullptr, "FUNC | %s>%s( ", record->File, record->Function);
			for (uint32_t i = 0; i < record->MessageLength; ++i)
				uart.DebugWrite(record->Message[i]);
			fctprintf(uart_wrapper, nullptr, " )\n");
			return;
		case LogLevelUbsan:
		{
			DbgLvlString = "UBSAN";
			fctprintf(uart_wrapper, nullptr, "%s| ", DbgLvlString);
			return;
		}
		default:
			DbgLvlString = "UNKNW";
			break;
		}
		fctprintf(uart_wrapper, nullptr, "%s| %s>%s:%d: ", DbgLvlString, record->File, record->Function, record->Line);

		for (uint32_t i = 0; i < record->MessageLength; ++i)
			uart.DebugWrite(record->Message[i]);
		uart.DebugWrite('\n');
	}
}
