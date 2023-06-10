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

#include <uart.hpp>
#include <printf.h>
#include <lock.hpp>

NewLock(DebuggerLock);

using namespace UniversalAsynchronousReceiverTransmitter;

static inline NIF void uart_wrapper(char c, void *unused)
{
    UART(COM1).Write(c);
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
