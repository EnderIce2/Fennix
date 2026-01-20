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

#define DB_TARGET_COMMAND 0
#define DB_TARGET_CONTROL_ENDPOINT 1

namespace Driver::ExtensibleHostControllerInterface
{
	extern dev_t DriverID;

	void DoorbellManager::Ring(uint8_t Slot, uint8_t Target)
	{
		DBs[Slot].raw = Target;
	}

	void DoorbellManager::RingCommand()
	{
		Ring(0, DB_TARGET_COMMAND);
	}

	void DoorbellManager::RingControlEndpoint(uint8_t Slot)
	{
		Ring(Slot, DB_TARGET_CONTROL_ENDPOINT);
	}

	DoorbellManager::DoorbellManager(Doorbell *db) : DBs(db)
	{
	}
}
