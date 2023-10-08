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
