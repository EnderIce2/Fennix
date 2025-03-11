/*
	This file is part of Fennix Core Utilities.

	Fennix Core Utilities is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Core Utilities is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Core Utilities. If not, see <https://www.gnu.org/licenses/>.
*/

#include <coreutils.h>

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

void PrintHelp()
{
	printf("Usage: echo [OPTION]... [STRING]...\n");
	printf("Echo the STRING(s) to standard output.\n\n");
	printf("  -n          do not output the trailing newline\n");
	printf("  -e          enable interpretation of backslash escapes\n");
	printf("  -E          disable interpretation of backslash escapes (default)\n");
	printf("  --help      display this help and exit\n");
	printf("  --version   output version information and exit\n\n");
	printf("If -e is specified, the following sequences are recognized:\n");
	printf("  \\\\  backslash\n");
	printf("  \\a  alert (BEL)\n");
	printf("  \\b  backspace\n");
	printf("  \\c  produce no further output\n");
	printf("  \\e  escape\n");
	printf("  \\f  form feed\n");
	printf("  \\n  new line\n");
	printf("  \\r  carriage return\n");
	printf("  \\t  horizontal tab\n");
	printf("  \\v  vertical tab\n");
}

static void PrintEscaped(const char *str)
{
	while (*str)
	{
		if (*str == '\\')
		{
			str++;
			switch (*str)
			{
			case 'n':
				putchar('\n');
				break;
			case 't':
				putchar('\t');
				break;
			case '\\':
				putchar('\\');
				break;
			case 'a':
				putchar('\a');
				break;
			case 'b':
				putchar('\b');
				break;
			case 'r':
				putchar('\r');
				break;
			case 'v':
				putchar('\v');
				break;
			case 'f':
				putchar('\f');
				break;
			case '0' ... '7':
			{
				int octal = 0;
				for (int i = 0; i < 3 && *str >= '0' && *str <= '7'; i++, str++)
				{
					octal = octal * 8 + (*str - '0');
				}
				putchar(octal);
				str--;
				break;
			}
			case 'x':
			{
				int hex = 0;
				str++;
				for (int i = 0; i < 2 && ((*str >= '0' && *str <= '9') ||
										  (*str >= 'a' && *str <= 'f') ||
										  (*str >= 'A' && *str <= 'F'));
					 i++, str++)
				{
					if (*str >= '0' && *str <= '9')
						hex = hex * 16 + (*str - '0');
					else if (*str >= 'a' && *str <= 'f')
						hex = hex * 16 + (*str - 'a' + 10);
					else if (*str >= 'A' && *str <= 'F')
						hex = hex * 16 + (*str - 'A' + 10);
				}
				putchar(hex);
				str--;
				break;
			}
			default:
				putchar(*str);
				break;
			}
		}
		else
		{
			putchar(*str);
		}
		str++;
	}
}

int main(int argc, char *argv[])
{
	bool newline = true;
	bool interpret_escapes = false;
	int arg_start = 1;

	if (argc == 2)
	{
		if (strcmp(argv[1], "--help") == 0)
		{
			PrintHelp();
			exit(EXIT_SUCCESS);
		}
		else if (strcmp(argv[1], "--version") == 0)
		{
			PRINTF_VERSION;
			exit(EXIT_SUCCESS);
		}
	}

	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			if (argv[i][0] == '-' && argv[i][1] != '\0')
			{
				for (size_t j = 1; argv[i][j] != '\0'; j++)
				{
					if (argv[i][j] == 'n')
						newline = false;
					else if (argv[i][j] == 'e')
						interpret_escapes = true;
					else if (argv[i][j] == 'E')
						interpret_escapes = false;
					else
						goto print_args;
				}
				arg_start++;
			}
			else
			{
				break;
			}
		}
	}

print_args:
	for (int i = arg_start; i < argc; i++)
	{
		if (interpret_escapes)
			PrintEscaped(argv[i]);
		else
			fputs(argv[i], stdout);
		if (i < argc - 1)
			putchar(' ');
	}

	if (newline)
		putchar('\n');
	return 0;
}
