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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

static int TestFile(const char *path, char mode)
{
	struct stat st;
	if (stat(path, &st) != 0)
		return 1;

	switch (mode)
	{
	case 'b':
		return S_ISBLK(st.st_mode) ? 0 : 1;
	case 'c':
		return S_ISCHR(st.st_mode) ? 0 : 1;
	case 'd':
		return S_ISDIR(st.st_mode) ? 0 : 1;
	case 'e':
		return 0;
	case 'f':
		return S_ISREG(st.st_mode) ? 0 : 1;
	case 'g':
		return (st.st_mode & S_ISGID) ? 0 : 1;
	case 'h':
	case 'L':
		return lstat(path, &st) == 0 && S_ISLNK(st.st_mode) ? 0 : 1;
	case 'p':
		return S_ISFIFO(st.st_mode) ? 0 : 1;
	case 'r':
		return access(path, R_OK) == 0 ? 0 : 1;
	case 's':
		return st.st_size > 0 ? 0 : 1;
	case 'u':
		return (st.st_mode & S_ISUID) ? 0 : 1;
	case 'w':
		return access(path, W_OK) == 0 ? 0 : 1;
	case 'x':
		return access(path, X_OK) == 0 ? 0 : 1;
	default:
		return 2;
	}
}

static int TestString(const char *s1, const char *op, const char *s2)
{
	if (!strcmp(op, "="))
		return strcmp(s1, s2) == 0 ? 0 : 1;
	if (!strcmp(op, "!="))
		return strcmp(s1, s2) != 0 ? 0 : 1;
	if (!strcmp(op, "<"))
		return strcmp(s1, s2) < 0 ? 0 : 1;
	if (!strcmp(op, ">"))
		return strcmp(s1, s2) > 0 ? 0 : 1;
	return 2;
}

static int TestInteger(const char *n1, const char *op, const char *n2)
{
	int i1 = atoi(n1), i2 = atoi(n2);
	if (!strcmp(op, "-eq"))
		return i1 == i2 ? 0 : 1;
	if (!strcmp(op, "-ne"))
		return i1 != i2 ? 0 : 1;
	if (!strcmp(op, "-gt"))
		return i1 > i2 ? 0 : 1;
	if (!strcmp(op, "-ge"))
		return i1 >= i2 ? 0 : 1;
	if (!strcmp(op, "-lt"))
		return i1 < i2 ? 0 : 1;
	if (!strcmp(op, "-le"))
		return i1 <= i2 ? 0 : 1;
	return 2;
}

int main(int argc, char *argv[])
{
	char *base = strrchr(argv[0], '/');
	base = base ? base + 1 : argv[0];
	int isBracketForm = (strcmp(base, "[") == 0);

	if (isBracketForm)
	{
		if (argc < 2 || strcmp(argv[argc - 1], "]") != 0)
		{
			fprintf(stderr, "Error: missing closing bracket ']'.\n");
			return 2;
		}
		argc--;
	}

	if (argc == 1)
		return 1;

	if (argc == 2)
		return argv[1][0] ? 0 : 1;

	if (argc == 3)
	{
		if (!strcmp(argv[1], "!"))
			return argv[2][0] ? 1 : 0;

		return TestFile(argv[2], argv[1][1]);
	}

	if (argc == 4)
	{
		if (!strcmp(argv[1], "!"))
			return !main(3, &argv[1]);

		if (strchr("=!<>", argv[2][0]))
			return TestString(argv[1], argv[2], argv[3]);

		return TestInteger(argv[1], argv[2], argv[3]);
	}

	return 2;
}
