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

#include <driver.h>
#include <errno.h>

dev_t DriverID = -1;

extern "C" int DriverEntry();
extern "C" int DriverFinal();
extern "C" int DriverPanic();
extern "C" int DriverProbe();

typedef void (*CallPtr)(void);
extern "C" CallPtr __init_array_start[0], __init_array_end[0];
extern "C" CallPtr __fini_array_start[0], __fini_array_end[0];

extern "C" int _start(dev_t id)
{
	DriverID = id;

	for (CallPtr *func = __init_array_start; func != __init_array_end; func++)
		(*func)();

	return 0;
}

extern "C" int _final()
{
	int err = DriverFinal();

	for (CallPtr *func = __fini_array_start; func != __fini_array_end; func++)
		(*func)();

	return err;
}

/* ---------------------------------------------------------- */

#define KCALL extern "C" __attribute__((used))

#define KernelFunction(Name)                                         \
	KCALL long __##Name(dev_t id,                                    \
						long arg0 = 0, long arg1 = 0, long arg2 = 0, \
						long arg3 = 0, long arg4 = 0, long arg5 = 0);

#define DefineFunction(ReturnType, Name, ...) \
	KernelFunction(Name);                     \
	KCALL ReturnType Name(__VA_ARGS__)

#define DefineWrapper(Name)                                                           \
	KernelFunction(Name);                                                             \
	KCALL long Name(long arg0, long arg1, long arg2, long arg3, long arg4, long arg5) \
	{                                                                                 \
		return __##Name(DriverID, arg0, arg1, arg2, arg3, arg4, arg5);                \
	}

DefineFunction(void, KernelPrint, const char *format, ...)
{
	__builtin_va_list args;
	__builtin_va_start(args, format);
	__KernelPrint(DriverID, (long)format, (long)args);
	__builtin_va_end(args);
}

DefineFunction(void, KernelLog, const char *format, ...)
{
	__builtin_va_list args;
	__builtin_va_start(args, format);
	__KernelLog(DriverID, (long)format, (long)args);
	__builtin_va_end(args);
}

DefineWrapper(RegisterInterruptHandler);
DefineWrapper(OverrideInterruptHandler);
DefineWrapper(UnregisterInterruptHandler);
DefineWrapper(UnregisterAllInterruptHandlers);

DefineWrapper(AllocateMemory);
DefineWrapper(FreeMemory);
DefineWrapper(AppendMapFlag);
DefineWrapper(RemoveMapFlag);
DefineWrapper(MapPages);
DefineWrapper(UnmapPages);

DefineWrapper(MemoryCopy);
DefineWrapper(MemorySet);
DefineWrapper(MemoryMove);
DefineWrapper(StringLength);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wbuiltin-declaration-mismatch"
DefineWrapper(strstr);
#pragma GCC diagnostic pop

DefineWrapper(EnterCriticalSection);
DefineWrapper(LeaveCriticalSection);

DefineWrapper(CreateKernelProcess);
DefineWrapper(CreateKernelThread);
DefineWrapper(GetCurrentProcess);
DefineWrapper(KillProcess);
DefineWrapper(KillThread);
DefineWrapper(Yield);
DefineWrapper(Sleep);

DefineWrapper(RegisterFileSystem);
DefineWrapper(UnregisterFileSystem);

DefineWrapper(PIC_EOI);
DefineWrapper(IRQ_MASK);
DefineWrapper(IRQ_UNMASK);
DefineWrapper(PS2Wait);
DefineWrapper(PS2WriteCommand);
DefineWrapper(PS2WriteData);
DefineWrapper(PS2ReadData);
DefineWrapper(PS2ReadStatus);
DefineWrapper(PS2ReadAfterACK);
DefineWrapper(PS2ClearOutputBuffer);
DefineWrapper(PS2ACKTimeout);

DefineWrapper(RegisterDevice);
DefineWrapper(UnregisterDevice);

DefineWrapper(ReportInputEvent);

DefineWrapper(InitializePCI);
DefineWrapper(GetPCIDevices);
DefineWrapper(GetBAR);
DefineWrapper(iLine);
DefineWrapper(iPin);
