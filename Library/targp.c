#include <targp.h>

#include <memory.hpp>
#include <convert.h>

void targp_parse(const char *cmd, char **argv, int *argc)
{
    const char delim[] = " ";

    char *token = strtok((char *)cmd, delim);
    while (token != NULL)
    {
        char *quote = strchr(token, '"');
        if (quote != NULL)
        {
            memmove(quote, quote + 1, strlen(quote));
            char *endQuote = strchr(quote, '"');
            if (endQuote != NULL)
                *endQuote = '\0';
        }

        char *arg = (char *)kmalloc(strlen(token) + 1);
        strcpy(arg, token);

        argv[(*argc)++] = arg;

        token = strtok(NULL, delim);
    }
    argv[(*argc)] = NULL;
}
