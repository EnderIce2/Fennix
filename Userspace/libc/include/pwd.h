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

#ifndef _PWD_H
#define _PWD_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <sys/types.h>

	struct passwd
	{
		char *pw_name;	/* User's login name. */
		uid_t pw_uid;	/* Numerical user ID. */
		gid_t pw_gid;	/* Numerical group ID. */
		char *pw_dir;	/* Initial working directory. */
		char *pw_shell; /* Program to use as shell. */
	};

	void endpwent(void);
	struct passwd *getpwent(void);
	struct passwd *getpwnam(const char *);
	int getpwnam_r(const char *, struct passwd *, char *, size_t, struct passwd **);
	struct passwd *getpwuid(uid_t);
	int getpwuid_r(uid_t, struct passwd *, char *, size_t, struct passwd **);
	void setpwent(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_PWD_H
