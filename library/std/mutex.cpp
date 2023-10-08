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

#include <mutex>

#include <algorithm>
#include <assert.h>
#include <cpu.hpp>

#include "../../kernel.h"

using namespace Tasking;

namespace std
{
	void mutex::lock()
	{
		bool Result = this->Locked.exchange(true, std::memory_order_acquire);
		__sync;

		if (Result)
		{
			this->Waiting.push_back(thisThread);
			thisThread->Block();
			TaskManager->Yield();
			return;
		}

		this->Holder = thisThread;
		this->Waiting.erase(std::find(this->Waiting.begin(),
									  this->Waiting.end(),
									  thisThread));
	}

	bool mutex::try_lock()
	{
		bool Result = this->Locked.exchange(true, std::memory_order_acquire);
		__sync;

		if (!Result)
		{
			this->Holder = thisThread;
			this->Waiting.erase(std::find(this->Waiting.begin(),
										  this->Waiting.end(),
										  thisThread));
		}
		return !Result;
	}

	void mutex::unlock()
	{
		__sync;
		this->Locked.store(false, std::memory_order_release);

		if (!this->Waiting.empty())
		{
			this->Holder = this->Waiting[0];

			this->Holder = this->Waiting.front();
			this->Waiting.erase(this->Waiting.begin());
			this->Holder->Unblock();
			TaskManager->Yield();
		}
		else
			this->Holder = nullptr;
	}
}
