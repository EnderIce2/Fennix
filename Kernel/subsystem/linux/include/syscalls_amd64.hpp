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

#ifndef __FENNIX_KERNEL_LINUX_SYSCALLS_x64_H__
#define __FENNIX_KERNEL_LINUX_SYSCALLS_x64_H__

#define __NR_amd64_read 0
#define __NR_amd64_write 1
#define __NR_amd64_open 2
#define __NR_amd64_close 3
#define __NR_amd64_stat 4
#define __NR_amd64_fstat 5
#define __NR_amd64_lstat 6
#define __NR_amd64_poll 7
#define __NR_amd64_lseek 8
#define __NR_amd64_mmap 9
#define __NR_amd64_mprotect 10
#define __NR_amd64_munmap 11
#define __NR_amd64_brk 12
#define __NR_amd64_rt_sigaction 13
#define __NR_amd64_rt_sigprocmask 14
#define __NR_amd64_rt_sigreturn 15
#define __NR_amd64_ioctl 16
#define __NR_amd64_pread64 17
#define __NR_amd64_pwrite64 18
#define __NR_amd64_readv 19
#define __NR_amd64_writev 20
#define __NR_amd64_access 21
#define __NR_amd64_pipe 22
#define __NR_amd64_select 23
#define __NR_amd64_sched_yield 24
#define __NR_amd64_mremap 25
#define __NR_amd64_msync 26
#define __NR_amd64_mincore 27
#define __NR_amd64_madvise 28
#define __NR_amd64_shmget 29
#define __NR_amd64_shmat 30
#define __NR_amd64_shmctl 31
#define __NR_amd64_dup 32
#define __NR_amd64_dup2 33
#define __NR_amd64_pause 34
#define __NR_amd64_nanosleep 35
#define __NR_amd64_getitimer 36
#define __NR_amd64_alarm 37
#define __NR_amd64_setitimer 38
#define __NR_amd64_getpid 39
#define __NR_amd64_sendfile 40
#define __NR_amd64_socket 41
#define __NR_amd64_connect 42
#define __NR_amd64_accept 43
#define __NR_amd64_sendto 44
#define __NR_amd64_recvfrom 45
#define __NR_amd64_sendmsg 46
#define __NR_amd64_recvmsg 47
#define __NR_amd64_shutdown 48
#define __NR_amd64_bind 49
#define __NR_amd64_listen 50
#define __NR_amd64_getsockname 51
#define __NR_amd64_getpeername 52
#define __NR_amd64_socketpair 53
#define __NR_amd64_setsockopt 54
#define __NR_amd64_getsockopt 55
#define __NR_amd64_clone 56
#define __NR_amd64_fork 57
#define __NR_amd64_vfork 58
#define __NR_amd64_execve 59
#define __NR_amd64_exit 60
#define __NR_amd64_wait4 61
#define __NR_amd64_kill 62
#define __NR_amd64_uname 63
#define __NR_amd64_semget 64
#define __NR_amd64_semop 65
#define __NR_amd64_semctl 66
#define __NR_amd64_shmdt 67
#define __NR_amd64_msgget 68
#define __NR_amd64_msgsnd 69
#define __NR_amd64_msgrcv 70
#define __NR_amd64_msgctl 71
#define __NR_amd64_fcntl 72
#define __NR_amd64_flock 73
#define __NR_amd64_fsync 74
#define __NR_amd64_fdatasync 75
#define __NR_amd64_truncate 76
#define __NR_amd64_ftruncate 77
#define __NR_amd64_getdents 78
#define __NR_amd64_getcwd 79
#define __NR_amd64_chdir 80
#define __NR_amd64_fchdir 81
#define __NR_amd64_rename 82
#define __NR_amd64_mkdir 83
#define __NR_amd64_rmdir 84
#define __NR_amd64_creat 85
#define __NR_amd64_link 86
#define __NR_amd64_unlink 87
#define __NR_amd64_symlink 88
#define __NR_amd64_readlink 89
#define __NR_amd64_chmod 90
#define __NR_amd64_fchmod 91
#define __NR_amd64_chown 92
#define __NR_amd64_fchown 93
#define __NR_amd64_lchown 94
#define __NR_amd64_umask 95
#define __NR_amd64_gettimeofday 96
#define __NR_amd64_getrlimit 97
#define __NR_amd64_getrusage 98
#define __NR_amd64_sysinfo 99
#define __NR_amd64_times 100
#define __NR_amd64_ptrace 101
#define __NR_amd64_getuid 102
#define __NR_amd64_syslog 103
#define __NR_amd64_getgid 104
#define __NR_amd64_setuid 105
#define __NR_amd64_setgid 106
#define __NR_amd64_geteuid 107
#define __NR_amd64_getegid 108
#define __NR_amd64_setpgid 109
#define __NR_amd64_getppid 110
#define __NR_amd64_getpgrp 111
#define __NR_amd64_setsid 112
#define __NR_amd64_setreuid 113
#define __NR_amd64_setregid 114
#define __NR_amd64_getgroups 115
#define __NR_amd64_setgroups 116
#define __NR_amd64_setresuid 117
#define __NR_amd64_getresuid 118
#define __NR_amd64_setresgid 119
#define __NR_amd64_getresgid 120
#define __NR_amd64_getpgid 121
#define __NR_amd64_setfsuid 122
#define __NR_amd64_setfsgid 123
#define __NR_amd64_getsid 124
#define __NR_amd64_capget 125
#define __NR_amd64_capset 126
#define __NR_amd64_rt_sigpending 127
#define __NR_amd64_rt_sigtimedwait 128
#define __NR_amd64_rt_sigqueueinfo 129
#define __NR_amd64_rt_sigsuspend 130
#define __NR_amd64_sigaltstack 131
#define __NR_amd64_utime 132
#define __NR_amd64_mknod 133
#define __NR_amd64_uselib 134
#define __NR_amd64_personality 135
#define __NR_amd64_ustat 136
#define __NR_amd64_statfs 137
#define __NR_amd64_fstatfs 138
#define __NR_amd64_sysfs 139
#define __NR_amd64_getpriority 140
#define __NR_amd64_setpriority 141
#define __NR_amd64_sched_setparam 142
#define __NR_amd64_sched_getparam 143
#define __NR_amd64_sched_setscheduler 144
#define __NR_amd64_sched_getscheduler 145
#define __NR_amd64_sched_get_priority_max 146
#define __NR_amd64_sched_get_priority_min 147
#define __NR_amd64_sched_rr_get_interval 148
#define __NR_amd64_mlock 149
#define __NR_amd64_munlock 150
#define __NR_amd64_mlockall 151
#define __NR_amd64_munlockall 152
#define __NR_amd64_vhangup 153
#define __NR_amd64_modify_ldt 154
#define __NR_amd64_pivot_root 155
#define __NR_amd64__sysctl 156
#define __NR_amd64_prctl 157
#define __NR_amd64_arch_prctl 158
#define __NR_amd64_adjtimex 159
#define __NR_amd64_setrlimit 160
#define __NR_amd64_chroot 161
#define __NR_amd64_sync 162
#define __NR_amd64_acct 163
#define __NR_amd64_settimeofday 164
#define __NR_amd64_mount 165
#define __NR_amd64_umount2 166
#define __NR_amd64_swapon 167
#define __NR_amd64_swapoff 168
#define __NR_amd64_reboot 169
#define __NR_amd64_sethostname 170
#define __NR_amd64_setdomainname 171
#define __NR_amd64_iopl 172
#define __NR_amd64_ioperm 173
#define __NR_amd64_create_module 174
#define __NR_amd64_init_module 175
#define __NR_amd64_delete_module 176
#define __NR_amd64_get_kernel_syms 177
#define __NR_amd64_query_module 178
#define __NR_amd64_quotactl 179
#define __NR_amd64_nfsservctl 180
#define __NR_amd64_getpmsg 181
#define __NR_amd64_putpmsg 182
#define __NR_amd64_afs_syscall 183
#define __NR_amd64_tuxcall 184
#define __NR_amd64_security 185
#define __NR_amd64_gettid 186
#define __NR_amd64_readahead 187
#define __NR_amd64_setxattr 188
#define __NR_amd64_lsetxattr 189
#define __NR_amd64_fsetxattr 190
#define __NR_amd64_getxattr 191
#define __NR_amd64_lgetxattr 192
#define __NR_amd64_fgetxattr 193
#define __NR_amd64_listxattr 194
#define __NR_amd64_llistxattr 195
#define __NR_amd64_flistxattr 196
#define __NR_amd64_removexattr 197
#define __NR_amd64_lremovexattr 198
#define __NR_amd64_fremovexattr 199
#define __NR_amd64_tkill 200
#define __NR_amd64_time 201
#define __NR_amd64_futex 202
#define __NR_amd64_sched_setaffinity 203
#define __NR_amd64_sched_getaffinity 204
#define __NR_amd64_set_thread_area 205
#define __NR_amd64_io_setup 206
#define __NR_amd64_io_destroy 207
#define __NR_amd64_io_getevents 208
#define __NR_amd64_io_submit 209
#define __NR_amd64_io_cancel 210
#define __NR_amd64_get_thread_area 211
#define __NR_amd64_lookup_dcookie 212
#define __NR_amd64_epoll_create 213
#define __NR_amd64_epoll_ctl_old 214
#define __NR_amd64_epoll_wait_old 215
#define __NR_amd64_remap_file_pages 216
#define __NR_amd64_getdents64 217
#define __NR_amd64_set_tid_address 218
#define __NR_amd64_restart_syscall 219
#define __NR_amd64_semtimedop 220
#define __NR_amd64_fadvise64 221
#define __NR_amd64_timer_create 222
#define __NR_amd64_timer_settime 223
#define __NR_amd64_timer_gettime 224
#define __NR_amd64_timer_getoverrun 225
#define __NR_amd64_timer_delete 226
#define __NR_amd64_clock_settime 227
#define __NR_amd64_clock_gettime 228
#define __NR_amd64_clock_getres 229
#define __NR_amd64_clock_nanosleep 230
#define __NR_amd64_exit_group 231
#define __NR_amd64_epoll_wait 232
#define __NR_amd64_epoll_ctl 233
#define __NR_amd64_tgkill 234
#define __NR_amd64_utimes 235
#define __NR_amd64_vserver 236
#define __NR_amd64_mbind 237
#define __NR_amd64_set_mempolicy 238
#define __NR_amd64_get_mempolicy 239
#define __NR_amd64_mq_open 240
#define __NR_amd64_mq_unlink 241
#define __NR_amd64_mq_timedsend 242
#define __NR_amd64_mq_timedreceive 243
#define __NR_amd64_mq_notify 244
#define __NR_amd64_mq_getsetattr 245
#define __NR_amd64_kexec_load 246
#define __NR_amd64_waitid 247
#define __NR_amd64_add_key 248
#define __NR_amd64_request_key 249
#define __NR_amd64_keyctl 250
#define __NR_amd64_ioprio_set 251
#define __NR_amd64_ioprio_get 252
#define __NR_amd64_inotify_init 253
#define __NR_amd64_inotify_add_watch 254
#define __NR_amd64_inotify_rm_watch 255
#define __NR_amd64_migrate_pages 256
#define __NR_amd64_openat 257
#define __NR_amd64_mkdirat 258
#define __NR_amd64_mknodat 259
#define __NR_amd64_fchownat 260
#define __NR_amd64_futimesat 261
#define __NR_amd64_newfstatat 262
#define __NR_amd64_unlinkat 263
#define __NR_amd64_renameat 264
#define __NR_amd64_linkat 265
#define __NR_amd64_symlinkat 266
#define __NR_amd64_readlinkat 267
#define __NR_amd64_fchmodat 268
#define __NR_amd64_faccessat 269
#define __NR_amd64_pselect6 270
#define __NR_amd64_ppoll 271
#define __NR_amd64_unshare 272
#define __NR_amd64_set_robust_list 273
#define __NR_amd64_get_robust_list 274
#define __NR_amd64_splice 275
#define __NR_amd64_tee 276
#define __NR_amd64_sync_file_range 277
#define __NR_amd64_vmsplice 278
#define __NR_amd64_move_pages 279
#define __NR_amd64_utimensat 280
#define __NR_amd64_epoll_pwait 281
#define __NR_amd64_signalfd 282
#define __NR_amd64_timerfd_create 283
#define __NR_amd64_eventfd 284
#define __NR_amd64_fallocate 285
#define __NR_amd64_timerfd_settime 286
#define __NR_amd64_timerfd_gettime 287
#define __NR_amd64_accept4 288
#define __NR_amd64_signalfd4 289
#define __NR_amd64_eventfd2 290
#define __NR_amd64_epoll_create1 291
#define __NR_amd64_dup3 292
#define __NR_amd64_pipe2 293
#define __NR_amd64_inotify_init1 294
#define __NR_amd64_preadv 295
#define __NR_amd64_pwritev 296
#define __NR_amd64_rt_tgsigqueueinfo 297
#define __NR_amd64_perf_event_open 298
#define __NR_amd64_recvmmsg 299
#define __NR_amd64_fanotify_init 300
#define __NR_amd64_fanotify_mark 301
#define __NR_amd64_prlimit64 302
#define __NR_amd64_name_to_handle_at 303
#define __NR_amd64_open_by_handle_at 304
#define __NR_amd64_clock_adjtime 305
#define __NR_amd64_syncfs 306
#define __NR_amd64_sendmmsg 307
#define __NR_amd64_setns 308
#define __NR_amd64_getcpu 309
#define __NR_amd64_process_vm_readv 310
#define __NR_amd64_process_vm_writev 311
#define __NR_amd64_kcmp 312
#define __NR_amd64_finit_module 313
#define __NR_amd64_sched_setattr 314
#define __NR_amd64_sched_getattr 315
#define __NR_amd64_renameat2 316
#define __NR_amd64_seccomp 317
#define __NR_amd64_getrandom 318
#define __NR_amd64_memfd_create 319
#define __NR_amd64_kexec_file_load 320
#define __NR_amd64_bpf 321
#define __NR_amd64_execveat 322
#define __NR_amd64_userfaultfd 323
#define __NR_amd64_membarrier 324
#define __NR_amd64_mlock2 325
#define __NR_amd64_copy_file_range 326
#define __NR_amd64_preadv2 327
#define __NR_amd64_pwritev2 328
#define __NR_amd64_pkey_mprotect 329
#define __NR_amd64_pkey_alloc 330
#define __NR_amd64_pkey_free 331
#define __NR_amd64_statx 332
#define __NR_amd64_io_pgetevents 333
#define __NR_amd64_rseq 334
#define __NR_amd64_pidfd_send_signal 424
#define __NR_amd64_io_uring_setup 425
#define __NR_amd64_io_uring_enter 426
#define __NR_amd64_io_uring_register 427
#define __NR_amd64_open_tree 428
#define __NR_amd64_move_mount 429
#define __NR_amd64_fsopen 430
#define __NR_amd64_fsconfig 431
#define __NR_amd64_fsmount 432
#define __NR_amd64_fspick 433
#define __NR_amd64_pidfd_open 434
#define __NR_amd64_clone3 435
#define __NR_amd64_close_range 436
#define __NR_amd64_openat2 437
#define __NR_amd64_pidfd_getfd 438
#define __NR_amd64_faccessat2 439
#define __NR_amd64_process_madvise 440
#define __NR_amd64_epoll_pwait2 441
#define __NR_amd64_mount_setattr 442
#define __NR_amd64_landlock_create_ruleset 444
#define __NR_amd64_landlock_add_rule 445
#define __NR_amd64_landlock_restrict_self 446

#endif // !__FENNIX_KERNEL_LINUX_SYSCALLS_x64_H__
