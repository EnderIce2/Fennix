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

#include <array>
#include <cstddef>
#include <cstdint>

void test_stl_array()
{
	{
		std::array<int, 3> a = {1, 2, 3};
		UNUSED(a);
	}
	{
		std::array<int, 5> arr = {1, 2, 3, 4, 5};

		for (size_t i = 0; i < arr.size(); ++i)
		{
			if (arr[i] != int(i + 1))
				throw std::runtime_error("Array test failed");
		}
	}
	{
		std::array<int, 5> arr = {1, 2, 3, 4, 5};
		std::array<int, 5> arr2 = arr;

		for (size_t i = 0; i < arr.size(); ++i)
		{
			if (arr2[i] != int(i + 1))
				throw std::runtime_error("Array copy test failed");
		}
	}
	{
		std::array<int, 5> arr = {1, 2, 3, 4, 5};
		arr.fill(0);

		for (size_t i = 0; i < arr.size(); ++i)
		{
			if (arr[i] != 0)
				throw std::runtime_error("Array fill test failed");
		}
	}
	{
		std::array<int, 5> arr = {1, 2, 3, 4, 5};
		std::array<int, 5> arr2 = {6, 7, 8, 9, 10};

		arr.swap(arr2);

		for (size_t i = 0; i < arr.size(); ++i)
		{
			if (arr[i] != int(i + 6))
				throw std::runtime_error("Array swap test failed");
		}
	}
	{
		std::array<int, 5> arr = {1, 2, 3, 4, 5};
		if (arr.empty())
			throw std::runtime_error("Array empty test failed");
		if (arr.size() != 5)
			throw std::runtime_error("Array size test failed");
		if (arr.max_size() != 5)
			throw std::runtime_error("Array max_size test failed");
		if (arr.front() != 1)
			throw std::runtime_error("Array front test failed");
		if (arr.back() != 5)
			throw std::runtime_error("Array back test failed");
		if (arr.data() != &arr[0])
			throw std::runtime_error("Array data test failed");
	}
}

#endif
