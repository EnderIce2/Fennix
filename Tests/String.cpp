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

#include <std/string.hpp>
#include <debug.h>

void TestString()
{
    std::string hw("Hello, world!");
    debug("String length: %d", hw.length());
    debug("String capacity: %d", hw.capacity());
    debug("String data: %s", hw.c_str());
    if (hw == "Hello, world!" && hw != "World, hello!")
        debug("String comparison works!");
    else
    {
        error("String comparison doesn't work! \"%s\"", hw.c_str());
        while (1)
            ;
    }

    std::string hi("Hi");
    char chi[3];
    chi[0] = hi[0];
    chi[1] = hi[1];
    chi[2] = '\0';
    if (strcmp(chi, "Hi") == 0)
        debug("String indexing works!");
    else
    {
        error("String indexing doesn't work! \"%s\" \"%s\"", chi, hi.c_str());
        while (1)
            ;
    }

    hi << " there!";
    if (hi == "Hi there!")
        debug("String concatenation works!");
    else
    {
        error("String concatenation doesn't work! \"%s\"", hi.c_str());
        while (1)
            ;
    }

    hi << " " << hw;
    if (hi == "Hi there! Hello, world!")
        debug("String concatenation works!");
    else
    {
        error("String concatenation doesn't work! \"%s\"", hi.c_str());
        while (1)
            ;
    }

    std::string eq0("Hello, world!");
    std::string eq1("Hello, world!");
    std::string eq2("World, hello!");

    if (eq0 == eq1)
        debug("String equality works!");
    else
    {
        error("String equality doesn't work! \"%s\" \"%s\"", eq0.c_str(), eq1.c_str());
        while (1)
            ;
    }

    if (eq0 != eq2)
        debug("String inequality works!");
    else
    {
        error("String inequality doesn't work! \"%s\" \"%s\"", eq0.c_str(), eq2.c_str());
        while (1)
            ;
    }

    char chw[14];
    int i = 0;
    foreach (auto c in hw)
    {
        chw[i] = c;
        i++;
    }
    chw[i] = '\0';

    if (strcmp(chw, "Hello, world!") == 0)
        debug("String iteration works!");
    else
    {
        error("String iteration doesn't work! \"%s\" \"%s\" %d", chw, hw.c_str(), i);
        while (1)
            ;
    }

    std::string a("Hello");
    std::string b("World");
    std::string c;
    c = a + ", " + b + "!";

    if (c == "Hello, World!")
        debug("String addition works!");
    else
    {
        error("String addition doesn't work! \"%s\"", c.c_str());
        while (1)
            ;
    }
}

#endif // DEBUG
