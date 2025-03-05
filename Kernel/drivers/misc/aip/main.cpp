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

#if defined(__amd64__) || defined(__i386__)

#include <driver.hpp>
#include <interface/aip.h>
#include <cpu.hpp>
#include <pci.hpp>

#include "aip.hpp"

extern Driver::Manager *DriverManager;
extern PCI::Manager *PCIManager;
EXTERNC void KPrint(const char *Format, ...);
namespace Driver::AdvancedIntegratedPeripheral
{
	dev_t DriverID;

	bool IsATAPresent()
	{
		outb(0x1F0 + 2, 0);
		outb(0x1F0 + 3, 0);
		outb(0x1F0 + 4, 0);
		outb(0x1F0 + 5, 0);
		outb(0x1F0 + 7, 0xEC);
		if (inb(0x1F0 + 7) == 0 || inb(0x1F0 + 1) != 0)
			return false;
		return true;
	}

	bool IsKeyboard(uint8_t ID)
	{
		/* Common keyboard IDs */
		return ID == 0xAB || ID == 0xAC || ID == 0x5D ||
			   ID == 0x2B || ID == 0x47 || ID == 0x60;
	}

	bool IsMouse(uint8_t ID)
	{
		/* Common mouse IDs */
		return ID == 0x00 || ID == 0x03 || ID == 0x04;
	}

	const char *GetPS2DeviceName(uint8_t ID, uint8_t SubID)
	{
		switch (ID)
		{
		case 0x00:
			return "Standard PS/2 Mouse";
		case 0x03:
			return "Mouse with scroll wheel";
		case 0x04:
			return "Mouse 5 buttons";
		case 0xAB:
		{
			switch (SubID)
			{
			case 0x83: /* Normal */
			case 0x41: /* Translated */
			case 0xC1: /* Normal + Translated */
				return "Standard PS/2 Keyboard";
			case 0x84:
			case 0x54:
				return "IBM Thinkpad/Spacesaver Keyboard";
			case 0x85:
				return "NCD N-97/122-Key Host Connect(ed) Keyboard";
			case 0x86:
				return "122-Key Keyboard";
			case 0x90:
				return "Japanese \"G\" Keyboard";
			case 0x91:
				return "Japanese \"P\" Keyboard";
			case 0x92:
				return "Japanese \"A\" Keyboard";
			default:
				return "Unknown PS/2 Keyboard";
			}
		}
		case 0xAC:
		{
			switch (SubID)
			{
			case 0xA1:
				return "NCD Sun Keyboard";
			default:
				return "Unknown NCD Sun Keyboard";
			}
		}
		case 0x5D:
		case 0x2B:
			return "Trust Keyboard";
		case 0x47:
		case 0x60:
			return "NMB SGI Keyboard";
		default:
			return "Unknown PS/2 Device";
		}
	}

	uint8_t Device1ID[2] = {0x00, 0x00};
	uint8_t Device2ID[2] = {0x00, 0x00};
	bool DualChannel = false;
	bool ATAPresent = false;

	int Entry()
	{
		v0::PS2WriteCommand(DriverID, PS2_CMD_DISABLE_PORT_1);
		v0::PS2WriteCommand(DriverID, PS2_CMD_DISABLE_PORT_2);
		v0::PS2ClearOutputBuffer(DriverID);

		v0::PS2WriteCommand(DriverID, PS2_CMD_READ_CONFIG);
		PS2_CONFIGURATION cfg = {.Raw = v0::PS2ReadData(DriverID)};

		DualChannel = cfg.Port2Clock;
		if (DualChannel)
			trace("Dual channel PS/2 controller detected");
		cfg.Port1Interrupt = 1;
		cfg.Port2Interrupt = 1;
		cfg.Port1Translation = 1;

		v0::PS2WriteCommand(DriverID, PS2_CMD_WRITE_CONFIG);
		v0::PS2WriteData(DriverID, cfg.Raw);

		v0::PS2WriteCommand(DriverID, PS2_CMD_TEST_CONTROLLER);
		uint8_t test = v0::PS2ReadData(DriverID);
		if (test != PS2_TEST_PASSED)
		{
			trace("PS/2 controller self test failed (%#x)", test);
			return -EFAULT;
		}

		v0::PS2WriteCommand(DriverID, PS2_CMD_WRITE_CONFIG);
		v0::PS2WriteData(DriverID, cfg.Raw);

		// bool port2avail = false;
		// if (DualChannel)
		// {
		// 	v0::PS2WriteCommand(DriverID, PS2_CMD_ENABLE_PORT_1);
		// 	v0::PS2WriteCommand(DriverID, PS2_CMD_READ_CONFIG);
		// 	cfg.Raw = v0::PS2ReadData(DriverID);
		// 	port2avail = cfg.Port2Clock;
		// 	v0::PS2WriteCommand(DriverID, PS2_CMD_DISABLE_PORT_1);
		// }

		v0::PS2WriteCommand(DriverID, PS2_CMD_TEST_PORT_1);
		test = v0::PS2ReadData(DriverID);
		if (test != 0x00)
		{
			trace("PS/2 Port 1 self test failed (%#x)", test);
			return -EFAULT;
		}

		if (DualChannel)
		{
			v0::PS2WriteCommand(DriverID, PS2_CMD_TEST_PORT_2);
			test = v0::PS2ReadData(DriverID);
			if (test != 0x00)
			{
				trace("PS/2 Port 2 self test failed (%#x)", test);
				return -EFAULT;
			}
		}

		v0::PS2WriteCommand(DriverID, PS2_CMD_ENABLE_PORT_1);
		if (DualChannel)
			v0::PS2WriteCommand(DriverID, PS2_CMD_ENABLE_PORT_2);

		int errK = InitializeKeyboard();

		int errM = 0;
		if (DualChannel)
			errM = InitializeMouse();

		ATAPresent = IsATAPresent();

		if (errK != 0 && errM != 0 && ATAPresent == false)
			return -ENODEV;
		return 0;
	}

	int Final()
	{
		FinalizeKeyboard();
		FinalizeMouse();
		v0::PS2WriteCommand(DriverID, PS2_CMD_DISABLE_PORT_1);
		v0::PS2WriteCommand(DriverID, PS2_CMD_DISABLE_PORT_2);
		return 0;
	}

	int Panic()
	{
		v0::PS2WriteCommand(DriverID, PS2_CMD_DISABLE_PORT_1);
		v0::PS2WriteCommand(DriverID, PS2_CMD_DISABLE_PORT_2);
		return 0;
	}

	void __intStub() {}
	int Probe()
	{
		v0::RegisterInterruptHandler(DriverID, 1, (void *)__intStub);
		v0::RegisterInterruptHandler(DriverID, 12, (void *)__intStub);

		int kbd = DetectPS2Keyboard();
		int mouse = DetectPS2Mouse();
		int uart = DetectUART();

		v0::UnregisterAllInterruptHandlers(DriverID, (void *)__intStub);

		if (kbd != 0 && mouse != 0 && uart != 0)
			return -ENODEV;

		if (kbd == 0)
		{
			if (!IsKeyboard(Device1ID[0]))
			{
				trace("PS/2 Port 1 is not a keyboard");
				// return -EINVAL;
			}
		}

		if (mouse == 0)
		{
			if (!IsMouse(Device2ID[0]))
			{
				trace("PS/2 Port 2 is not a mouse");
				// return -EINVAL;
			}
		}

		KPrint("PS/2 Port 1: %s (0x%X 0x%X)",
			   GetPS2DeviceName(Device1ID[0], Device1ID[1]),
			   Device1ID[0], Device1ID[1]);
		KPrint("PS/2 Port 2: %s (0x%X 0x%X)",
			   GetPS2DeviceName(Device2ID[0], Device2ID[1]),
			   Device2ID[0], Device2ID[1]);
		return 0;
	}

	REGISTER_BUILTIN_DRIVER(aip,
							"Advanced Integrated Peripheral Driver",
							"enderice2",
							1, 0, 0,
							Entry,
							Final,
							Panic,
							Probe);
}

#endif
