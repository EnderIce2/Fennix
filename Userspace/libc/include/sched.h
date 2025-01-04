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

#ifndef _SCHED_H
#define _SCHED_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <time.h>

	typedef struct sched_param
	{
		int sched_priority;					  /* Process or thread execution scheduling priority. */
		int sched_ss_low_priority;			  /* Low scheduling priority for sporadic server. */
		struct timespec sched_ss_repl_period; /* Replenishment period for sporadic server. */
		struct timespec sched_ss_init_budget; /* Initial budget for sporadic server. */
		int sched_ss_max_repl;				  /* Maximum pending replenishments for sporadic server. */
	} sched_param_t;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_SCHED_H
