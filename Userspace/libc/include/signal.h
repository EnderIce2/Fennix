#ifndef _SIGNAL_H
#define _SIGNAL_H

#include <bits/types/sigset_t.h>

/* https://www-uxsup.csx.cam.ac.uk/courses/moved.Building/signals.pdf */
/* https://man7.org/linux/man-pages/man7/signal.7.html */

/** @brief Hangup detected on controlling or death of controlling process */
#define SIGHUP 1

/** @brief Interrupt from keyboard */
#define SIGINT 2

/** @brief Quit from keyboard */
#define SIGQUIT 3

/** @brief Illegal Instruction */
#define SIGILL 4

/** @brief Trace/breakpoint trap */
#define SIGTRAP 5

/** @brief Abort signal from abort(3) */
#define SIGABRT 6

/** @brief Bus error (bad memory access) */
#define SIGBUS 7

/** @brief Floating point exception */
#define SIGFPE 8

/** @brief Kill signal */
#define SIGKILL 9

/** @brief User-defined signal 1 */
#define SIGUSR1 10

/** @brief Invalid memory reference */
#define SIGSEGV 11

/** @brief User-defined signal 2 */
#define SIGUSR2 12

/** @brief Broken pipe: write to pipe with no readers */
#define SIGPIPE 13

/** @brief Timer signal from alarm(2) */
#define SIGALRM 14

/** @brief Termination signal */
#define SIGTERM 15

/** @brief Stack fault on coprocessor (unused) */
#define SIGSTKFLT 16

/** @brief Child stopped or terminated */
#define SIGCHLD 17

/** @brief Continue if stopped */
#define SIGCONT 18

/** @brief Stop process */
#define SIGSTOP 19

/** @brief Stop typed at terminal */
#define SIGTSTP 20

/** @brief Terminal input for background process */
#define SIGTTIN 21

/** @brief Terminal output for background process */
#define SIGTTOU 22

/** @brief Urgent condition on socket */
#define SIGURG 23

/** @brief CPU time limit exceeded */
#define SIGXCPU 24

/** @brief File size limit exceeded */
#define SIGXFSZ 25

/** @brief Virtual timer expired */
#define SIGVTALRM 26

/** @brief Profiling timer expired */
#define SIGPROF 27

/** @brief Window resize signal */
#define SIGWINCH 28

/** @brief I/O now possible */
#define SIGIO 29

/** @brief Power failure */
#define SIGPWR 30

/** @brief Bad system call */
#define SIGSYS 31

#endif // !_SIGNAL_H
