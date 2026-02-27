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

EXTERNC void __c_kprint_error(const char *File, int Line, const char *Function, const char *Format, ...)
{
	va_list args;
	va_start(args, Format);
	Log::printk(Log::LogLevel::LogLevelError, File, Line, Function, Format, args);
	va_end(args);
}

EXTERNC void __c_kprint_warning(const char *File, int Line, const char *Function, const char *Format, ...)
{
	va_list args;
	va_start(args, Format);
	Log::printk(Log::LogLevel::LogLevelWarning, File, Line, Function, Format, args);
	va_end(args);
}

EXTERNC void __c_kprint_info(const char *File, int Line, const char *Function, const char *Format, ...)
{
	va_list args;
	va_start(args, Format);
	Log::printk(Log::LogLevel::LogLevelInfo, File, Line, Function, Format, args);
	va_end(args);
}

EXTERNC void __c_kprint_debug(const char *File, int Line, const char *Function, const char *Format, ...)
{
	va_list args;
	va_start(args, Format);
	Log::printk(Log::LogLevel::LogLevelDebug, File, Line, Function, Format, args);
	va_end(args);
}

EXTERNC void __c_kprint_ubsan(const char *File, int Line, const char *Function, const char *Format, ...)
{
	va_list args;
	va_start(args, Format);
	Log::printk(Log::LogLevel::LogLevelUbsan, File, Line, Function, Format, args);
	va_end(args);
}

EXTERNC void __c_kprint_func(const char *File, int Line, const char *Function, const char *Format, ...)
{
	va_list args;
	va_start(args, Format);
	Log::printk(Log::LogLevel::LogLevelFunction, File, Line, Function, Format, args);
	va_end(args);
}

EXTERNC void __c_kprint_fixme(const char *File, int Line, const char *Function, const char *Format, ...)
{
	va_list args;
	va_start(args, Format);
	Log::printk(Log::LogLevel::LogLevelFixme, File, Line, Function, Format, args);
	va_end(args);
}

EXTERNC void __c_kprint_stub(const char *File, int Line, const char *Function)
{
	Log::printk(Log::LogLevel::LogLevelStub, File, Line, Function, "stub");
}
