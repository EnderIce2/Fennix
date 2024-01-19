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

__driverAPI *API = NULL;

int RegisterInterruptHandler(uint8_t IRQ, void *Handler)
{
	if (Handler == NULL)
		return -EINVAL;

	return API->RegisterInterruptHandler(API->MajorID,
										 IRQ,
										 Handler);
}

int OverrideInterruptHandler(uint8_t IRQ, void *Handler)
{
	if (Handler == NULL)
		return -EINVAL;

	return API->OverrideInterruptHandler(API->MajorID,
										 IRQ,
										 Handler);
}

int UnregisterInterruptHandler(uint8_t IRQ, void *Handler)
{
	if (Handler == NULL)
		return -EINVAL;

	return API->UnregisterInterruptHandler(API->MajorID,
										   IRQ,
										   Handler);
}

int UnregisterAllInterruptHandlers(void *Handler)
{
	if (Handler == NULL)
		return -EINVAL;

	return API->UnregisterAllInterruptHandlers(API->MajorID,
											   Handler);
}

void *AllocateMemory(size_t Pages)
{
	if (Pages == 0)
		return NULL;

	return API->RequestPages(API->MajorID,
							 Pages);
}

void FreeMemory(void *Pointer, size_t Pages)
{
	if (Pointer == NULL || Pages == 0)
		return;

	API->FreePages(API->MajorID,
				   Pointer,
				   Pages);
}

void AppendMapFlag(void *Address, PageMapFlags Flag)
{
	API->AppendMapFlag(API->MajorID,
					   Address,
					   Flag);
}

void RemoveMapFlag(void *Address, PageMapFlags Flag)
{
	API->RemoveMapFlag(API->MajorID,
					   Address,
					   Flag);
}

void MapPages(void *PhysicalAddress, void *VirtualAddress, size_t Pages, uint32_t Flags)
{
	API->MapPages(API->MajorID,
				  PhysicalAddress,
				  VirtualAddress,
				  Pages,
				  Flags);
}

void UnmapPages(void *VirtualAddress, size_t Pages)
{
	API->UnmapPages(API->MajorID,
					VirtualAddress,
					Pages);
}

void KPrint(const char *Format, ...)
{
	va_list Args;
	va_start(Args, Format);

	API->KPrint(API->MajorID,
				Format,
				Args);

	va_end(Args);
}

void Log(const char *Format, ...)
{
	va_list Args;
	va_start(Args, Format);

	API->KernelLog(API->MajorID,
				   Format,
				   Args);

	va_end(Args);
}

CriticalState EnterCriticalSection()
{
	CriticalState cs;

#if defined(__i386__) || defined(__x86_64__)

	uintptr_t Flags;
#if defined(__x86_64__)
	asmv("pushfq");
	asmv("popq %0"
		 : "=r"(Flags));
#else
	asmv("pushfl");
	asmv("popl %0"
		 : "=r"(Flags));
#endif
	cs = Flags & (1 << 9);
	asmv("cli");

#elif defined(__arm__) || defined(__aarch64__)

	uintptr_t Flags;
	asmv("mrs %0, cpsr"
		 : "=r"(Flags));
	cs = Flags & (1 << 7);
	asmv("cpsid i");

#endif

	return cs;
}

void LeaveCriticalSection(CriticalState PreviousState)
{
#if defined(__i386__) || defined(__x86_64__)

	if (PreviousState)
		asmv("sti");

#elif defined(__arm__) || defined(__aarch64__)

	if (PreviousState)
		asmv("cpsie i");

#endif
}
