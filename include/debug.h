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

#ifndef __FENNIX_KERNEL_DEBUGGER_H__
#define __FENNIX_KERNEL_DEBUGGER_H__

#include <types.h>

enum DebugLevel
{
	DebugLevelNone = 0,
	DebugLevelError,
	DebugLevelWarning,
	DebugLevelInfo,
	DebugLevelDebug,
	DebugLevelTrace,
	DebugLevelFixme,
	DebugLevelUbsan,
	DebugLevelStub,
	DebugLevelFunction,
};

#ifdef __cplusplus

namespace SysDbg
{
	void Write(DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...);
	void WriteLine(DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...);
	void LockedWrite(DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...);
	void LockedWriteLine(DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...);
}

#define error(Format, ...) SysDbg::WriteLine(DebugLevelError, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define warn(Format, ...) SysDbg::WriteLine(DebugLevelWarning, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define info(Format, ...) SysDbg::WriteLine(DebugLevelInfo, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#ifdef DEBUG
#define debug(Format, ...) SysDbg::WriteLine(DebugLevelDebug, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define ubsan(Format, ...) SysDbg::WriteLine(DebugLevelUbsan, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define function(Format, ...) SysDbg::WriteLine(DebugLevelFunction, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#else
#define debug(Format, ...)
#define ubsan(Format, ...)
#define function(Format, ...)
#endif
#define trace(Format, ...) SysDbg::WriteLine(DebugLevelTrace, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define fixme(Format, ...) SysDbg::WriteLine(DebugLevelFixme, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define stub SysDbg::WriteLine(DebugLevelStub, __FILE__, __LINE__, __FUNCTION__, "stub")

#define l_error(Format, ...) SysDbg::LockedWriteLine(DebugLevelError, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define l_warn(Format, ...) SysDbg::LockedWriteLine(DebugLevelWarning, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define l_info(Format, ...) SysDbg::LockedWriteLine(DebugLevelInfo, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#ifdef DEBUG
#define l_debug(Format, ...) SysDbg::LockedWriteLine(DebugLevelDebug, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define l_ubsan(Format, ...) SysDbg::LockedWriteLine(DebugLevelUbsan, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define l_function(Format, ...) SysDbg::LockedWriteLine(DebugLevelFunction, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#else
#define l_debug(Format, ...)
#define l_ubsan(Format, ...)
#define l_function(Format, ...)
#endif
#define l_trace(Format, ...) SysDbg::LockedWriteLine(DebugLevelTrace, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define l_fixme(Format, ...) SysDbg::LockedWriteLine(DebugLevelFixme, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define l_stub SysDbg::LockedWriteLine(DebugLevelStub, __FILE__, __LINE__, __FUNCTION__, "stub")

#else

void SysDbgWrite(enum DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...);
void SysDbgWriteLine(enum DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...);
void SysDbgLockedWrite(enum DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...);
void SysDbgLockedWriteLine(enum DebugLevel Level, const char *File, int Line, const char *Function, const char *Format, ...);

#define error(Format, ...) SysDbgWriteLine(DebugLevelError, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define warn(Format, ...) SysDbgWriteLine(DebugLevelWarning, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define info(Format, ...) SysDbgWriteLine(DebugLevelInfo, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#ifdef DEBUG
#define debug(Format, ...) SysDbgWriteLine(DebugLevelDebug, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define ubsan(Format, ...) SysDbgWriteLine(DebugLevelUbsan, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define function(Format, ...) SysDbgWriteLine(DebugLevelFunction, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#else
#define debug(Format, ...)
#define ubsan(Format, ...)
#define function(Format, ...)
#endif
#define trace(Format, ...) SysDbgWriteLine(DebugLevelTrace, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define fixme(Format, ...) SysDbgWriteLine(DebugLevelFixme, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define stub SysDbgWriteLine(DebugLevelStub, __FILE__, __LINE__, __FUNCTION__, "stub")

#define l_error(Format, ...) SysDbgLockedWriteLine(DebugLevelError, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define l_warn(Format, ...) SysDbgLockedWriteLine(DebugLevelWarning, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define l_info(Format, ...) SysDbgLockedWriteLine(DebugLevelInfo, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#ifdef DEBUG
#define l_debug(Format, ...) SysDbgLockedWriteLine(DebugLevelDebug, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define l_ubsan(Format, ...) SysDbgLockedWriteLine(DebugLevelUbsan, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define l_function(Format, ...) SysDbgLockedWriteLine(DebugLevelFunction, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#else
#define l_debug(Format, ...)
#define l_ubsan(Format, ...)
#define l_function(Format, ...)
#endif
#define l_trace(Format, ...) SysDbgLockedWriteLine(DebugLevelTrace, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define l_fixme(Format, ...) SysDbgLockedWriteLine(DebugLevelFixme, __FILE__, __LINE__, __FUNCTION__, Format, ##__VA_ARGS__)
#define l_stub SysDbgLockedWriteLine(DebugLevelStub, __FILE__, __LINE__, __FUNCTION__, "stub")

#endif // __cplusplus

EXTERNC void uart_wrapper(char c, void *unused);

#endif // !__FENNIX_KERNEL_DEBUGGER_H__
