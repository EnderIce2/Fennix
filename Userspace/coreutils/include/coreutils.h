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

#ifndef _COREUTILS_H
#define _COREUTILS_H

#ifndef PROGRAM_NAME
#define PROGRAM_NAME "<unknown>"
#endif

#ifndef PROGRAM_VERSION
#define PROGRAM_VERSION "<unknown>"
#endif

#define BUILD_YEAR (__DATE__ + 7)

#define PRINTF_VERSION                                                          \
	printf("%s %s\n", PROGRAM_NAME, PROGRAM_VERSION);                           \
	printf("Fennix Core Utilities  Copyright (C) %s  EnderIce2\n", BUILD_YEAR); \
	printf("This program comes with ABSOLUTELY NO WARRANTY\n");                 \
	printf("This is free software, and you are welcome to redistribute it\n");  \
	printf("under certain conditions\n")

#endif // _COREUTILS_H
