/*
	This file is part of Fennix Kernel.

	Fennix Kernel is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Kernel is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Kernel. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef __FENNIX_KERNEL_SHELL_CMDS_H__
#define __FENNIX_KERNEL_SHELL_CMDS_H__

#include <types.h>

void cmd_lsof(const char *args);
void cmd_clear(const char *args);
void cmd_echo(const char *args);
void cmd_ls(const char *args);
void cmd_tree(const char *args);
void cmd_cd(const char *args);
void cmd_cat(const char *args);
void cmd_ps(const char *args);
void cmd_uptime(const char *args);
void cmd_whoami(const char *args);
void cmd_uname(const char *args);
void cmd_mem(const char *args);
void cmd_kill(const char *args);
void cmd_killall(const char *args);
void cmd_top(const char *args);
void cmd_exit(const char *args);
void cmd_shutdown(const char *args);
void cmd_reboot(const char *args);
void cmd_lspci(const char *args);
void cmd_lsacpi(const char *args);
void cmd_lsmod(const char *args);
void cmd_modinfo(const char *args);
void cmd_panic(const char *args);
void cmd_dump(const char *args);

#define IF_ARG(x) strcmp(args, x) == 0

#endif // !__FENNIX_KERNEL_SHELL_CMDS_H__
