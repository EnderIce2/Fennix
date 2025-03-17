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

#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H

#pragma region Syscall Wrappers

#define scarg __UINTPTR_TYPE__

static inline scarg syscall0(scarg syscall)
{
	scarg ret;
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall)
						 : "rcx", "r11", "memory");
	return ret;
}

static inline scarg syscall1(scarg syscall, scarg arg1)
{
	scarg ret;
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall), "D"(arg1)
						 : "rcx", "r11", "memory");
	return ret;
}

static inline scarg syscall2(scarg syscall, scarg arg1, scarg arg2)
{
	scarg ret;
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall), "D"(arg1), "S"(arg2)
						 : "rcx", "r11", "memory");
	return ret;
}

static inline scarg syscall3(scarg syscall, scarg arg1, scarg arg2, scarg arg3)
{
	scarg ret;
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3)
						 : "rcx", "r11", "memory");
	return ret;
}

static inline scarg syscall4(scarg syscall, scarg arg1, scarg arg2, scarg arg3, scarg arg4)
{
	scarg ret;
	register scarg r10 __asm__("r10") = arg4;
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10)
						 : "rcx", "r11", "memory");
	return ret;
}

static inline scarg syscall5(scarg syscall, scarg arg1, scarg arg2, scarg arg3, scarg arg4, scarg arg5)
{
	scarg ret;
	register scarg r10 __asm__("r10") = arg4;
	register scarg r8 __asm__("r8") = arg5;
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10), "r"(r8)
						 : "rcx", "r11", "memory");
	return ret;
}

static inline scarg syscall6(scarg syscall, scarg arg1, scarg arg2, scarg arg3, scarg arg4, scarg arg5, scarg arg6)
{
	scarg ret;
	register scarg r10 __asm__("r10") = arg4;
	register scarg r8 __asm__("r8") = arg5;
	register scarg r9 __asm__("r9") = arg6;
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10), "r"(r8), "r"(r9)
						 : "rcx", "r11", "memory");
	return ret;
}

#pragma endregion Syscall Wrappers

#define sys_read 0
#define sys_write 1
#define sys_open 2
#define sys_close 3
#define sys_stat 4
#define sys_fstat 5
#define sys_lstat 6
#define sys_poll 7
#define sys_lseek 8
#define sys_mmap 9
#define sys_mprotect 10
#define sys_munmap 11
#define sys_brk 12
#define sys_rt_sigaction 13
#define sys_rt_sigprocmask 14
#define sys_rt_sigreturn 15
#define sys_ioctl 16
#define sys_pread64 17
#define sys_pwrite64 18
#define sys_readv 19
#define sys_writev 20
#define sys_access 21
#define sys_pipe 22
#define sys_select 23
#define sys_sched_yield 24
#define sys_mremap 25
#define sys_msync 26
#define sys_mincore 27
#define sys_madvise 28
#define sys_shmget 29
#define sys_shmat 30
#define sys_shmctl 31
#define sys_dup 32
#define sys_dup2 33
#define sys_pause 34
#define sys_nanosleep 35
#define sys_getitimer 36
#define sys_alarm 37
#define sys_setitimer 38
#define sys_getpid 39
#define sys_sendfile 40
#define sys_socket 41
#define sys_connect 42
#define sys_accept 43
#define sys_sendto 44
#define sys_recvfrom 45
#define sys_sendmsg 46
#define sys_recvmsg 47
#define sys_shutdown 48
#define sys_bind 49
#define sys_listen 50
#define sys_getsockname 51
#define sys_getpeername 52
#define sys_socketpair 53
#define sys_setsockopt 54
#define sys_getsockopt 55
#define sys_clone 56
#define sys_fork 57
#define sys_vfork 58
#define sys_execve 59
#define sys_exit 60
#define sys_wait4 61
#define sys_kill 62
#define sys_uname 63
#define sys_semget 64
#define sys_semop 65
#define sys_semctl 66
#define sys_shmdt 67
#define sys_msgget 68
#define sys_msgsnd 69
#define sys_msgrcv 70
#define sys_msgctl 71
#define sys_fcntl 72
#define sys_flock 73
#define sys_fsync 74
#define sys_fdatasync 75
#define sys_truncate 76
#define sys_ftruncate 77
#define sys_getdents 78
#define sys_getcwd 79
#define sys_chdir 80
#define sys_fchdir 81
#define sys_rename 82
#define sys_mkdir 83
#define sys_rmdir 84
#define sys_creat 85
#define sys_link 86
#define sys_unlink 87
#define sys_symlink 88
#define sys_readlink 89
#define sys_chmod 90
#define sys_fchmod 91
#define sys_chown 92
#define sys_fchown 93
#define sys_lchown 94
#define sys_umask 95
#define sys_gettimeofday 96
#define sys_getrlimit 97
#define sys_getrusage 98
#define sys_sysinfo 99
#define sys_times 100
#define sys_ptrace 101
#define sys_getuid 102
#define sys_syslog 103
#define sys_getgid 104
#define sys_setuid 105
#define sys_setgid 106
#define sys_geteuid 107
#define sys_getegid 108
#define sys_setpgid 109
#define sys_getppid 110
#define sys_getpgrp 111
#define sys_setsid 112
#define sys_setreuid 113
#define sys_setregid 114
#define sys_getgroups 115
#define sys_setgroups 116
#define sys_setresuid 117
#define sys_getresuid 118
#define sys_setresgid 119
#define sys_getresgid 120
#define sys_getpgid 121
#define sys_setfsuid 122
#define sys_setfsgid 123
#define sys_getsid 124
#define sys_capget 125
#define sys_capset 126
#define sys_rt_sigpending 127
#define sys_rt_sigtimedwait 128
#define sys_rt_sigqueueinfo 129
#define sys_rt_sigsuspend 130
#define sys_sigaltstack 131
#define sys_utime 132
#define sys_mknod 133
#define sys_uselib 134
#define sys_personality 135
#define sys_ustat 136
#define sys_statfs 137
#define sys_fstatfs 138
#define sys_sysfs 139
#define sys_getpriority 140
#define sys_setpriority 141
#define sys_sched_setparam 142
#define sys_sched_getparam 143
#define sys_sched_setscheduler 144
#define sys_sched_getscheduler 145
#define sys_sched_get_priority_max 146
#define sys_sched_get_priority_min 147
#define sys_sched_rr_get_interval 148
#define sys_mlock 149
#define sys_munlock 150
#define sys_mlockall 151
#define sys_munlockall 152
#define sys_vhangup 153
#define sys_modify_ldt 154
#define sys_pivot_root 155
#define sys__sysctl 156
#define sys_prctl 157
#define sys_arch_prctl 158
#define sys_adjtimex 159
#define sys_setrlimit 160
#define sys_chroot 161
#define sys_sync 162
#define sys_acct 163
#define sys_settimeofday 164
#define sys_mount 165
#define sys_umount2 166
#define sys_swapon 167
#define sys_swapoff 168
#define sys_reboot 169
#define sys_sethostname 170
#define sys_setdomainname 171
#define sys_iopl 172
#define sys_ioperm 173
#define sys_create_module 174
#define sys_init_module 175
#define sys_delete_module 176
#define sys_get_kernel_syms 177
#define sys_query_module 178
#define sys_quotactl 179
#define sys_nfsservctl 180
#define sys_getpmsg 181
#define sys_putpmsg 182
#define sys_afs_syscall 183
#define sys_tuxcall 184
#define sys_security 185
#define sys_gettid 186
#define sys_readahead 187
#define sys_setxattr 188
#define sys_lsetxattr 189
#define sys_fsetxattr 190
#define sys_getxattr 191
#define sys_lgetxattr 192
#define sys_fgetxattr 193
#define sys_listxattr 194
#define sys_llistxattr 195
#define sys_flistxattr 196
#define sys_removexattr 197
#define sys_lremovexattr 198
#define sys_fremovexattr 199
#define sys_tkill 200
#define sys_time 201
#define sys_futex 202
#define sys_sched_setaffinity 203
#define sys_sched_getaffinity 204
#define sys_set_thread_area 205
#define sys_io_setup 206
#define sys_io_destroy 207
#define sys_io_getevents 208
#define sys_io_submit 209
#define sys_io_cancel 210
#define sys_get_thread_area 211
#define sys_lookup_dcookie 212
#define sys_epoll_create 213
#define sys_epoll_ctl_old 214
#define sys_epoll_wait_old 215
#define sys_remap_file_pages 216
#define sys_getdents64 217
#define sys_set_tid_address 218
#define sys_restart_syscall 219
#define sys_semtimedop 220
#define sys_fadvise64 221
#define sys_timer_create 222
#define sys_timer_settime 223
#define sys_timer_gettime 224
#define sys_timer_getoverrun 225
#define sys_timer_delete 226
#define sys_clock_settime 227
#define sys_clock_gettime 228
#define sys_clock_getres 229
#define sys_clock_nanosleep 230
#define sys_exit_group 231
#define sys_epoll_wait 232
#define sys_epoll_ctl 233
#define sys_tgkill 234
#define sys_utimes 235
#define sys_vserver 236
#define sys_mbind 237
#define sys_set_mempolicy 238
#define sys_get_mempolicy 239
#define sys_mq_open 240
#define sys_mq_unlink 241
#define sys_mq_timedsend 242
#define sys_mq_timedreceive 243
#define sys_mq_notify 244
#define sys_mq_getsetattr 245
#define sys_kexec_load 246
#define sys_waitid 247
#define sys_add_key 248
#define sys_request_key 249
#define sys_keyctl 250
#define sys_ioprio_set 251
#define sys_ioprio_get 252
#define sys_inotify_init 253
#define sys_inotify_add_watch 254
#define sys_inotify_rm_watch 255
#define sys_migrate_pages 256
#define sys_openat 257
#define sys_mkdirat 258
#define sys_mknodat 259
#define sys_fchownat 260
#define sys_futimesat 261
#define sys_newfstatat 262
#define sys_unlinkat 263
#define sys_renameat 264
#define sys_linkat 265
#define sys_symlinkat 266
#define sys_readlinkat 267
#define sys_fchmodat 268
#define sys_faccessat 269
#define sys_pselect6 270
#define sys_ppoll 271
#define sys_unshare 272
#define sys_set_robust_list 273
#define sys_get_robust_list 274
#define sys_splice 275
#define sys_tee 276
#define sys_sync_file_range 277
#define sys_vmsplice 278
#define sys_move_pages 279
#define sys_utimensat 280
#define sys_epoll_pwait 281
#define sys_signalfd 282
#define sys_timerfd_create 283
#define sys_eventfd 284
#define sys_fallocate 285
#define sys_timerfd_settime 286
#define sys_timerfd_gettime 287
#define sys_accept4 288
#define sys_signalfd4 289
#define sys_eventfd2 290
#define sys_epoll_create1 291
#define sys_dup3 292
#define sys_pipe2 293
#define sys_inotify_init1 294
#define sys_preadv 295
#define sys_pwritev 296
#define sys_rt_tgsigqueueinfo 297
#define sys_perf_event_open 298
#define sys_recvmmsg 299
#define sys_fanotify_init 300
#define sys_fanotify_mark 301
#define sys_prlimit64 302
#define sys_name_to_handle_at 303
#define sys_open_by_handle_at 304
#define sys_clock_adjtime 305
#define sys_syncfs 306
#define sys_sendmmsg 307
#define sys_setns 308
#define sys_getcpu 309
#define sys_process_vm_readv 310
#define sys_process_vm_writev 311
#define sys_kcmp 312
#define sys_finit_module 313
#define sys_sched_setattr 314
#define sys_sched_getattr 315
#define sys_renameat2 316
#define sys_seccomp 317
#define sys_getrandom 318
#define sys_memfd_create 319
#define sys_kexec_file_load 320
#define sys_bpf 321
#define sys_execveat 322
#define sys_userfaultfd 323
#define sys_membarrier 324
#define sys_mlock2 325
#define sys_copy_file_range 326
#define sys_preadv2 327
#define sys_pwritev2 328
#define sys_pkey_mprotect 329
#define sys_pkey_alloc 330
#define sys_pkey_free 331
#define sys_statx 332
#define sys_io_pgetevents 333
#define sys_rseq 334
#define sys_pidfd_send_signal 424
#define sys_io_uring_setup 425
#define sys_io_uring_enter 426
#define sys_io_uring_register 427
#define sys_open_tree 428
#define sys_move_mount 429
#define sys_fsopen 430
#define sys_fsconfig 431
#define sys_fsmount 432
#define sys_fspick 433
#define sys_pidfd_open 434
#define sys_clone3 435
#define sys_close_range 436
#define sys_openat2 437
#define sys_pidfd_getfd 438
#define sys_faccessat2 439
#define sys_process_madvise 440
#define sys_epoll_pwait2 441
#define sys_mount_setattr 442
#define sys_quotactl_fd 443
#define sys_landlock_create_ruleset 444
#define sys_landlock_add_rule 445
#define sys_landlock_restrict_self 446
#define sys_memfd_secret 447
#define sys_process_mrelease 448

struct kutsname
{
	char sysname[65];
	char nodename[65];
	char release[65];
	char version[65];
	char machine[65];
	char domainname[65];
};

#endif // _BITS_SYSCALLS_H
