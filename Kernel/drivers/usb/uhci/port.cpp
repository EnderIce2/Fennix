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

#include "uhci.hpp"

namespace Driver::UniversalHostControllerInterface
{
	extern dev_t DriverID;

	PORTSC Port::Status()
	{
		PORTSC sc = inw(PortIO);

		/* This MUST be 1; otherwise something is wrong */
		assert(sc & PORT_ALWAYS_ONE);
		return sc;
	}

	int Port::Clear(PORTSC Change)
	{
		PORTSC sc = Status();
		sc &= ~Change;
		sc &= ~(PORT_CSC | PORT_PEC);
		sc |= (PORT_CSC | PORT_PEC) & Change;
		outw(PortIO, sc);
		return 0;
	}

	int Port::Set(PORTSC Change)
	{
		PORTSC sc = Status();
		sc |= Change;
		sc &= ~(PORT_CSC | PORT_PEC);
		outw(PortIO, sc);
		return 0;
	}

	int Port::Reset()
	{
		Set(PORT_PR);
		v0::Sleep(DriverID, 100);
		Clear(PORT_PR);

		while (true)
		{
			v0::Sleep(DriverID, 50);
			PORTSC sc = Status();

			if (~sc & PORT_CCS)
			{
				debug("device is not present");
				return -ENODEV;
			}

			if (sc & (PORT_CSC | PORT_PEC))
			{
				debug("status changed");
				Clear(PORT_CSC | PORT_PEC);
				continue;
			}

			if (sc & PORT_PE)
			{
				debug("port is enabled");
				break;
			}

			Set(PORT_PE);
		}
		return 0;
	}

	int Port::Probe()
	{
		Reset();
		PORTSC sc = Status();

		if (sc & PORT_PE)
		{
			Speed = (sc & PORT_LSDA) == 1 ? USB_LOW_SPEED : USB_FULL_SPEED;
			return 0;
		}

		debug("port is not enabled");
		return -ENODEV;
	}

	Port::Port(uint16_t io) : PortIO(io) {}
	Port::~Port() = default;
}
