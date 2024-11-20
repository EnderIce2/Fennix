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

#ifdef DEBUG

#include "t.h"

#include "../kernel.h"

#include <mutex>
std::mutex test_mutex;

void mutex_test_long()
{
	while (true)
	{
		test_mutex.lock();
		debug("Long Thread %d got mutex",
			  thisThread->ID);
		// TaskManager->Sleep(2000);
		test_mutex.unlock();
	}
}

void mutex_test()
{
	while (true)
	{
		test_mutex.lock();
		debug("Thread %d got mutex",
			  thisThread->ID);
		// TaskManager->Sleep(200);
		test_mutex.unlock();
	}
}

void tasking_test_mutex()
{
	TaskManager->CreateThread(thisProcess, Tasking::IP(mutex_test_long));
	TaskManager->Yield();
	TaskManager->CreateThread(thisProcess, Tasking::IP(mutex_test));
	TaskManager->CreateThread(thisProcess, Tasking::IP(mutex_test));
	TaskManager->CreateThread(thisProcess, Tasking::IP(mutex_test));
	TaskManager->CreateThread(thisProcess, Tasking::IP(mutex_test));
	TaskManager->CreateThread(thisProcess, Tasking::IP(mutex_test));
	TaskManager->CreateThread(thisProcess, Tasking::IP(mutex_test));
	TaskManager->CreateThread(thisProcess, Tasking::IP(mutex_test));
	TaskManager->CreateThread(thisProcess, Tasking::IP(mutex_test));
	TaskManager->CreateThread(thisProcess, Tasking::IP(mutex_test));
	ilp;
}

#endif // DEBUG
