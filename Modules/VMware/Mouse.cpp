#include "mouse.hpp"

#include <debug.h>
#include <io.h>

#include "../../DAPI.hpp"
#include "../drv.hpp"
#include "../../kernel.h"

/* https://wiki.osdev.org/VMware_tools */

namespace VMwareMouse
{
	KernelAPI KAPI;

	void CommandSend(VMwareCommand *cmd)
	{
		cmd->magic = VMWARE_MAGIC;
		cmd->port = VMWARE_PORT;
		asm volatile("in %%dx, %0"
					 : "+a"(cmd->ax), "+b"(cmd->bx), "+c"(cmd->cx), "+d"(cmd->dx), "+S"(cmd->si), "+D"(cmd->di));
	}

	bool IsVMwareBackdoorAvailable(void)
	{
		VMwareCommand cmd;
		cmd.bx = ~VMWARE_MAGIC;
		cmd.command = CMD_GETVERSION;
		CommandSend(&cmd);
		if (cmd.bx != VMWARE_MAGIC || cmd.ax == 0xFFFFFFFF)
			return false;
		return true;
	}

	int DriverEntry(void *Data)
	{
		if (!Data)
			return INVALID_KERNEL_API;
		KAPI = *(KernelAPI *)Data;
		if (KAPI.Version.Major < 0 || KAPI.Version.Minor < 0 || KAPI.Version.Patch < 0)
			return KERNEL_API_VERSION_NOT_SUPPORTED;

		if (!IsVMwareBackdoorAvailable())
			return SYSTEM_NOT_SUPPORTED;

		return OK;
	}

	void Absolute(void)
	{
		VMwareCommand cmd;

		/* Enable */
		cmd.bx = ABSPOINTER_ENABLE;
		cmd.command = CMD_ABSPOINTER_COMMAND;
		CommandSend(&cmd);

		/* Status */
		cmd.bx = 0;
		cmd.command = CMD_ABSPOINTER_STATUS;
		CommandSend(&cmd);

		/* Read data (1) */
		cmd.bx = 1;
		cmd.command = CMD_ABSPOINTER_DATA;
		CommandSend(&cmd);

		/* Enable absolute */
		cmd.bx = ABSPOINTER_ABSOLUTE;
		cmd.command = CMD_ABSPOINTER_COMMAND;
		CommandSend(&cmd);
	}

	void Relative(void)
	{
		VMwareCommand cmd;
		cmd.bx = ABSPOINTER_RELATIVE;
		cmd.command = CMD_ABSPOINTER_COMMAND;
		CommandSend(&cmd);
	}

	enum Config
	{
		READ_CONFIG = 0x20,
		WRITE_CONFIG = 0x60
	};

	enum Ports
	{
		DATA = 0x60,
		STATUS = 0x64,
		COMMAND = 0x64,
	};

	enum State
	{
		OUTPUT_FULL = (1 << 0),
		INPUT_FULL = (1 << 1),
		MOUSE_BYTE = (1 << 5)
	};

	void WaitRead()
	{
		uint64_t Timeout = 100000;
		while (Timeout--)
			if (inb(Ports::STATUS) & State::OUTPUT_FULL)
				return;
	}

	void WaitWrite()
	{
		uint64_t Timeout = 100000;
		while (Timeout--)
			if ((inb(Ports::STATUS) & State::INPUT_FULL) == 0)
				return;
	}

	void Write(uint16_t Port, uint8_t Value)
	{
		WaitWrite();
		outb(Port, Value);
	}

	uint8_t Read()
	{
		WaitRead();
		return inb(Ports::DATA);
	}

	int MouseX = 0, MouseY = 0, MouseZ = 0;
	int MouseButton = 0;

	bool InputReceived = false;

	int CallbackHandler(KernelCallback *Data)
	{
		switch (Data->Reason)
		{
		case AcknowledgeReason:
		{
			debug("Kernel acknowledged the driver.");
			break;
		}
		case ConfigurationReason:
		{
			outb(COMMAND, 0xA8);
			Write(COMMAND, READ_CONFIG);
			uint8_t Status = Read();
			Status |= 0b10;
			Write(COMMAND, WRITE_CONFIG);
			Write(DATA, Status);
			Write(COMMAND, 0xD4);
			Write(DATA, 0xF6);
			Read();
			Write(COMMAND, 0xD4);
			Write(DATA, 0xF4);
			Read();
			Absolute();
			trace("VMware mouse configured.");
			break;
		}
		case QueryReason:
		{
			Data->InputCallback.Mouse.X = (MouseX * KAPI.Display.GetWidth()) / 0xFFFF;
			Data->InputCallback.Mouse.Y = (MouseY * KAPI.Display.GetHeight()) / 0xFFFF;
			Data->InputCallback.Mouse.Z = MouseZ;
			Data->InputCallback.Mouse.Buttons.Left = MouseButton & 0x20;
			Data->InputCallback.Mouse.Buttons.Right = MouseButton & 0x10;
			Data->InputCallback.Mouse.Buttons.Middle = MouseButton & 0x08;
			break;
		}
		case PollWaitReason:
		{
			while (!InputReceived)
				TaskManager->Yield();
			InputReceived = false;

			Data->InputCallback.Mouse.X = (MouseX * KAPI.Display.GetWidth()) / 0xFFFF;
			Data->InputCallback.Mouse.Y = (MouseY * KAPI.Display.GetHeight()) / 0xFFFF;
			Data->InputCallback.Mouse.Z = MouseZ;
			Data->InputCallback.Mouse.Buttons.Left = MouseButton & 0x20;
			Data->InputCallback.Mouse.Buttons.Right = MouseButton & 0x10;
			Data->InputCallback.Mouse.Buttons.Middle = MouseButton & 0x08;
			break;
		}
		case StopReason:
		{
			Relative();
			// TODO: UNTESTED!!!
			outb(COMMAND, 0xA8);
			Write(COMMAND, READ_CONFIG);
			uint8_t Status = Read();
			Status &= ~0b10;
			Write(COMMAND, WRITE_CONFIG);
			Write(DATA, Status);
			Write(COMMAND, 0xD4);
			Write(DATA, 0xF5);
			Read();
			debug("Driver stopped.");
			break;
		}
		default:
		{
			warn("Unknown reason.");
			break;
		}
		}
		return OK;
	}

	int InterruptCallback(CPURegisters *)
	{
		uint8_t Data = inb(0x60);
		(void)Data;
		VMwareCommand cmd;
		cmd.bx = 0;
		cmd.command = CMD_ABSPOINTER_STATUS;
		CommandSend(&cmd);

		if (cmd.ax == 0xFFFF0000)
		{
			warn("VMware mouse is not connected?");
			Relative();
			Absolute();
			return ERROR;
		}
		if ((cmd.ax & 0xFFFF) < 4)
			return ERROR;

		cmd.bx = 4;
		cmd.command = CMD_ABSPOINTER_DATA;
		CommandSend(&cmd);

		int flags = (cmd.ax & 0xFFFF0000) >> 16; /* Not important */
		(void)flags;
		MouseButton = (cmd.ax & 0xFFFF); /* 0x10 = Right, 0x20 = Left, 0x08 = Middle */
		MouseX = cmd.bx;                 /* Both X and Y are scaled from 0 to 0xFFFF */
		MouseY = cmd.cx;                 /* You should map these somewhere to the actual resolution. */
		MouseZ = (int8_t)cmd.dx;         /* Z is a single signed byte indicating scroll direction. */
		InputReceived = true;
		return OK;
	}
}
