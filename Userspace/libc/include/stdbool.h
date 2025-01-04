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

#ifndef _STDBOOL_H
#define _STDBOOL_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#define bool _Bool
#define true 1
#define false 0

#define __bool_true_false_are_defined 1

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_STDBOOL_H
