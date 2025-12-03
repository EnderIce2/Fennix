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

extern Driver::Manager *DriverManager;
namespace Driver::UniversalHostControllerInterface
{
	extern dev_t DriverID;

	Queue::Queue()
	{
		this->sched = new UniversalSerialBus::Scheduler(1024, 1, 900);
		auto qhpool = this->sched->CreateNewPool(sizeof(QH), 0x10, 8, offsetof(QH, HEAD), offsetof(QH, ELEMENT), offsetof(QH, __software));
		auto tdpool = this->sched->CreateNewPool(sizeof(TD), 0x10, 32, offsetof(TD, LINK), offsetof(TD, LINK), offsetof(TD, __software));
		this->sched->Initialize(0);

		stub;
	}

	Queue::~Queue()
	{
		delete this->sched;
	}
}
