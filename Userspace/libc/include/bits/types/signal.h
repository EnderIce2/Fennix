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

#ifndef __BITS_TYPES_SIGNAL_H
#define __BITS_TYPES_SIGNAL_H

union sigval
{
	int sival_int;	 /* Integer signal value. */
	void *sival_ptr; /* Pointer signal value. */
};

typedef struct sigevent
{
	int sigev_notify;							 /* Notification type. */
	int sigev_signo;							 /* Signal number. */
	union sigval sigev_value;					 /* Signal value. */
	void (*sigev_notify_function)(union sigval); /* Notification function. */
	pthread_attr_t *sigev_notify_attributes;	 /* Notification attributes. */
} sigevent;

#endif // __BITS_TYPES_SIGNAL_H
