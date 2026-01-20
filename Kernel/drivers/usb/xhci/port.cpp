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

#include "xhci.hpp"

namespace Driver::ExtensibleHostControllerInterface
{
	extern dev_t DriverID;

	bool Port::IsPowered() { return Reg->PORTSC.PortPower(); }
	bool Port::IsEnabled() { return Reg->PORTSC.PortEnabledDisabled(); }
	bool Port::IsConnected() { return Reg->PORTSC.CurrentConnectStatus(); }
	bool Port::IsOverCurrent() { return Reg->PORTSC.OverCurrentActive(); }

	int Port::PowerOn()
	{
		if (IsPowered())
			return 0;

		Reg->PORTSC.PortPower(1);
		bool timeout = false;
		whileto(IsPowered() == 0, 50, timeout) v0::Sleep(DriverID, 1);

		if (timeout == false)
			return 0;

		error("Unable to power on port!");
		return ESTALE;
	}

	int Port::Reset()
	{
		int ret = PowerOn();
		if (ret != 0)
			return ret;

		Reg->PORTSC.ConnectStatusChange(1);
		Reg->PORTSC.PortEnabledDisabledChange(1);
		Reg->PORTSC.PortResetChange(1);

		bool timeout = false;
		if ((Proto->C.RevisionMajor() & 0xF) == 0x3)
		{
			Reg->PORTSC.WarmPortReset(1);
			whileto(Reg->PORTSC.WarmPortResetChange() == 0, 100, timeout) v0::Sleep(DriverID, 1);
		}
		else
		{
			Reg->PORTSC.PortReset(1);
			whileto(Reg->PORTSC.PortResetChange() == 0, 100, timeout) v0::Sleep(DriverID, 1);
		}

		if (timeout)
		{
			error("Timeout waiting for port reset!");
			return ETIMEDOUT;
		}
		v0::Sleep(DriverID, 10); /* give the hc a chance to settle */
		Reg->PORTSC.PortResetChange(1);
		Reg->PORTSC.WarmPortResetChange(1);
		Reg->PORTSC.ConnectStatusChange(1);
		Reg->PORTSC.PortEnabledDisabledChange(1);
		Reg->PORTSC.PortEnabledDisabled(0);
		v0::Sleep(DriverID, 10); /* give the hc a chance to settle */

		if (Reg->PORTSC.PortEnabledDisabled() == 0)
		{
			error("Unable to reset port!");
			return ENODEV;
		}
		return 0;
	}

	Port::Port(PortRegister *r, SupportedProtocolCapability *p) : Reg(r), Proto(p) {}
}
