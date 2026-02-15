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
		/* This MUST be 1; otherwise something is wrong */
		assert(port.has(PORTSC::AlwaysOne));
		return port;
	}

	int Port::Clear(uint16_t Change)
	{
		uint16_t sc = port.in();
		sc &= ~Change;
		sc &= ~(PORTSC::CSC | PORTSC::PEC);
		sc |= (PORTSC::CSC | PORTSC::PEC) & Change;
		port.out(sc);
		return 0;
	}

	int Port::Set(uint16_t Change)
	{
		uint16_t sc = port.in();
		sc |= Change;
		sc &= ~(PORTSC::CSC | PORTSC::PEC);
		port.out(sc);
		return 0;
	}

	int Port::Reset()
	{
		Set(PORTSC::PR);
		v0::Sleep(DriverID, 100);
		Clear(PORTSC::PR);

		while (true)
		{
			v0::Sleep(DriverID, 50);
			uint16_t sc = port.in();

			if (~sc & PORTSC::CCS)
			{
				debug("device is not present");
				return -ENODEV;
			}

			if (sc & (PORTSC::CSC | PORTSC::PEC))
			{
				debug("status changed");
				Clear(PORTSC::CSC | PORTSC::PEC);
				continue;
			}

			if (sc & PORTSC::PE)
			{
				debug("port is enabled");
				break;
			}

			Set(PORTSC::PE);
		}
		return 0;
	}

	int Port::Probe()
	{
		Reset();
		uint16_t sc = port.in();

		if (sc & PORTSC::PE)
		{
			Speed = (sc & PORTSC::LSDA) ? USB_LOW_SPEED : USB_FULL_SPEED;
			return 0;
		}

		debug("port is not enabled");
		return -ENODEV;
	}

	Port::Port(PORTSC io) : port(io) {}
	Port::~Port() = default;
}
