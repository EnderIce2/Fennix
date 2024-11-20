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

#include <assert.h>
#include <stdexcept>
#include <exception>
#include <printf.h>

nsa void __stl_test_exception0()
{
	try
	{
		throw;
	}
	catch (std::exception &e)
	{
		debug("caught exception");
	}
}

nsa void __stl_test_exception1()
{
	try
	{
		throw std::out_of_range("out of range");
	}
	catch (...)
	{
		debug("caught out_of_range");
	}
}

nsa void __stl_test_exception2()
{
	try
	{
		throw std::out_of_range("out of range");
	}
	catch (std::out_of_range &e)
	{
		debug("caught: %s", e.what());
	}
}

nsa void __stl_test_exception3()
{
	try
	{
		throw std::exception();
	}
	catch (std::out_of_range &e)
	{
		debug("caught out_of_range: %s", e.what());
	}
}

nsa void __stl_test_exception4()
{
	try
	{
		throw std::out_of_range("test");
	}
	catch (std::out_of_range &e)
	{
		debug("caught out_of_range: %s", e.what());
	}
	catch (std::exception &e)
	{
		debug("caught exception: %s", e.what());
	}
	catch (...)
	{
		debug("caught ...");
	}
}

nsa void __stl_test_exception5()
{
	throw std::out_of_range("test");
}

namespace test_class
{
	class MyTestClass
	{
	public:
		MyTestClass() = default;
		~MyTestClass() = default;

		MyTestClass(const char *str)
		{
			throw std::out_of_range(str);
		}

		void throwTest()
		{
			throw std::out_of_range("Throw Test Exception");
		}

		void throwTest2();
	};

	void MyTestClass::throwTest2()
	{
		throw std::out_of_range("Throw Test as Method");
	}
}

nsa void __stl_test_exception6_0()
{
	try
	{
		test_class::MyTestClass test("Hello World!");
	}
	catch (std::out_of_range &e)
	{
		debug("caught out_of_range: %s", e.what());
	}
}

nsa void __stl_test_exception6_1()
{
	test_class::MyTestClass test;
	try
	{
		test.throwTest();
	}
	catch (std::out_of_range &e)
	{
		debug("caught out_of_range: %s", e.what());
	}

	try
	{
		test.throwTest2();
	}
	catch (std::out_of_range &e)
	{
		debug("caught out_of_range: %s", e.what());
	}
}

nsa void __stl_test_exception6()
{
	__stl_test_exception6_0();
	__stl_test_exception6_1();
}

void test_stl_exception()
{
	debug("C++ exception  ...");

	fixme("C++ exception tests are not implemented");
	// __stl_test_exception0();
	// __stl_test_exception1();
	// __stl_test_exception2();
	// __stl_test_exception3();
	// __stl_test_exception4();
	// __stl_test_exception5();
	// __stl_test_exception6();

	debug("C++ exception  OK");
}

#endif // DEBUG
