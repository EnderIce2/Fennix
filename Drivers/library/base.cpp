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

#include <base.h>

#include <driver.h>
#include <errno.h>
#include <fs.h>

extern "C" dev_t DriverID;

#define KernelFunction(Name)                     \
	extern "C" __attribute__((naked, used)) long \
		__##Name(dev_t, long, long, long,        \
				 long, long, long) { return 0; }

void *operator new(__SIZE_TYPE__) { return (void *)1; }
void *operator new[](__SIZE_TYPE__) { return (void *)1; }
void operator delete(void *) {}
void operator delete[](void *) {}
void operator delete(void *, __SIZE_TYPE__) {}
void operator delete[](void *, __SIZE_TYPE__) {}

KernelFunction(KernelPrint);
KernelFunction(KernelLog);

KernelFunction(RegisterInterruptHandler);
KernelFunction(OverrideInterruptHandler);
KernelFunction(UnregisterInterruptHandler);
KernelFunction(UnregisterAllInterruptHandlers);

KernelFunction(AllocateMemory);
KernelFunction(FreeMemory);
KernelFunction(AppendMapFlag);
KernelFunction(RemoveMapFlag);
KernelFunction(MapPages);
KernelFunction(UnmapPages);

KernelFunction(MemoryCopy);
KernelFunction(MemorySet);
KernelFunction(MemoryMove);
KernelFunction(StringLength);
KernelFunction(strstr);

KernelFunction(EnterCriticalSection);
KernelFunction(LeaveCriticalSection);

KernelFunction(CreateKernelProcess);
KernelFunction(CreateKernelThread);
KernelFunction(GetCurrentProcess);
KernelFunction(KillProcess);
KernelFunction(KillThread);
KernelFunction(Yield);
KernelFunction(Sleep);

KernelFunction(RegisterFileSystem);
KernelFunction(UnregisterFileSystem);

KernelFunction(PIC_EOI);
KernelFunction(IRQ_MASK);
KernelFunction(IRQ_UNMASK);
KernelFunction(PS2Wait);
KernelFunction(PS2WriteCommand);
KernelFunction(PS2WriteData);
KernelFunction(PS2ReadData);
KernelFunction(PS2ReadStatus);
KernelFunction(PS2ReadAfterACK);
KernelFunction(PS2ClearOutputBuffer);
KernelFunction(PS2ACKTimeout);

KernelFunction(RegisterDevice);
KernelFunction(UnregisterDevice);

KernelFunction(ReportInputEvent);

KernelFunction(InitializePCI);
KernelFunction(GetPCIDevices);
KernelFunction(GetBAR);
KernelFunction(iLine);
KernelFunction(iPin);
