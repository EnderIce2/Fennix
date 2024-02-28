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

#include <display.hpp>
#include <bitmap.hpp>
#include <convert.h>
#include <printf.h>
#include <lock.hpp>
#include <rand.hpp>
#include <uart.hpp>
#include <time.hpp>
#include <debug.h>
#include <smp.hpp>
#include <cpu.hpp>
#include <io.h>

#if defined(a64)
#include "../../arch/amd64/cpu/gdt.hpp"
#include "../arch/amd64/cpu/apic.hpp"
#elif defined(a32)
#include "../../arch/i386/cpu/gdt.hpp"
#include "../arch/i386/cpu/apic.hpp"
#elif defined(aa64)
#endif

#include "../../kernel.h"
#include "keyboard.hpp"

extern void ExPrint(const char *Format, ...);
extern Video::Font *CrashFont;
extern void DisplayTopOverlay();
extern CPU::ExceptionFrame *ExFrame;

/* TODO: Add more info */
struct DiagnosticFile
{
	__packed __aligned(16) struct
	{
		uint8_t Signature[5] = {'D', 'I', 'A', 'G', '\0'};
		uint32_t Version = 1;
		char Is64Bit = 0;
		uint32_t Length = 0;
	} Header;

	__packed __aligned(16) struct
	{
		CPU::ExceptionFrame Frame;

		uint32_t KernelMemoryLength;
		uint8_t KernelMemory[];
	} Data;
};

nsa bool WriteDiagDataToNode(vfs::RefNode *refFile)
{
	uintptr_t KStart = (uintptr_t)&_kernel_start;
	uintptr_t kEnd = (uintptr_t)&_kernel_end;
	size_t kSize = kEnd - KStart;

	size_t fileSize = sizeof(DiagnosticFile) + kSize;
	uint8_t *buf = (uint8_t *)KernelAllocator.RequestPages(TO_PAGES(fileSize));
	if (!buf)
	{
		ExPrint("\eFF0000Failed to allocate memory for diagnostic data\n");
		Display->UpdateBuffer();
		return false;
	}

	DiagnosticFile *file = (DiagnosticFile *)buf;
	file->Header = {};
	file->Header.Length = uint32_t(fileSize);
	file->Header.Is64Bit = sizeof(void *) == 8;

	file->Data.Frame = *ExFrame;
	file->Data.KernelMemoryLength = uint32_t(kSize);
	memcpy(file->Data.KernelMemory, (void *)KStart, kSize);

	ExPrint("\eFAFAFAWriting to %s\n", refFile->node->FullPath);
	size_t w = refFile->write(buf, fileSize);
	if (w != fileSize)
	{
		debug("%d out of %d bytes written", w, fileSize);
		ExPrint("\eFF0000Failed to write diagnostic data to file: %s\n",
				strerror((int)w));
		Display->UpdateBuffer();
		return false;
	}

	return true;
}

nsa void DiagnosticDataCollection()
{
	Display->ClearBuffer();
	DisplayTopOverlay();

	ExPrint("\n\eFAFAFAPlease wait while we collect some diagnostic information...\n");
	ExPrint("This may take a while...\n");

	vfs::Node *panicDir = fs->CreateIfNotExists("/var/panic", vfs::DIRECTORY);
	if (!panicDir)
	{
		ExPrint("\eFF0000Failed to create /var/panic\n");
		Display->UpdateBuffer();
		return;
	}

	Time::Clock clock = Time::ReadClock();
	char filename[64];
	for (int i = 0; i < INT32_MAX; i++)
	{
		sprintf(filename, "dump-%d-%d-%d-%d.dmp",
				clock.Year, clock.Month, clock.Day, i);
		if (fs->PathExists(filename, panicDir))
			continue;

		fs->Create(filename, vfs::FILE, panicDir);
		break;
	}

	vfs::RefNode *refFile = fs->Open(filename, panicDir);
	if (!WriteDiagDataToNode(refFile))
	{
		delete refFile;
		return;
	}

	ExPrint("You can find the diagnostic file in /var/panic/%s\n", filename);
	Display->UpdateBuffer();
	delete refFile;
}
