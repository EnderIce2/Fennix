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
