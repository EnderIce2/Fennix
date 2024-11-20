#include <ctype.h>

#include <sys/types.h> // For PUBLIC

PUBLIC int tolower(int c)
{
    if (c >= 'A' && c <= 'Z')
    {
        c -= 'A';
        c += 'a';
    }

    return c;
}

PUBLIC int toupper(int c)
{
    if (c >= 'a' && c <= 'z')
    {
        c -= 'a';
        c += 'A';
    }
    return c;
}

PUBLIC int isspace(int c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\v';
}
