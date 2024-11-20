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
	RetryLock:
		bool Result = this->Locked.exchange(true, std::memory_order_acquire);

		TCB *tcb = thisThread;
		assert(tcb != nullptr);
		if (Result == true)
		{
			debug("%#lx: Mutex is locked, blocking task %d (\"%s\" : %d)", this,
				  tcb->ID, tcb->Parent->Name, tcb->Parent->ID);

			this->Waiting.push_back(tcb);
			tcb->Block();
			TaskManager->Yield();
			goto RetryLock;
		}

		this->Holder = tcb;
		this->Waiting.remove(tcb);

		debug("%#lx: Mutex locked by task %d (\"%s\" : %d)", this,
			  tcb->ID, tcb->Parent->Name, tcb->Parent->ID);
	}

	bool mutex::try_lock()
	{
		bool Result = this->Locked.exchange(true, std::memory_order_acquire);

		TCB *tcb = thisThread;
		assert(tcb != nullptr);
		if (Result == true)
		{
			debug("%#lx: Mutex is locked, task %d (\"%s\" : %d) failed to lock", this,
				  tcb->ID, tcb->Parent->Name, tcb->Parent->ID);
			return false;
		}

		this->Holder = tcb;
		this->Waiting.remove(tcb);

		debug("%#lx: Mutex locked by task %d (\"%s\" : %d)", this,
			  tcb->ID, tcb->Parent->Name, tcb->Parent->ID);
		return true;
	}

	void mutex::unlock()
	{
		TCB *tcb = thisThread;
		assert(tcb != nullptr);
		assert(this->Holder == tcb);

		this->Holder = nullptr;
		this->Locked.store(false, std::memory_order_release);

		if (this->Waiting.empty())
		{
			debug("%#lx: Mutex unlocked, no tasks to unblock", this);
			return;
		}

		TCB *Next = this->Waiting.front();
		this->Waiting.pop_front();

		debug("%#lx: Mutex unlocked, task %d (\"%s\" : %d) unblocked", this,
			  Next->ID, Next->Parent->Name, Next->Parent->ID);

		Next->Unblock();
	}

	mutex::mutex()
	{
		debug("%#lx: Creating mutex", this);
	}

	mutex::~mutex()
	{
		debug("%#lx: Destroying mutex", this);
		assert(this->Holder == nullptr);
		assert(this->Waiting.empty());
	}
}
