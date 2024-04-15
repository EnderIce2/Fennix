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

#ifndef __FENNIX_KERNEL_STD_SMART_POINTER_H__
#define __FENNIX_KERNEL_STD_SMART_POINTER_H__

#include <types.h>

#include <type_traits>
#include <debug.h>

// show debug messages
// #define DEBUG_SMARTPOINTERS 1

#ifdef DEBUG_SMARTPOINTERS
#define spdbg(m, ...) debug(m, ##__VA_ARGS__)
#else
#define spdbg(m, ...)
#endif

namespace std
{
	/**
	 * @brief A smart pointer class
	 *
	 * This class is a smart pointer class. It is used to manage the lifetime of
	 * objects. It is a reference counted pointer, so when the last reference to
	 * the object is removed, the object is deleted.
	 *
	 * Basic Usage:
	 * smart_ptr<char> pointer(new char());
	 * *pointer = 'a';
	 * printf("%c", *pointer); // Prints "a"
	 */
	template <class T>
	class smart_ptr
	{
		T *RealPointer;

	public:
		explicit smart_ptr(T *Pointer = nullptr)
		{
			spdbg("Smart pointer created (%#lx)", this->RealPointer);
			this->RealPointer = Pointer;
		}

		~smart_ptr()
		{
			spdbg("Smart pointer deleted (%#lx)", this->RealPointer);
			delete this->RealPointer, this->RealPointer = nullptr;
		}

		T &operator*()
		{
			spdbg("Smart pointer dereferenced (%#lx)", this->RealPointer);
			return *this->RealPointer;
		}

		T *operator->()
		{
			spdbg("Smart pointer dereferenced (%#lx)", this->RealPointer);
			return this->RealPointer;
		}

		T *get()
		{
			spdbg("Smart pointer returned (%#lx)", this->RealPointer);
			return this->RealPointer;
		}
	};

	template <class T>
	class auto_ptr
	{
	};

	template <class T>
	class unique_ptr
	{
	};

	template <class T>
	class weak_ptr
	{
	};

	template <typename T>
	class shared_ptr
	{
	private:
		class counter
		{
		private:
			unsigned int RefCount{};

		public:
			counter() : RefCount(0) { spdbg("Counter %#lx created", this); };
			counter(const counter &) = delete;
			counter &operator=(const counter &) = delete;
			~counter() { spdbg("Counter %#lx deleted", this); }
			void reset()
			{
				this->RefCount = 0;
				spdbg("reset");
			}

			unsigned int get()
			{
				return this->RefCount;
				spdbg("return");
			}

			void operator++()
			{
				this->RefCount++;
				spdbg("increment");
			}

			void operator++(int)
			{
				this->RefCount++;
				spdbg("increment");
			}

			void operator--()
			{
				this->RefCount--;
				spdbg("decrement");
			}

			void operator--(int)
			{
				this->RefCount--;
				spdbg("decrement");
			}
		};

		counter *ReferenceCounter;
		T *RealPointer;

	public:
		explicit shared_ptr(T *Pointer = nullptr)
		{
			this->RealPointer = Pointer;
			this->ReferenceCounter = new counter();
			spdbg("[%#lx] Shared pointer created (ptr=%#lx, ref=%#lx)", this, Pointer, this->ReferenceCounter);
			if (Pointer)
				(*this->ReferenceCounter)++;
		}

		shared_ptr(shared_ptr<T> &SPtr)
		{
			spdbg("[%#lx] Shared pointer copied (ptr=%#lx, ref=%#lx)", this, SPtr.RealPointer, SPtr.ReferenceCounter);
			this->RealPointer = SPtr.RealPointer;
			this->ReferenceCounter = SPtr.ReferenceCounter;
			(*this->ReferenceCounter)++;
		}

		~shared_ptr()
		{
			spdbg("[%#lx] Shared pointer destructor called", this);
			(*this->ReferenceCounter)--;
			if (this->ReferenceCounter->get() == 0)
			{
				spdbg("[%#lx] Shared pointer deleted (ptr=%#lx, ref=%#lx)", this, this->RealPointer, this->ReferenceCounter);
				delete this->ReferenceCounter, this->ReferenceCounter = nullptr;
				delete this->RealPointer, this->RealPointer = nullptr;
			}
		}

		unsigned int get_count()
		{
			spdbg("[%#lx] Shared pointer count (%d)", this, this->ReferenceCounter->get());
			return this->ReferenceCounter->get();
		}

		T *get()
		{
			spdbg("[%#lx] Shared pointer get (%#lx)", this, this->RealPointer);
			return this->RealPointer;
		}

		T &operator*()
		{
			spdbg("[%#lx] Shared pointer dereference (ptr*=%#lx)", this, *this->RealPointer);
			return *this->RealPointer;
		}

		T *operator->()
		{
			spdbg("[%#lx] Shared pointer dereference (ptr->%#lx)", this, this->RealPointer);
			return this->RealPointer;
		}

		void reset(T *Pointer = nullptr)
		{
			if (this->RealPointer == Pointer)
				return;
			spdbg("[%#lx] Shared pointer reset (ptr=%#lx, ref=%#lx)", this, Pointer, this->ReferenceCounter);
			(*this->ReferenceCounter)--;
			if (this->ReferenceCounter->get() == 0)
			{
				delete this->RealPointer;
				delete this->ReferenceCounter;
			}
			this->RealPointer = Pointer;
			this->ReferenceCounter = new counter();
			if (Pointer)
				(*this->ReferenceCounter)++;
		}

		void reset()
		{
			spdbg("[%#lx] Shared pointer reset (ptr=%#lx, ref=%#lx)", this, this->RealPointer, this->ReferenceCounter);
			if (this->ReferenceCounter->get() == 1)
			{
				delete this->RealPointer, this->RealPointer = nullptr;
				delete this->ReferenceCounter, this->ReferenceCounter = nullptr;
			}
			else
			{
				(*this->ReferenceCounter)--;
			}
		}

		void swap(shared_ptr<T> &Other)
		{
			spdbg("[%#lx] Shared pointer swap (ptr=%#lx, ref=%#lx <=> ptr=%#lx, ref=%#lx)",
				  this, this->RealPointer, this->ReferenceCounter, Other.RealPointer, Other.ReferenceCounter);
			T *tempRealPointer = this->RealPointer;
			counter *tempReferenceCounter = this->ReferenceCounter;
			this->RealPointer = Other.RealPointer;
			this->ReferenceCounter = Other.ReferenceCounter;
			Other.RealPointer = tempRealPointer;
			Other.ReferenceCounter = tempReferenceCounter;
		}
	};

	template <typename T, typename... Args>
	shared_ptr<T> make_shared(Args &&...args)
	{
		return shared_ptr<T>(new T(forward<Args>(args)...));
	};

	template <typename T, typename... Args>
	smart_ptr<T> make_smart(Args &&...args)
	{
		return smart_ptr<T>(new T(forward<Args>(args)...));
	};
}

#endif // !__FENNIX_KERNEL_STD_SMART_POINTER_H__
