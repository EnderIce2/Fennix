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

#ifndef _LOCALE_H
#define _LOCALE_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

	typedef struct lconv
	{
		char *currency_symbol;
		char *decimal_point;
		char frac_digits;
		char *grouping;
		char *int_curr_symbol;
		char int_frac_digits;
		char int_n_cs_precedes;
		char int_n_sep_by_space;
		char int_n_sign_posn;
		char int_p_cs_precedes;
		char int_p_sep_by_space;
		char int_p_sign_posn;
		char *mon_decimal_point;
		char *mon_grouping;
		char *mon_thousands_sep;
		char *negative_sign;
		char n_cs_precedes;
		char n_sep_by_space;
		char n_sign_posn;
		char *positive_sign;
		char p_cs_precedes;
		char p_sep_by_space;
		char p_sign_posn;
		char *thousands_sep;
	} lconv;

	typedef struct locale_t
	{
		char data[1]; /* FIXME: implement locale_t */
	} locale_t;

#define LC_ALL
#define LC_COLLATE
#define LC_CTYPE
#define LC_MESSAGES
#define LC_MONETARY
#define LC_NUMERIC
#define LC_TIME

	locale_t duplocale(locale_t);
	void freelocale(locale_t);
	const char *getlocalename_l(int, locale_t);
	struct lconv *localeconv(void);
	locale_t newlocale(int, const char *, locale_t);
	char *setlocale(int, const char *);
	locale_t uselocale(locale_t);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_LOCALE_H
