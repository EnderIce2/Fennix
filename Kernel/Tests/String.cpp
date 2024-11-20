#ifdef DEBUG

#include <string.hpp>
#include <debug.h>

void TestString()
{
    String hw("Hello, world!");
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

    String hi("Hi");
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

    String eq0("Hello, world!");
    String eq1("Hello, world!");
    String eq2("World, hello!");

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

    String a("Hello");
    String b("World");
    String c;
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
