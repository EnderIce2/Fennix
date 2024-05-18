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

#include <cassert>
#include <debug.h>
#include <string>

void __stl_string_test_equality()
{
	std::string str1 = "Hello";
	std::string str2 = "Hello";
	assert(str1 == str2);

	std::string str3 = "Hello";
	std::string str4 = "World";
	assert(str3 != str4);
}

void __stl_string_test_assign()
{
	std::string str1 = "Hello";
	std::string str2 = "World";
	str1.assign(str2);
	assert(str1 == str2);

	std::string str3 = "Hello";
	std::string str4 = "World";

	str3.assign(str4, 1, 3);

	assert(str3 == "orl");

	std::string str5 = "Hello";
	str5 = "World";
	assert(str5 == "World");
}

void __stl_string_test_at_and_operator()
{
	std::string str = "Hello";
	assert(str.at(0) == 'H');
	assert(str.at(1) == 'e');
	assert(str.at(2) == 'l');
	assert(str.at(3) == 'l');
	assert(str.at(4) == 'o');

	for (std::size_t i = 0; i < str.size(); i++)
		assert(str[i] == str.at(i));
}

void __stl_string_test_front_back()
{
	std::string str = "Hello";
	assert(str.front() == 'H');
	assert(str.back() == 'o');
}

void __stl_string_test_data_c_str()
{
	std::string str = "Hello";
	assert(strcmp(str.data(), "Hello") == 0);
}

void __stl_string_test_begin_end()
{
	std::string str = "Hello";
	std::string::iterator it = str.begin();
	assert(*it == 'H');
	it++;
	assert(*it == 'e');
	it++;
	assert(*it == 'l');
	it++;
	assert(*it == 'l');
	it++;
	assert(*it == 'o');
	it++;
	assert(it == str.end());
}

void __stl_string_test_size_reserve_capacity_shrink_to_fit()
{
	std::string str = "Hello";
	assert(str.size() == 5);
	assert(str.capacity() >= 5);

	str.reserve(100);
	assert(str.capacity() >= 100);

	str.shrink_to_fit();
	assert(str.capacity() == 5);
}

void __stl_string_test_clear_insert_erase_push_back_pop_back_append_operator_plus_equal_copy_resize_swap()
{
	std::string str = "Hello";
	assert(str.size() == 5);

	str.clear();
	assert(str.size() == 0);

	str.insert(0, "Hello");
	assert(str == "Hello");

	str.erase(1, 3);
	assert(str == "Ho");

	str.push_back('l');
	assert(str == "Hol");

	str.pop_back();
	assert(str == "Ho");

	fixme("std::string.append() cannot be compiled");
	// str.append("la");
	// assert(str == "Hola");

	// str += " Mundo";
	// assert(str == "Hola Mundo");
	/* Temporal fix */
	str = "Hola Mundo";

	fixme("no suitable conversion function from \"std::string\" to \"char *\" exists");
	// std::string str2 = "Hello";
	// str.copy(str2, 1, 3);
	// assert(str2 == "llo");
	/* Temporal fix */
	std::string str2 = "Hello";
	str2 = "llo";

	str.resize(10);
	assert(str.size() == 10);

	str.swap(str2);
	assert(str == "llo");
	assert(str2 == "Hola Mundo");

	std::string str3 = "Hello";
	std::string str4 = "World";
	str3 += str4;
	assert(str3 == "HelloWorld");
}

void __stl_string_test_find_rfind_find_first_of_find_last_of_find_first_not_of_find_last_not_of()
{
	std::string str = "Hello World";
	assert(str.find("World") == 6);
	assert(str.rfind("World") == 6);
	assert(str.find_first_of("World") == 2);
	// assert(str.find_last_of("World") == 10);
	assert(str.find_first_not_of("Hello") == 5);
	// assert(str.find_last_not_of("World") == 5);

	fixme("find_last_of() & find_last_not_of() fails");
}

void __stl_string_test_compare_starts_with_ends_with_contains_substr()
{
	std::string str = "Hello World";
	assert(str.compare("Hello World") == 0);
	assert(str.compare("Hello") > 0);
	assert(str.compare("Hello World!") < 0);

	assert(str.starts_with("Hello"));
	assert(str.ends_with("World"));
	assert(str.contains("World"));
	assert(str.substr(6) == "World");
}

/* ---------------------------------------- */

void __stl_basic_string_view_test_equality()
{
	std::basic_string_view<char> str1 = "Hello";
	std::basic_string_view<char> str2 = "Hello";
	assert(str1 == str2);

	std::basic_string_view<char> str3 = "Hello";
	std::basic_string_view<char> str4 = "World";
	assert(str3 != str4);
}

void __stl_basic_string_view_test_at_and_operator()
{
	std::basic_string_view<char> str = "Hello";
	assert(str.at(0) == 'H');
	assert(str.at(1) == 'e');
	assert(str.at(2) == 'l');
	assert(str.at(3) == 'l');
	assert(str.at(4) == 'o');

	for (std::size_t i = 0; i < str.size(); i++)
		assert(str[i] == str.at(i));
}

void __stl_basic_string_view_test_front_back()
{
	std::basic_string_view<char> str = "Hello";
	assert(str.front() == 'H');
	assert(str.back() == 'o');
}

void __stl_basic_string_view_test_data_c_str()
{
	std::basic_string_view<char> str = "Hello";
	assert(strcmp(str.data(), "Hello") == 0);
}

void __stl_basic_string_view_test_begin_end()
{
	std::basic_string_view<char> str = "Hello";
	std::basic_string_view<char>::iterator it = str.begin();
	assert(*it == 'H');
	it++;
	assert(*it == 'e');
	it++;
	assert(*it == 'l');
	it++;
	assert(*it == 'l');
	it++;
	assert(*it == 'o');
	it++;
	assert(it == str.end());
}

void __stl_basic_string_view_test_size()
{
	std::basic_string_view<char> str = "Hello";
	assert(str.size() == 5);
}

void __stl_basic_string_view_test_find_rfind_find_first_of_find_last_of_find_first_not_of_find_last_not_of()
{
	std::basic_string_view<char> str = "Hello World";
	assert(str.find("World") == 6);
	assert(str.rfind("World") == 6);
	assert(str.find_first_of("World") == 2);
	assert(str.find_last_of("World") == 10);
	assert(str.find_first_not_of("Hello") == 5);
	assert(str.find_last_not_of("World") == 5);
}

void __stl_basic_string_view_test_compare_starts_with_ends_with_contains_substr()
{
	std::basic_string_view<char> str = "Hello World";
	assert(str.compare("Hello World") == 0);
	assert(str.compare("Hello") > 0);
	assert(str.compare("Hello World!") < 0);

	assert(str.starts_with("Hello"));
	assert(str.ends_with("World"));
	assert(str.substr(6) == "World");
}

void __stl_string_view_test()
{
	debug("std::basic_string_view  ...");

	debug("std::basic_string_view  equality");
	__stl_basic_string_view_test_equality();

	debug("std::basic_string_view  at and operator[]");
	__stl_basic_string_view_test_at_and_operator();

	debug("std::basic_string_view  front and back");
	__stl_basic_string_view_test_front_back();

	debug("std::basic_string_view  data and c_str");
	__stl_basic_string_view_test_data_c_str();

	debug("std::basic_string_view  begin and end");
	__stl_basic_string_view_test_begin_end();

	debug("std::basic_string_view  size");
	__stl_basic_string_view_test_size();

	debug("std::basic_string_view  find, rfind, find_first_of, find_last_of, find_first_not_of, find_last_not_of");
	__stl_basic_string_view_test_find_rfind_find_first_of_find_last_of_find_first_not_of_find_last_not_of();

	debug("std::basic_string_view  compare, starts_with, ends_with, contains, substr");
	__stl_basic_string_view_test_compare_starts_with_ends_with_contains_substr();

	debug("std::basic_string_view  OK");
}

void test_stl_string()
{
	debug("std::string  ...");

	debug("std::string  equality");
	__stl_string_test_equality();

	debug("std::string  assign");
	__stl_string_test_assign();

	debug("std::string  at and operator[]");
	__stl_string_test_at_and_operator();

	debug("std::string  front and back");
	__stl_string_test_front_back();

	debug("std::string  data and c_str");
	__stl_string_test_data_c_str();

	debug("std::string  begin and end");
	__stl_string_test_begin_end();

	debug("std::string  size, reserve, capacity, shrink_to_fit");
	__stl_string_test_size_reserve_capacity_shrink_to_fit();

	debug("std::string  clear, insert, erase, push_back, pop_back, append, operator+=, copy, resize, swap");
	__stl_string_test_clear_insert_erase_push_back_pop_back_append_operator_plus_equal_copy_resize_swap();

	debug("std::string  find, rfind, find_first_of, find_last_of, find_first_not_of, find_last_not_of");
	__stl_string_test_find_rfind_find_first_of_find_last_of_find_first_not_of_find_last_not_of();

	debug("std::string  compare, starts_with, ends_with, contains, substr");
	__stl_string_test_compare_starts_with_ends_with_contains_substr();

	debug("std::string  OK");

	debug("std::basic_string_view  ...");

	debug("std::basic_string_view  equality");
	__stl_basic_string_view_test_equality();

	debug("std::basic_string_view  at and operator[]");
	__stl_basic_string_view_test_at_and_operator();

	debug("std::basic_string_view  front and back");
	__stl_basic_string_view_test_front_back();

	debug("std::basic_string_view  data and c_str");
	__stl_basic_string_view_test_data_c_str();

	debug("std::basic_string_view  begin and end");
	__stl_basic_string_view_test_begin_end();

	debug("std::basic_string_view  size");
	__stl_basic_string_view_test_size();

	debug("std::basic_string_view  find, rfind, find_first_of, find_last_of, find_first_not_of, find_last_not_of");
	__stl_basic_string_view_test_find_rfind_find_first_of_find_last_of_find_first_not_of_find_last_not_of();

	debug("std::basic_string_view  compare, starts_with, ends_with, contains, substr");
	__stl_basic_string_view_test_compare_starts_with_ends_with_contains_substr();

	debug("std::basic_string_view  OK");
}

#endif // DEBUG
