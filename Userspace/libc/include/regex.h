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

#ifndef _REGEX_H
#define _REGEX_H

#include <sys/types.h>
#include <stddef.h>

typedef struct
{
	size_t re_nsub; /* Number of parenthesized subexpressions */
} regex_t;

typedef ptrdiff_t regoff_t;

typedef struct
{
	regoff_t rm_so; /* Byte offset to start of substring */
	regoff_t rm_eo; /* Byte offset after end of substring */
} regmatch_t;

/* Flags for regcomp() */
#define REG_EXTENDED 0x01 /* Use Extended Regular Expressions */
#define REG_ICASE 0x02	  /* Case-insensitive matching */
#define REG_MINIMAL 0x04  /* Leftmost shortest match (for REG_EXTENDED) */
#define REG_NOSUB 0x08	  /* Suppress subexpression reporting */
#define REG_NEWLINE 0x10  /* Alter newline handling */

/* Flags for regexec() */
#define REG_NOTBOL 0x01 /* ^ does not match start of string */
#define REG_NOTEOL 0x02 /* $ does not match end of string */

/* Error codes */
#define REG_NOMATCH 1  /* No match found */
#define REG_BADPAT 2   /* Invalid regular expression */
#define REG_ECOLLATE 3 /* Invalid collating element */
#define REG_ECTYPE 4   /* Invalid character class */
#define REG_EESCAPE 5  /* Trailing backslash */
#define REG_ESUBREG 6  /* Invalid backreference */
#define REG_EBRACK 7   /* Unbalanced '[]' */
#define REG_EPAREN 8   /* Unbalanced '()' */
#define REG_EBRACE 9   /* Unbalanced '{}' */
#define REG_BADBR 10   /* Invalid content in {} */
#define REG_ERANGE 11  /* Invalid range endpoint */
#define REG_ESPACE 12  /* Memory exhaustion */
#define REG_BADRPT 13  /* Invalid repetition operator */

int regcomp(regex_t *restrict preg, const char *restrict pattern, int cflags);
size_t regerror(int errcode, const regex_t *restrict preg, char *restrict errbuf, size_t errbuf_size);
int regexec(const regex_t *restrict preg, const char *restrict string, size_t nmatch, regmatch_t pmatch[restrict], int eflags);
void regfree(regex_t *preg);

#endif /* _REGEX_H */
