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

#include <future>
#include <thread>
#include <assert.h>

void test_stl_future()
{
	// {
	// 	std::packaged_task<int()> task([]
	// 								   { return 7; });
	// 	std::future<int> f1 = task.get_future();
	// 	std::thread t(std::move(task));

	// 	std::future<int> f2 = std::async(std::launch::async, []
	// 									 { return 8; });

	// 	std::promise<int> p;
	// 	std::future<int> f3 = p.get_future();
	// 	std::thread([&p]
	// 				{ p.set_value_at_thread_exit(9); })
	// 		.detach();

	// 	debug("Waiting for futures...");
	// 	f1.wait();
	// 	f2.wait();
	// 	f3.wait();
	// 	debug("results: %d %d %d", f1.get(), f2.get(), f3.get());
	// 	t.join();
	// }

	// {
	// 	std::future<int> f = std::async(std::launch::async, []
	// 									{
	// 		for (uint64_t i = 0; i < 100000; ++i);
	// 		return 1; });

	// 	debug("waiting for future");
	// 	int result = f.get();
	// 	debug("future result is %d", result);
	// }

	// 	{
	// 		std::promise<int> p;
	// 		std::future<int> f = p.get_future();

	// 		assert(f.valid());
	// 		p.set_value(42);
	// 		assert(f.get() == 42);

	// 		try
	// 		{
	// 			f.get();
	// 			assert(false);
	// 		}
	// 		catch (const std::future_error &e)
	// 		{
	// 			// assert(e.code() == std::future_errc::future_already_retrieved);
	// 		}
	// 	}

	// 	{
	// 		auto future = std::async([]()
	// 								 { return 42; });
	// 		assert(future.get() == 42);
	// 	}

	// {
	// 	std::promise<int> p;
	// 	std::future<int> f = p.get_future();
	// 	std::shared_future<int> sf = f.share();

	// 	p.set_value(42);

	// 	assert(sf.get() == 42);
	// 	assert(sf.get() == 42);
	// }

	// 	{
	// 		std::promise<int> p;
	// 		std::future<int> f = p.get_future();

	// 		auto status = f.wait_for(std::chrono::milliseconds(100));
	// 		assert(status == std::future_status::timeout);

	// 		p.set_value(42);
	// 		status = f.wait_for(std::chrono::milliseconds(100));
	// 		assert(status == std::future_status::ready);
	// 	}

	// 	{
	// 		auto future = std::async(std::launch::async, []()
	// 								 { return 42; });
	// 		assert(future.get() == 42);

	// 		auto deferred = std::async(std::launch::deferred, []()
	// 								   { return 42; });
	// 		assert(deferred.get() == 42);
	// 	}

	// 	{
	// 		std::promise<int> p1, p2;
	// 		std::future<int> f1 = p1.get_future();
	// 		std::future<int> f2 = p2.get_future();

	// 		p1.set_value(42);
	// 		p2.set_value(24);

	// 		assert(f1.get() == 42);
	// 		assert(f2.get() == 24);
	// 	}

	// 	{
	// 		std::promise<int> p;
	// 		std::future<int> f1 = p.get_future();
	// 		std::future<int> f2 = std::move(f1);

	// 		p.set_value(42);
	// 		assert(f2.get() == 42);
	// 	}

	// 	{
	// 		std::promise<int> p;
	// 		std::shared_future<int> sf = p.get_future().share();

	// 		std::atomic<int> sum{0};
	// 		std::vector<std::thread> threads;

	// 		for (int i = 0; i < 10; ++i)
	// 		{
	// 			threads.emplace_back([&sf, &sum]()
	// 								 { sum += sf.get(); });
	// 		}

	// 		p.set_value(42);

	// 		for (auto &t : threads)
	// 		{
	// 			t.join();
	// 		}

	// 		assert(sum == 420);
	// }
}
