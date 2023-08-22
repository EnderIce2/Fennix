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

#ifndef __FENNIX_KERNEL_STD_STRING_H__
#define __FENNIX_KERNEL_STD_STRING_H__

#include <types.h>
#include <convert.h>
#include <debug.h>

// Show debug messages
// #define DEBUG_CPP_STRING 1
// #define DEBUG_CPP_STRING_VERBOSE 1

#ifdef DEBUG_CPP_STRING
#define strdbg(m, ...) debug(m, ##__VA_ARGS__)
#else
#define strdbg(m, ...)
#endif

#ifdef DEBUG_CPP_STRING_VERBOSE
#define v_strdbg(m, ...) debug(m, ##__VA_ARGS__)
#else
#define v_strdbg(m, ...)
#endif

// TODO: Somewhere the delete is called twice, causing a double free error.

namespace std
{
	/**
	 * @brief String class
	 * String class that can be used to store strings.
	 */
	class string
	{
	private:
		char *Data{};
		size_t Length{};
		size_t Capacity{};

	public:
		static const size_t npos = -1;

		string(const char *Str = "")
		{
			this->Length = strlen(Str);
			this->Capacity = this->Length + 1;
			this->Data = new char[this->Capacity];
			strcpy(this->Data, Str);
			strdbg("%#lx: New string created: \"%s\" (data: %#lx, length: %d, capacity: %d)",
				   this, this->Data, this->Data, this->Length, this->Capacity);
		}

		~string()
		{
			strdbg("%#lx: String deleted: \"%s\" (data: %#lx, length: %d, capacity: %d)",
				   this, this->Data, this->Data, this->Length, this->Capacity);
			delete[] this->Data, this->Data = nullptr;
		}

		size_t length() const
		{
			v_strdbg("%#lx: String length: %d",
					 this, this->Length);
			return this->Length;
		}

		size_t capacity() const
		{
			v_strdbg("%#lx: String capacity: %d",
					 this, this->Capacity);
			return this->Capacity;
		}

		const char *c_str() const
		{
			v_strdbg("%#lx: String data: \"%s\"",
					 this, this->Data);
			return this->Data;
		}

		void resize(size_t NewLength)
		{
			strdbg("%#lx: String resize: %d",
				   this, NewLength);
			if (NewLength < this->Capacity)
			{
				this->Length = NewLength;
				this->Data[this->Length] = '\0';

				strdbg("%#lx: String resized: \"%s\" (data: %#lx, length: %d, capacity: %d)",
					   this, this->Data, this->Data, this->Length, this->Capacity);
				return;
			}

			size_t newCapacity = NewLength + 1;
			char *newData = new char[newCapacity];
			strcpy(newData, this->Data);

			strdbg("%#lx: old: %#lx, new: %#lx",
				   this, this->Data, newData);

			delete[] this->Data;
			this->Data = newData;
			this->Length = NewLength;
			this->Capacity = newCapacity;

			strdbg("%#lx: String resized: \"%s\" (data: %#lx, length: %d, capacity: %d)",
				   this, this->Data, this->Data, this->Length, this->Capacity);
		}

		void concat(const string &Other)
		{
			size_t NewLength = this->Length + Other.Length;
			this->resize(NewLength);

			strcat(this->Data, Other.Data);
			strdbg("%#lx: String concatenated: \"%s\" (data: %#lx, length: %d, capacity: %d)",
				   this, this->Data, this->Data, this->Length, this->Capacity);
		}

		bool empty() const
		{
			strdbg("%#lx: String empty: %d",
				   this, this->Length == 0);
			return this->Length == 0;
		}

		size_t size() const
		{
			strdbg("%#lx: String size: %d",
				   this, this->Length);
			return this->Length;
		}

		void clear()
		{
			strdbg("%#lx: String clear", this);
			this->resize(0);
		}

		size_t find(const char *Str, size_t Pos = 0) const
		{
			strdbg("%#lx: String find: \"%s\", %d",
				   this, Str, Pos);
			if (Pos >= this->Length)
				return npos;

			for (size_t i = Pos; i < this->Length; i++)
			{
				bool found = true;
				for (size_t j = 0; Str[j] != '\0'; j++)
				{
					if (this->Data[i + j] != Str[j])
					{
						found = false;
						break;
					}
				}
				if (found)
					return i;
			}
			return npos;
		}

		size_t find(const string &Str, size_t Pos = 0) const
		{
			strdbg("%#lx: String find: \"%s\", %d",
				   this, Str.c_str(), Pos);
			return this->find(Str.c_str(), Pos);
		}

		void erase(int Index, int Count = 1)
		{
			strdbg("%#lx: String erase: %d, %d",
				   this, Index, Count);
			if (Index < 0 || (size_t)Index >= this->Length)
				return;

			if (Count < 0)
				return;

			if ((size_t)(Index + Count) > this->Length)
				Count = (int)this->Length - Index;

			for (size_t i = Index; i < this->Length - Count; i++)
				this->Data[i] = this->Data[i + Count];

			this->Length -= Count;
			this->Data[this->Length] = '\0';
			strdbg("%#lx: String erased: \"%s\" (data: %#lx, length: %d, capacity: %d)",
				   this, this->Data, this->Data, this->Length, this->Capacity);
		}

		size_t find_last_not_of(const char *Str, size_t Pos = npos) const
		{
			strdbg("%#lx: String find_last_not_of: \"%s\", %d",
				   this, Str, Pos);
			if (Pos == npos)
				Pos = this->Length - 1;

			for (int i = (int)Pos; i >= 0; i--)
			{
				bool found = false;
				for (size_t j = 0; Str[j] != '\0'; j++)
				{
					if (this->Data[i] == Str[j])
					{
						found = true;
						break;
					}
				}
				if (!found)
					return i;
			}
			return npos;
		}

		size_t find_first_not_of(const char *Str, size_t Pos = 0) const
		{
			strdbg("%#lx: String find_first_not_of: \"%s\", %d",
				   this, Str, Pos);
			if (Pos >= this->Length)
				return npos;

			for (size_t i = Pos; i < this->Length; i++)
			{
				bool found = false;
				for (size_t j = 0; Str[j] != '\0'; j++)
				{
					if (this->Data[i] == Str[j])
					{
						found = true;
						break;
					}
				}
				if (!found)
					return i;
			}
			return npos;
		}

		size_t find_first_of(const char *Str, size_t Pos = 0) const
		{
			strdbg("%#lx: String find_first_of: \"%s\", %d",
				   this, Str, Pos);
			if (Pos >= this->Length)
				return npos;

			for (size_t i = Pos; i < this->Length; i++)
			{
				bool found = false;
				for (size_t j = 0; Str[j] != '\0'; j++)
				{
					if (this->Data[i] == Str[j])
					{
						found = true;
						break;
					}
				}
				if (found)
					return i;
			}
			return npos;
		}

		size_t find_last_of(const char *Str, size_t Pos = npos) const
		{
			strdbg("%#lx: String find_last_of: \"%s\", %d",
				   this, Str, Pos);
			if (Pos == npos)
				Pos = this->Length - 1;

			for (int i = (int)Pos; i >= 0; i--)
			{
				bool found = false;
				for (int j = 0; Str[j] != '\0'; j++)
				{
					if (this->Data[i] == Str[j])
					{
						found = true;
						break;
					}
				}
				if (found)
					return i;
			}
			return npos;
		}

		size_t find_first_of(char C, size_t Pos = 0) const
		{
			strdbg("%#lx: String find_first_of: '%c', %d",
				   this, C, Pos);
			if (Pos >= this->Length)
				return npos;

			for (size_t i = Pos; i < this->Length; i++)
			{
				if (this->Data[i] == C)
					return i;
			}
			return npos;
		}

		size_t find_last_of(char C, size_t Pos = npos) const
		{
			strdbg("%#lx: String find_last_of: '%c', %d",
				   this, C, Pos);
			if (Pos == npos)
				Pos = this->Length - 1;

			for (int i = (int)Pos; i >= 0; i--)
			{
				if (this->Data[i] == C)
					return i;
			}
			return npos;
		}

		size_t substr(const char *Str, size_t Pos = 0) const
		{
			strdbg("%#lx: String substr: \"%s\", %d",
				   this, Str, Pos);
			if (Pos >= this->Length)
				return npos;

			for (size_t i = Pos; i < this->Length; i++)
			{
				bool found = true;
				for (size_t j = 0; Str[j] != '\0'; j++)
				{
					if (this->Data[i + j] != Str[j])
					{
						found = false;
						break;
					}
				}
				if (found)
					return i;
			}
			return npos;
		}

		size_t substr(const string &Str, size_t Pos = 0) const
		{
			strdbg("%#lx: String substr: \"%s\", %d",
				   this, Str.c_str(), Pos);
			return this->substr(Str.c_str(), Pos);
		}

		string substr(size_t Pos = 0, size_t Count = npos) const
		{
			strdbg("%#lx: String substr: %d, %d",
				   this, Pos, Count);
			if (Pos >= this->Length)
				return string();

			if (Count == npos)
				Count = this->Length - Pos;

			if (Pos + Count > this->Length)
				Count = this->Length - Pos;

			string ret;
			ret.resize(Count);
			for (size_t i = 0; i < Count; i++)
				ret.Data[i] = this->Data[Pos + i];
			ret.Data[Count] = '\0';
			return ret;
		}

		void replace(size_t Pos, size_t Count, const char *Str)
		{
			strdbg("%#lx: String replace: %d, %d, \"%s\"",
				   this, Pos, Count, Str);
			if (Pos >= this->Length)
				return;

			if ((int64_t)Count <= 0)
				return;

			if (Pos + Count > this->Length)
				Count = this->Length - Pos;

			size_t NewLength = this->Length - Count + strlen(Str);
			this->resize(NewLength);

			for (size_t i = this->Length - 1; i >= Pos + strlen(Str); i--)
				this->Data[i] = this->Data[i - strlen(Str) + Count];

			for (unsigned long i = 0; i < strlen(Str); i++)
				this->Data[Pos + i] = Str[i];

			strdbg("%#lx: String replaced: \"%s\" (data: %#lx, length: %d, capacity: %d)",
				   this, this->Data, this->Data, this->Length, this->Capacity);
		}

		void replace(size_t Pos, size_t Count, const string &Str)
		{
			strdbg("%#lx: String replace: %d, %d, \"%s\"",
				   this, Pos, Count, Str.Data);
			if (Pos >= this->Length)
				return;

			if ((int64_t)Count <= 0)
				return;

			if (Pos + Count > this->Length)
				Count = this->Length - Pos;

			size_t NewLength = this->Length - Count + Str.Length;
			this->resize(NewLength);

			for (size_t i = this->Length - 1; i >= Pos + Str.Length; i--)
				this->Data[i] = this->Data[i - Str.Length + Count];

			for (size_t i = 0; i < Str.Length; i++)
				this->Data[Pos + i] = Str.Data[i];

			strdbg("%#lx: String replaced: \"%s\" (data: %#lx, length: %d, capacity: %d)",
				   this, this->Data, this->Data, this->Length, this->Capacity);
		}

		void pop_back()
		{
			strdbg("%#lx: String pop_back", this);
			if (this->Length > 0)
			{
				this->Data[this->Length - 1] = '\0';
				this->Length--;
			}
		}

		string operator+(const string &Other) const
		{
			string result = *this;
			result.concat(Other);
			strdbg("%#lx: String added: \"%s\" (data: %#lx, length: %d, capacity: %d)",
				   this, result.Data, result.Data, result.Length, result.Capacity);
			return result;
		}

		string operator+(const char *Other) const
		{
			string result = *this;
			result.concat(Other);
			strdbg("%#lx: String added: \"%s\" (data: %#lx, length: %d, capacity: %d)",
				   this, result.Data, result.Data, result.Length, result.Capacity);
			return result;
		}

		string &operator+=(const string &Other)
		{
			this->concat(Other);
			strdbg("%#lx: String appended: \"%s\" (data: %#lx, length: %d, capacity: %d)",
				   this, this->Data, this->Data, this->Length, this->Capacity);
			return *this;
		}

		string &operator+=(const char *Other)
		{
			this->concat(Other);
			strdbg("%#lx: String appended: \"%s\" (data: %#lx, length: %d, capacity: %d)",
				   this, this->Data, this->Data, this->Length, this->Capacity);
			return *this;
		}

		string &operator+=(char Other)
		{
			const char str[2] = {Other, '\0'};
			this->concat(str);
			strdbg("%#lx: String appended: \"%s\" (data: %#lx, length: %d, capacity: %d)",
				   this, this->Data, this->Data, this->Length, this->Capacity);
			return *this;
		}

		/* warning: implicitly-declared ‘constexpr String::String(const String&)’ is deprecated [-Wdeprecated-copy] */
		string &operator=(const string &Other) = default;

		// string &operator=(const string &Other)
		// {
		// 	if (this != &Other)
		// 	{
		// 		delete[] this->Data;
		// 		this->Data = Other.Data;
		// 		this->Length = Other.Length;
		// 		this->Capacity = Other.Capacity;
		// 		strdbg("%#lx: String assigned: \"%s\" (data: %#lx, length: %d, capacity: %d)",
		// 			   this, this->Data, this->Data, this->Length, this->Capacity);
		// 	}
		// 	return *this;
		// }

		string &operator=(const char *Other)
		{
			this->Length = strlen(Other);
			this->Capacity = this->Length + 1;
			delete[] this->Data;
			this->Data = new char[this->Capacity];
			strcpy(this->Data, Other);
			strdbg("%#lx: String assigned: \"%s\" (data: %#lx, length: %d, capacity: %d)",
				   this, this->Data, this->Data, this->Length, this->Capacity);
			return *this;
		}

		string &operator<<(const string &Other)
		{
			this->concat(Other);
			strdbg("%#lx: String appended: \"%s\" (data: %#lx, length: %d, capacity: %d)",
				   this, this->Data, this->Data, this->Length, this->Capacity);
			return *this;
		}

		string &operator<<(const char *Other)
		{
			this->concat(Other);
			strdbg("%#lx: String appended: \"%s\" (data: %#lx, length: %d, capacity: %d)",
				   this, this->Data, this->Data, this->Length, this->Capacity);
			return *this;
		}

		char &operator[](int Index)
		{
			strdbg("%#lx: String index: %d", this, Index);
			return this->Data[Index];
		}

		const char &operator[](int Index) const
		{
			strdbg("%#lx: String index: %d", this, Index);
			return this->Data[Index];
		}

		char &operator[](size_t Index)
		{
			strdbg("%#lx: String index: %d", this, Index);
			return this->Data[Index];
		}

		const char &operator[](size_t Index) const
		{
			strdbg("%#lx: String index: %d", this, Index);
			return this->Data[Index];
		}

		bool operator==(const string &Other) const
		{
			strdbg("%#lx: String compared: \"%s\" == \"%s\"",
				   this, this->Data, Other.Data);
			return strcmp(this->Data, Other.Data) == 0;
		}

		bool operator!=(const char *Other) const
		{
			strdbg("%#lx: String compared: \"%s\" != \"%s\"",
				   this, this->Data, Other);
			return strcmp(this->Data, Other) != 0;
		}

		bool operator!=(const string &Other) const
		{
			strdbg("%#lx: String compared: \"%s\" != \"%s\"",
				   this, this->Data, Other.Data);
			return strcmp(this->Data, Other.Data) != 0;
		}

		bool operator==(const char *Other) const
		{
			strdbg("%#lx: String compared: \"%s\" == \"%s\"",
				   this, this->Data, Other);
			return strcmp(this->Data, Other) == 0;
		}

		class iterator
		{
		private:
			char *Pointer;

		public:
			iterator(char *Pointer) : Pointer(Pointer) {}

			iterator &operator++()
			{
				++this->Pointer;
				strdbg("%#lx: String iterator incremented: %#lx",
					   this, this->Pointer);
				return *this;
			}

			char &operator*()
			{
				strdbg("%#lx: String iterator dereferenced: %#lx",
					   this, this->Pointer);
				return *this->Pointer;
			}

			bool operator!=(const iterator &Other) const
			{
				strdbg("%#lx: String iterator compared: %#lx != %#lx",
					   this, this->Pointer, Other.Pointer);
				return this->Pointer != Other.Pointer;
			}

			bool operator==(const iterator &Other) const
			{
				strdbg("%#lx: String iterator compared: %#lx == %#lx",
					   this, this->Pointer, Other.Pointer);
				return this->Pointer == Other.Pointer;
			}
		};

		iterator begin()
		{
			strdbg("%#lx: String iterator begin: %#lx",
				   this, this->Data);
			return iterator(this->Data);
		}

		iterator end()
		{
			strdbg("%#lx: String iterator end: %#lx",
				   this, this->Data + this->Length);
			return iterator(this->Data + this->Length);
		}
	};
}

#endif // !__FENNIX_KERNEL_STD_STRING_H__
