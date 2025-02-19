/*
	This file is part of Fennix C Library.

	Fennix C Library is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix C Library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix C Library. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _CTYPE_H
#define _CTYPE_H

#define isalnum(c) ((c) >= '0' && (c) <= '9' || (c) >= 'A' && (c) <= 'Z' || (c) >= 'a' && (c) <= 'z')
#define isalpha(c) ((c) >= 'A' && (c) <= 'Z' || (c) >= 'a' && (c) <= 'z')
#define isblank(c) ((c) == ' ' || (c) == '\t')
#define iscntrl(c) ((c) < 32 || (c) == 127)
#define isdigit(c) ((c) >= '0' && (c) <= '9')
#define isgraph(c) ((c) > 32 && (c) < 127)
#define islower(c) ((c) >= 'a' && (c) <= 'z')
#define isprint(c) ((c) >= 32 && (c) < 127)
#define ispunct(c) (isgraph(c) && !isalnum(c))
#define isspace(c) ((c) == ' ' || (c) == '\f' || (c) == '\n' || (c) == '\r' || (c) == '\t' || (c) == '\v')
#define isupper(c) ((c) >= 'A' && (c) <= 'Z')
#define isxdigit(c) (isdigit(c) || ((c) >= 'A' && (c) <= 'F') || ((c) >= 'a' && (c) <= 'f'))
#define tolower(c) (isupper(c) ? (c) + 'a' - 'A' : (c))
#define toupper(c) (islower(c) ? (c) + 'A' - 'a' : (c))

/* FIXME: locale */
#define isalnum_l(c, l) isalnum(c)
#define isalpha_l(c, l) isalpha(c)
#define isblank_l(c, l) isblank(c)
#define iscntrl_l(c, l) iscntrl(c)
#define isdigit_l(c, l) isdigit(c)
#define isgraph_l(c, l) isgraph(c)
#define islower_l(c, l) islower(c)
#define isprint_l(c, l) isprint(c)
#define ispunct_l(c, l) ispunct(c)
#define isspace_l(c, l) isspace(c)
#define isupper_l(c, l) isupper(c)
#define isxdigit_l(c, l) isxdigit(c)
#define tolower_l(c, l) tolower(c)
#define toupper_l(c, l) toupper(c)

#endif // _CTYPE_H
