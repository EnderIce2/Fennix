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
	void unhandled_exception() { assert(!"std::terminate();"); }
};

resumable foo()
{
	debug("await start");
	co_await std::suspend_always();
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
			assert("std::terminate();");
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

	/* Example of coroutine */
	auto p = foo();
	while (p.resume())
		;
}

#endif // DEBUG
