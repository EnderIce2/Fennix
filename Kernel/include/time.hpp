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

#ifndef __FENNIX_KERNEL_TIME_H__
#define __FENNIX_KERNEL_TIME_H__

#include <types.h>
#include <debug.h>
#include <cassert>
#include <vector>

namespace Time
{
	class ITimer
	{
	protected:
		uint64_t ClassCreationTime = 0;

	public:
		virtual const char *Name() const = 0;
		virtual bool IsAvailable() const = 0;
		virtual bool SupportsNanoseconds() const = 0;
		virtual bool Sleep(uint64_t Nanoseconds) = 0;
		virtual uint64_t GetNanoseconds() = 0;
		virtual ~ITimer() = default;
	};

	struct Clock
	{
		int Year, Month, Day, Hour, Minute, Second;
		size_t Counter;
	};

	Clock ReadClock();
	Clock ConvertFromUnix(int Timestamp);

	inline uint64_t FromSeconds(uint64_t Seconds) { return Seconds * 1'000'000'000ULL; }
	inline uint64_t FromMilliseconds(uint64_t Milliseconds) { return Milliseconds * 1'000'000ULL; }

	inline uint64_t ToSeconds(uint64_t Nanoseconds) { return Nanoseconds / 1'000'000'000ULL; }
	inline uint64_t ToMilliseconds(uint64_t Nanoseconds) { return Nanoseconds / 1'000'000ULL; }

	class ProgrammableIntervalTimer : public ITimer
	{
	public:
		const char *Name() const override { return "PIT"; }
		bool IsAvailable() const override;
		bool SupportsNanoseconds() const override { return false; }

		bool Sleep(uint64_t Nanoseconds) override;
		uint64_t GetNanoseconds() override { return 0; }

		ProgrammableIntervalTimer();
		~ProgrammableIntervalTimer();
	};

	class RealTimeClock : public ITimer
	{
	public:
		const char *Name() const override { return "RTC"; }
		bool IsAvailable() const override;
		bool SupportsNanoseconds() const override { return false; }

		bool Sleep(uint64_t Nanoseconds) override;
		uint64_t GetNanoseconds() override { return 0; }

		RealTimeClock();
		~RealTimeClock();
	};

	class HighPrecisionEventTimer : public ITimer
	{
	private:
		struct HPET
		{
			uint64_t CapabilitiesID;
			uint64_t __reserved0;
			uint64_t Configuration;
			uint64_t __reserved1;
			uint64_t InterruptStatus;
			uint64_t __reserved2[25];
			uint64_t MainCounter;
			uint64_t __reserved3;
		};

		uint64_t Period = 0;
		HPET *hpet = nullptr;

	public:
		const char *Name() const override { return "HPET"; }
		bool IsAvailable() const override { return hpet != nullptr; }
		bool SupportsNanoseconds() const override { return true; }
		bool Sleep(uint64_t Nanoseconds) override;
		uint64_t GetNanoseconds() override;

		HighPrecisionEventTimer(void *hpet);
		~HighPrecisionEventTimer();
	};

	class TimeStampCounter : public ITimer
	{
	private:
		uint64_t clk = 0;

	public:
		const char *Name() const override { return "TSC"; }
		bool IsAvailable() const override { return clk != 0; }
		bool SupportsNanoseconds() const override { return true; }
		bool Sleep(uint64_t Nanoseconds) override;
		uint64_t GetNanoseconds() override;

		TimeStampCounter();
		~TimeStampCounter() = default;
	};

	class KVMClock : public ITimer
	{
	private:
		struct kvm_clock_pairing
		{
			int64_t sec;
			int64_t nsec;
			uint64_t tsc;
			uint32_t flags;
			uint32_t pad[9];
		};

		struct pvclock_vcpu_time_info
		{
			uint32_t version;
			uint32_t pad0;
			uint64_t tsc_timestamp;
			uint64_t system_time;
			uint32_t tsc_to_system_mul;
			int8_t tsc_shift;
			uint8_t flags;
			uint8_t pad[2];
		};

		struct ms_hyperv_tsc_page
		{
			volatile uint32_t tsc_sequence;
			uint32_t reserved1;
			volatile uint64_t tsc_scale;
			volatile int64_t tsc_offset;
			uint64_t reserved2[509];
		};

		uint64_t clk = 0;
		kvm_clock_pairing *Pairing = nullptr;

	public:
		const char *Name() const override { return "KVM"; }
		bool IsAvailable() const override { return clk != 0; }
		bool SupportsNanoseconds() const override { return true; }
		bool Sleep(uint64_t Nanoseconds) override;
		uint64_t GetNanoseconds() override;

		KVMClock();
		~KVMClock();
	};

	class Manager
	{
	private:
		void *acpi = nullptr;
		std::vector<ITimer *> Timers;
		int ActiveTimer = -1;

	public:
		void CheckActiveTimer();
		bool Sleep(uint64_t Nanoseconds);
		uint64_t GetTimeNs();
		const char *GetActiveTimerName();

		void InitializeTimers();
		Manager(void *acpi);
		~Manager() = delete;
	};
}

#endif // !__FENNIX_KERNEL_TIME_H__
