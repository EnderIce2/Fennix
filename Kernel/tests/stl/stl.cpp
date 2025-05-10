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

void test_stl_exception();
void test_stl_iostream();
void test_stl_cmath() {}
void test_stl_list();
void test_stl_vector();
void test_stl_bitset();
void test_stl_string();
void test_stl_unordered_map() {}
void test_stl_future();
void test_stl_array();
void test_stl_shared_ptr();

void Test_stl()
{
	test_stl_exception();
	test_stl_iostream();
	test_stl_cmath();
	test_stl_list();
	test_stl_vector();
	test_stl_bitset();
	test_stl_string();
	test_stl_unordered_map();
	test_stl_future();
	test_stl_array();
	test_stl_shared_ptr();
}

#endif // DEBUG
