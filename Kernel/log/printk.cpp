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

#include <printf.h>
#include <atomic>

#include "../kernel.h"

namespace Log
{
	void UARTWrite(const Log::LogRecord *record);

#define EARLY_LOG_CAP 128
#define MAX_SINKS 8

	struct LogSink
	{
		void (*Write)(const LogRecord *);
		union
		{
			struct
			{
				uint32_t Active : 1;
				uint32_t Reserved : 31;
			};
			uint32_t raw;
		} Flags;
	};

	LogSink RegisteredSinks[MAX_SINKS];

	LogRecord EarlyLogBuffer[EARLY_LOG_CAP];
	std::atomic_uint32_t EarlyWriteIndex;
	std::atomic_uint64_t SequenceCounter;
	bool EarlyLogging = true;

	void AddRecord(LogRecord &Record)
	{
		if (EarlyLogging)
		{
			uint32_t index = EarlyWriteIndex.fetch_add(1) % EARLY_LOG_CAP;
			EarlyLogBuffer[index] = Record;
		}
		else
		{
			CPUData *cpu = GetCurrentCPU();
			uint32_t index = cpu->LogWriteIndex.fetch_add(1, std::memory_order_relaxed) % cpu->LogEntries;
			cpu->LogRecords[index] = Record;
		}
	}

	void Dispatch(const LogRecord &record)
	{
		for (size_t i = 0; i < MAX_SINKS; i++)
		{
			if (RegisteredSinks[i].Flags.Active == 0)
				continue;

			RegisteredSinks[i].Write(&record);
		}
	}

	int RegisterSink(void (*Write)(const LogRecord *))
	{
		for (size_t i = 0; i < MAX_SINKS; i++)
		{
			if (RegisteredSinks[i].Flags.Active == 1)
				continue;

			RegisteredSinks[i].Write = Write;
			RegisteredSinks[i].Flags.Active = 1;
			return EOK;
		}

		error("Failed to register log sink: no available slots.");
		return ENOMEM;
	}

	int UnregisterSink(void (*Write)(const LogRecord *))
	{
		for (size_t i = 0; i < MAX_SINKS; i++)
		{
			if (RegisteredSinks[i].Flags.Active == 0 || RegisteredSinks[i].Write != Write)
				continue;

			RegisteredSinks[i].Flags.Active = 0;
			return EOK;
		}

		error("Failed to unregister log sink: not found.");
		return ENOENT;
	}

	void printk(LogLevel Level, const char *File, int Line, const char *Function, const char *Format, ...)
	{
		char localBuffer[LOG_MESSAGE_MAX];

		va_list args;
		va_start(args, Format);
		int len = vsnprintf(localBuffer, sizeof(localBuffer), Format, args);
		va_end(args);

		/* FIXME: make sure that GetCurrentCPU and TimeManager->GetTimeNs won't create a call loop */

		LogRecord record;
		record.Sequence = SequenceCounter.fetch_add(1, std::memory_order_relaxed);
		record.TimestampNs = TimeManager ? TimeManager->GetTimeNs() : 0;
		record.CpuID = GetCurrentCPU()->ID;
		record.ThreadID = GetCurrentCPU()->CurrentThread ? GetCurrentCPU()->CurrentThread->ID : 0;
		record.Level = Level;
		record.File = File;
		record.Function = Function;
		record.Line = Line;
		record.MessageLength = (len < LOG_MESSAGE_MAX) ? len : (LOG_MESSAGE_MAX - 1);
		memcpy(record.Message, localBuffer, record.MessageLength);
		record.Message[record.MessageLength] = '\0';

		AddRecord(record);
		if (EarlyLogging)
			Dispatch(record);
	}

	void EarlyInit()
	{
		Log::RegisterSink(UARTWrite);
	}

	void MemoryInit()
	{
		CPUData *cpu = GetCurrentCPU();
		cpu->LogEntries = 256;
		cpu->LogRecords = new LogRecord[cpu->LogEntries];
		cpu->LogWriteIndex.store(0, std::memory_order_relaxed);

		static int MovedToHeap = 0; /* this is thread-safe, changed only on CPU 0 */
		if (!MovedToHeap++)
		{
			/* Flush early logs to the new buffers */
			for (size_t i = 0; i < EARLY_LOG_CAP; i++)
			{
				if (EarlyLogBuffer[i].MessageLength == 0)
					continue;

				AddRecord(EarlyLogBuffer[i]);
			}
		}
	}

	void LateInit()
	{
		// EarlyLogging = false;
	}
}
