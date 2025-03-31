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

/* https://gcc.gnu.org/bugzilla/show_bug.cgi?id=109867 */
#pragma GCC diagnostic ignored "-Wswitch-default"

#include "../t.h"
#include "../kernel.h"

#include <coroutine>
#include <thread>

/* https://gist.github.com/Qix-/caa277fbf1a4e6ca55a27f2242df3b9a */

class resumable
{
public:
	struct promise_type;
	using coro_handle = std::coroutine_handle<promise_type>;

	resumable(coro_handle handle) : co_handle(handle) { assert(handle); }
	resumable(resumable &) = delete;
	resumable(resumable &&) = delete;

	~resumable() { co_handle.destroy(); }

	bool resume()
	{
		if (!co_handle.done())
			co_handle.resume();
		return !co_handle.done();
	}

private:
	coro_handle co_handle;
};

struct resumable::promise_type
{
	using coro_handle = std::coroutine_handle<promise_type>;

	auto get_return_object() { return coro_handle::from_promise(*this); }
	auto initial_suspend() { return std::suspend_always(); }
	auto final_suspend() noexcept { return std::suspend_always(); }
	void return_void() {}
	void unhandled_exception() { std::terminate(); }
};

resumable foo()
{
	debug("await start");
	co_await std::suspend_always();
	debug("done");
}

/* ===================================================================== */

struct Generator
{
	struct promise_type
	{
		int current_value;

		Generator get_return_object()
		{
			return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
		}

		std::suspend_always initial_suspend()
		{
			return {};
		}

		std::suspend_always final_suspend() noexcept
		{
			return {};
		}

		void return_void()
		{
		}

		std::suspend_always yield_value(int value)
		{
			current_value = value;
			return {};
		}

		void unhandled_exception()
		{
			std::terminate();
		}
	};

	std::coroutine_handle<promise_type> handle;

	Generator(std::coroutine_handle<promise_type> h) : handle(h) {}

	~Generator()
	{
		if (handle)
			handle.destroy();
	}

	bool next()
	{
		if (!handle || handle.done())
			return false;

		handle.resume();
		return true;
	}

	int value() const
	{
		int ret = handle.promise().current_value;
		return ret;
	}
};

Generator CountToThree()
{
	debug("1");
	co_yield 1;
	debug("2");
	co_yield 2;
	debug("3");
	co_yield 3;
	debug("end");
}

/* ===================================================================== */

struct Task
{
	struct promise_type
	{
		Task get_return_object() { return Task{std::coroutine_handle<promise_type>::from_promise(*this)}; }
		std::suspend_never initial_suspend() { return {}; }
		std::suspend_never final_suspend() noexcept { return {}; }
		void return_void() {}
		void unhandled_exception() { std::terminate(); }
	};

	std::coroutine_handle<promise_type> handle;
	Task(std::coroutine_handle<promise_type> h) : handle(h) {}
	~Task()
	{
		if (handle)
			handle.destroy();
	}
};

struct Awaiter
{
	bool await_ready() { return false; }
	void await_suspend(std::coroutine_handle<> h)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
		h.resume();
	}
	void await_resume() {}
};

Task AsyncFunc()
{
	debug("waiting");
	co_await Awaiter{};
	debug("done");
}

/* ===================================================================== */

class SyscallAwaitable
{
public:
	bool await_ready() const noexcept
	{
		debug("Syscall await_ready() called");
		return false; /* false - coroutine is not ready, so suspend */
	}

	void await_suspend(std::coroutine_handle<> handle) const noexcept
	{
		debug("Syscall suspended, waiting for data...");
	}

	int await_resume() const noexcept
	{
		debug("Syscall resumed and returned 42");
		return 42;
	}
};

class SyscallTask
{
public:
	struct promise_type
	{
		SyscallTask get_return_object()
		{
			return SyscallTask{std::coroutine_handle<promise_type>::from_promise(*this)};
		}

		std::suspend_never initial_suspend()
		{
			debug("Syscall started");
			return {};
		}

		std::suspend_never final_suspend() noexcept
		{
			debug("Syscall finished");
			return {};
		}

		void return_void()
		{
			debug("Syscall finished");
		}

		void unhandled_exception()
		{
			std::terminate();
		}
	};

	std::coroutine_handle<promise_type> handle;
	explicit SyscallTask(std::coroutine_handle<promise_type> h) : handle(h) {}
	~SyscallTask()
	{
		if (handle)
			handle.destroy();
	}
};

SyscallTask perform_syscall()
{
	debug("Initializing syscall I/O...");
	int data = co_await SyscallAwaitable{};
	debug("Syscall finished: %d", data);
}

void coroutineTest()
{
	/* Example of syscall using coroutine */
	auto task = perform_syscall();
	task.handle.resume();

	/* async task */
	AsyncFunc();

	/* generator */
	auto gen = CountToThree();
	while (gen.next())
	{
		auto a = gen.value();
		debug("%d", a);
	}

	/* Example of coroutine */
	auto p = foo();
	while (p.resume())
		;
}

#endif // DEBUG
