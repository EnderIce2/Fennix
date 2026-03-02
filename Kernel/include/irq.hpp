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

#pragma once

#include <types.h>
#include <pci.hpp>
#include <cpu.hpp>

namespace Interrupt
{
	enum IRQLines
	{
		/* IRQ */

		IRQ0,
		IRQ1,
		IRQ2,
		IRQ3,
		IRQ4,
		IRQ5,
		IRQ6,
		IRQ7,
		IRQ8,
		IRQ9,
		IRQ10,
		IRQ11,
		IRQ12,
		IRQ13,
		IRQ14,
		IRQ15,

		/* Reserved by OS */

		IRQ16, /* Reserved for multitasking */
		IRQ17,
		IRQ18,
		IRQ19,
		IRQ20,
		IRQ21,
		IRQ22,
		IRQ23,
		IRQ24,
		IRQ25,
		IRQ26,
		IRQ27,
		IRQ28,
		IRQ29,
		IRQ30,
		IRQ31, /* Halt core interrupt */

		/* Free */

		IRQ32,
		IRQ33,
		IRQ34,
		IRQ35,
		IRQ36,
		IRQ37,
		IRQ38,
		IRQ39,
		IRQ40,
		IRQ41,
		IRQ42,
		IRQ43,
		IRQ44,
		IRQ45,
		IRQ46,
		IRQ47,
		IRQ48,
		IRQ49,
		IRQ50,
		IRQ51,
		IRQ52,
		IRQ53,
		IRQ54,
		IRQ55,
		IRQ56,
		IRQ57,
		IRQ58,
		IRQ59,
		IRQ60,
		IRQ61,
		IRQ62,
		IRQ63,
		IRQ64,
		IRQ65,
		IRQ66,
		IRQ67,
		IRQ68,
		IRQ69,
		IRQ70,
		IRQ71,
		IRQ72,
		IRQ73,
		IRQ74,
		IRQ75,
		IRQ76,
		IRQ77,
		IRQ78,
		IRQ79,
		IRQ80,
		IRQ81,
		IRQ82,
		IRQ83,
		IRQ84,
		IRQ85,
		IRQ86,
		IRQ87,
		IRQ88,
		IRQ89,
		IRQ90,
		IRQ91,
		IRQ92,
		IRQ93,
		IRQ94,
		IRQ95,
		IRQ96,
		IRQ97,
		IRQ98,
		IRQ99,
		IRQ100,
		IRQ101,
		IRQ102,
		IRQ103,
		IRQ104,
		IRQ105,
		IRQ106,
		IRQ107,
		IRQ108,
		IRQ109,
		IRQ110,
		IRQ111,
		IRQ112,
		IRQ113,
		IRQ114,
		IRQ115,
		IRQ116,
		IRQ117,
		IRQ118,
		IRQ119,
		IRQ120,
		IRQ121,
		IRQ122,
		IRQ123,
		IRQ124,
		IRQ125,
		IRQ126,
		IRQ127,
		IRQ128,
		IRQ129,
		IRQ130,
		IRQ131,
		IRQ132,
		IRQ133,
		IRQ134,
		IRQ135,
		IRQ136,
		IRQ137,
		IRQ138,
		IRQ139,
		IRQ140,
		IRQ141,
		IRQ142,
		IRQ143,
		IRQ144,
		IRQ145,
		IRQ146,
		IRQ147,
		IRQ148,
		IRQ149,
		IRQ150,
		IRQ151,
		IRQ152,
		IRQ153,
		IRQ154,
		IRQ155,
		IRQ156,
		IRQ157,
		IRQ158,
		IRQ159,
		IRQ160,
		IRQ161,
		IRQ162,
		IRQ163,
		IRQ164,
		IRQ165,
		IRQ166,
		IRQ167,
		IRQ168,
		IRQ169,
		IRQ170,
		IRQ171,
		IRQ172,
		IRQ173,
		IRQ174,
		IRQ175,
		IRQ176,
		IRQ177,
		IRQ178,
		IRQ179,
		IRQ180,
		IRQ181,
		IRQ182,
		IRQ183,
		IRQ184,
		IRQ185,
		IRQ186,
		IRQ187,
		IRQ188,
		IRQ189,
		IRQ190,
		IRQ191,
		IRQ192,
		IRQ193,
		IRQ194,
		IRQ195,
		IRQ196,
		IRQ197,
		IRQ198,
		IRQ199,
		IRQ200,
		IRQ201,
		IRQ202,
		IRQ203,
		IRQ204,
		IRQ205,
		IRQ206,
		IRQ207,
		IRQ208,
		IRQ209,
		IRQ210,
		IRQ211,
		IRQ212,
		IRQ213,
		IRQ214,
		IRQ215,
		IRQ216,
		IRQ217,
		IRQ218,
		IRQ219,
		IRQ220,
		IRQ221,
		IRQ222,
		IRQ223,
		IRQ224,
		IRQ225,
		IRQ226,
		IRQ227,
		IRQ228,
		IRQ229,
		IRQ230,
		IRQ231,
		IRQ232,
		IRQ233,
		IRQ234,
		IRQ235,
		IRQ236,
		IRQ237,
		IRQ238,
		IRQ239,
		IRQ240,
		IRQ241,
		IRQ242,
		IRQ243,
		IRQ244,
		IRQ245,
		IRQ246,
		IRQ247,
		IRQ248,
		IRQ249,
		IRQ250,
		IRQ251,
		IRQ252,
		IRQ253,
		IRQ254,
		IRQ255
	};

	/**
	 * Line = IRQ
	 *
	 * Vector = Interrupt Number
	 *
	 * Example:
	 *   Line = 0
	 *   Vector = 32 (x86 IRQ0)
	 */
	class Controller
	{
	public:
		virtual void Mask(uint32_t Line) = 0;
		virtual void Unmask(uint32_t Line) = 0;

		virtual uint32_t TranslateToLine(uint32_t Vector) = 0;

		virtual void EOI(uint32_t Vector) = 0;

		virtual bool CanRoute() const { return false; }
		virtual void Route(uint32_t Vector, uint32_t Core) {}

		virtual ~Controller() = default;
	};

	typedef int (*Handle)(CPU::TrapFrame *);

	class Manager
	{
	private:
		Controller *ctrl;

		struct HandleData
		{
			Handle Callback;
			bool IsVirtualFunction;

			/**
			 * Context for the callback
			 *
			 * Used for C-style callbacks if the callback needs a context.
			 * (e.g. a driver)
			 */
			void *Context;
		};

		struct Line
		{
			fnx::spinlock_t lock;
			std::list<HandleData> Handlers;
		} RegisteredLines[256];

	public:
		decltype(ctrl) &ctl = ctrl;

		void RegisterHandler(Handle Callback, uint32_t InterruptLine, void *Context = nullptr, bool Virtual = false);
		void UnregisterHandler(Handle Callback, uint32_t InterruptLine);

		void UnregisterHandler(Handle Callback);
		void UnregisterHandler(uint32_t InterruptLine);
		void UnregisterAllHandlers();

		void Dispatch(CPU::TrapFrame *Frame);
		void Dispatch(CPU::SchedulerFrame *Frame);
		void Dispatch(CPU::ExceptionFrame *Frame);

		void Initialize(Controller &controller);
	};

	class Handler
	{
	private:
		int InterruptLine;

	protected:
		/**
		 * @brief Set a new interrupt number.
		 * @param InterruptLine The interrupt number. NOT the IRQ number! (IRQ0 != 32)
		 */
		void SetInterruptNumber(int InterruptLine) { this->InterruptLine = InterruptLine; }
		int GetInterruptNumber() { return this->InterruptLine; }

		/**
		 * @brief Create a new interrupt handler.
		 * @param InterruptLine The interrupt number. NOT the IRQ number! (IRQ0 != 32)
		 */
		Handler(int InterruptLine);
		Handler(PCI::PCIDevice Device) : Handler(((PCI::PCIHeader0 *)Device.Header)->InterruptLine) {}

		Handler() = delete;
		~Handler();

	public:
		/** EOK if is good, ENOTSUP if needs it needs to go for other handler */
		virtual int OnInterruptReceived(CPU::TrapFrame *Frame);
		virtual int OnInterruptReceived(CPU::SchedulerFrame *Frame);
	};
}
