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

#ifndef __FENNIX_KERNEL_APIC_H__
#define __FENNIX_KERNEL_APIC_H__

#include <types.h>

#include <ints.hpp>
#include <cpu.hpp>

namespace APIC
{
	enum APICRegisters
	{
		/* APIC ID Register */
		APIC_ID = 0x20,
		/* APIC Version Register */
		APIC_VER = 0x30,
		/* Task Priority Register (TPR) */
		APIC_TPR = 0x80,
		/* Arbitration Priority Register (APR) */
		APIC_APR = 0x90,
		/* Processor Priority Register (PPR) */
		APIC_PPR = 0xA0,
		/* End of Interrupt Register (EOI) */
		APIC_EOI = 0xB0,
		/* Remote Read Register */
		APIC_RRD = 0xC0,
		/* Logical Destination Register (LDR) */
		APIC_LDR = 0xD0,
		/* Destination Format Register (DFR) */
		APIC_DFR = 0xE0,
		/* Spurious Interrupt Vector Register */
		APIC_SVR = 0xF0,
		/* In-Service Register (ISR) */
		APIC_ISR = 0x100,
		/* Trigger Mode Register (TMR) */
		APIC_TMR = 0x180,
		/* Interrupt Request Register (IRR) */
		APIC_IRR = 0x200,
		/* Error Status Register (ESR) */
		APIC_ESR = 0x280,
		/* Interrupt Command Register Low (bits 31:0) */
		APIC_ICRLO = 0x300,
		/* Interrupt Command Register High (bits 63:32) */
		APIC_ICRHI = 0x310,
		/* Timer Local Vector Table Entry */
		APIC_TIMER = 0x320,
		/* Thermal Local Vector Table Entry */
		APIC_THERMAL = 0x330,
		/* Performance Counter Local Vector Table Entry */
		APIC_PERF = 0x340,
		/* Local Interrupt 0 Vector Table Entry */
		APIC_LINT0 = 0x350,
		/* Local Interrupt 1 Vector Table Entry */
		APIC_LINT1 = 0x360,
		/* Error Vector Table Entry */
		APIC_ERROR = 0x370,
		/* Timer Initial Count Register */
		APIC_TICR = 0x380,
		/* Timer Current Count Register */
		APIC_TCCR = 0x390,
		/* Timer Divide Configuration Register */
		APIC_TDCR = 0x3E0,
		/* Extended APIC Feature Register */
		APIC_EFR = 0x400,
		/* Extended APIC Control Register */
		APIC_ECR = 0x410,
		/* Specific End of Interrupt Register (SEOI) */
		APIC_SEOI = 0x420,
		/* Interrupt Enable Registers (IER) */
		APIC_IER0 = 0x480,
		/* Extended Interrupt [3:0] Local Vector Table Registers */
		APIC_EILVT0 = 0x500,
	};

	enum IOAPICRegisters
	{
		GetIOAPICVersion = 0x1
	};

	enum IOAPICFlags
	{
		ActiveHighLow = 2,
		EdgeLevel = 8
	};

	enum APICMessageType
	{
		Fixed = 0b000,
		LowestPriority = 0b001, /* Reserved */
		SMI = 0b010,
		DeliveryMode = 0b011, /* Reserved */
		NMI = 0b100,
		INIT = 0b101,
		Startup = 0b110,
		ExtINT = 0b111 /* Reserved */
	};

	enum APICDestinationMode
	{
		Physical = 0b0,
		Logical = 0b1
	};

	enum APICDeliveryStatus
	{
		Idle = 0b0,
		SendPending = 0b1
	};

	enum APICLevel
	{
		DeAssert = 0b0,
		Assert = 0b1
	};

	enum APICTriggerMode
	{
		Edge = 0b0,
		Level = 0b1
	};

	enum APICDestinationShorthand
	{
		NoShorthand = 0b00,
		Self = 0b01,
		AllIncludingSelf = 0b10,
		AllExcludingSelf = 0b11
	};

	enum LVTTimerDivide
	{
		DivideBy2 = 0b000,
		DivideBy4 = 0b001,
		DivideBy8 = 0b010,
		DivideBy16 = 0b011,
		DivideBy32 = 0b100,
		DivideBy64 = 0b101,
		DivideBy128 = 0b110,
		DivideBy1 = 0b111
	};

	enum LVTTimerMask
	{
		Unmasked = 0b0,
		Masked = 0b1
	};

	enum LVTTimerMode
	{
		OneShot = 0b00,
		Periodic = 0b01,
		TSCDeadline = 0b10
	};

	typedef union
	{
		struct
		{
			/** Vector */
			uint64_t VEC : 8;
			/** Reserved */
			uint64_t Reserved0 : 4;
			/** Delivery Status */
			uint64_t DS : 1;
			/** Reserved */
			uint64_t Reserved1 : 3;
			/** Mask */
			uint64_t M : 1;
			/** Timer Mode */
			uint64_t TMM : 1;
			/** Reserved */
			uint64_t Reserved2 : 14;
		};
		uint32_t raw;
	} __packed LVTTimer;

	typedef union
	{
		struct
		{
			/** Vector */
			uint64_t VEC : 8;
			/** APIC Software Enable */
			uint64_t ASE : 1;
			/** Focus CPU Core Checking */
			uint64_t FCC : 1;
			/** Reserved */
			uint64_t Reserved0 : 22;
		};
		uint32_t raw;
	} __packed Spurious;

	typedef union
	{
		struct
		{
			/** Vector */
			uint64_t VEC : 8;
			/** Message Type */
			uint64_t MT : 3;
			/** Destination Mode */
			uint64_t DM : 1;
			/** Delivery Status */
			uint64_t DS : 1;
			/** Reserved */
			uint64_t Reserved0 : 1;
			/** Level */
			uint64_t L : 1;
			/** Trigger Mode */
			uint64_t TGM : 1;
			/** Remote Read Status */
			uint64_t RSS : 2;
			/** Destination Shorthand */
			uint64_t DSH : 2;
			/** Reserved */
			uint64_t Reserved2 : 36;
			/** Destination */
			uint64_t DES : 8;
		};
		struct
		{
			/** Vector */
			uint64_t VEC : 8;
			/** Message Type */
			uint64_t MT : 3;
			/** Destination Mode */
			uint64_t DM : 1;
			/** Reserved */
			uint64_t Reserved0 : 2;
			/** Level */
			uint64_t L : 1;
			/** Trigger Mode */
			uint64_t TGM : 1;
			/** Reserved */
			uint64_t Reserved1 : 2;
			/** Destination Shorthand */
			uint64_t DSH : 2;
			/** Reserved */
			uint64_t Reserved2 : 12;
			/** Destination */
			uint64_t DES : 32;
		} x2;
		struct
		{
			uint32_t Low;
			uint32_t High;
		} split;
		uint64_t raw;
	} __packed InterruptCommandRegister;

	typedef union
	{
		struct
		{
			/** Reserved */
			uint64_t Reserved0 : 2;
			/** Sent Accept Error */
			uint64_t SAE : 1;
			/** Receive Accept Error */
			uint64_t RAE : 1;
			/** Reserved */
			uint64_t Reserved1 : 1;
			/** Sent Illegal Vector */
			uint64_t SIV : 1;
			/** Received Illegal Vector */
			uint64_t RIV : 1;
			/** Illegal Register Address */
			uint64_t IRA : 1;
			/** Reserved */
			uint64_t Reserved2 : 24;
		};
		uint32_t raw;
	} ErrorStatusRegister;

	typedef union
	{
		struct
		{
			/** Interrupt Vector */
			uint64_t VEC : 8;
			/** Delivery Mode */
			uint64_t MT : 3;
			/** Destination Mode */
			uint64_t DM : 1;
			/** Delivery Status */
			uint64_t DS : 1;
			/** Interrupt Input Pin Polarity */
			uint64_t IPP : 1;
			/** Remote IRR */
			uint64_t RIR : 1;
			/** Trigger Mode */
			uint64_t TGM : 1;
			/** Mask */
			uint64_t M : 1;
			/** Reserved */
			uint64_t Reserved0 : 15;
			/** Reserved */
			uint64_t Reserved1 : 24;
			/** Destination */
			uint64_t DES : 8;
		};
		struct
		{
			uint32_t Low;
			uint32_t High;
		} split;
		uint64_t raw;
	} __packed IOAPICRedirectEntry;

	typedef union
	{
		struct
		{
			/** Version */
			uint64_t VER : 8;
			/** Reserved */
			uint64_t Reserved0 : 8;
			/** Max LVT Entries */
			uint64_t MLE : 8;
			/** Reserved */
			uint64_t Reserved1 : 7;
			/** Extended APIC Register Space Present */
			uint64_t EAS : 1;
		};
		uint32_t raw;
	} __packed IOAPICVersion;

	class APIC
	{
	private:
		bool x2APICSupported = false;
		uint64_t APICBaseAddress = 0;

	public:
		decltype(x2APICSupported) &x2APIC = x2APICSupported;

		uint32_t Read(uint32_t Register);
		void Write(uint32_t Register, uint32_t Value);
		void IOWrite(uint64_t Base, uint32_t Register, uint32_t Value);
		uint32_t IORead(uint64_t Base, uint32_t Register);
		void EOI();
		void RedirectIRQs(uint8_t CPU = 0);
		void WaitForIPI();
		void ICR(InterruptCommandRegister icr);
		void SendInitIPI(int CPU);
		void SendStartupIPI(int CPU, uint64_t StartupAddress);
		uint32_t IOGetMaxRedirect(uint32_t APICID);
		void RawRedirectIRQ(uint8_t Vector, uint32_t GSI, uint16_t Flags, uint8_t CPU, int Status);
		void RedirectIRQ(uint8_t CPU, uint8_t IRQ, int Status);
		APIC(int Core);
		~APIC();
	};

	class Timer : public Interrupts::Handler
	{
	private:
		APIC *lapic;
		uint64_t Ticks = 0;
		int OnInterruptReceived(CPU::TrapFrame *Frame);

	public:
		uint64_t GetTicks() { return Ticks; }
		void OneShot(uint32_t Vector, uint64_t Miliseconds);
		Timer(APIC *apic);
		~Timer();
	};
}

#endif // !__FENNIX_KERNEL_APIC_H__
