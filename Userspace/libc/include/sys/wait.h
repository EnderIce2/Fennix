#ifndef _SYS_WAIT_H
#define _SYS_WAIT_H

#include <sys/types.h>

typedef enum
{
	P_ALL, /* Wait for any child. */
	P_PID, /* Wait for specified process. */
	P_PGID /* Wait for members of process group. */
} idtype_t;

typedef struct
{
	int stub;
} siginfo_t;

#include <bits/waitstatus.h>

#define WCONTINUED 1
#define WNOHANG 2
#define WUNTRACED 4
#define WEXITED 8
#define WNOWAIT 16
#define WSTOPPED 32

/**
 * @brief Macro for extracting the exit status from a status value.
 *
 * If the child process terminated normally by calling exit(3) or _exit(2),
 * the macro WEXITSTATUS() returns the low-order 8 bits of the status value.
 *
 * @param status The status value to extract the exit status from.
 * @return The exit status of the child process.
 */
#define WEXITSTATUS(status) __WEXITSTATUS(status)

/**
 * @brief Macro for extracting the termination signal from a status value.
 *
 * If the child process was terminated by a signal, the macro WTERMSIG()
 * returns the signal number of the terminating signal.
 *
 * @param status The status value to extract the termination signal from.
 * @return The termination signal of the child process.
 */
#define WTERMSIG(status) __WTERMSIG(status)

/**
 * @brief Macro for extracting the stop signal from a status value.
 *
 * If the child process was stopped by a signal, the macro WSTOPSIG()
 * returns the signal number of the stop signal.
 *
 * @param status The status value to extract the stop signal from.
 * @return The stop signal of the child process.
 */
#define WSTOPSIG(status) __WSTOPSIG(status)

/**
 * @brief Macro for testing whether a process exited normally.
 *
 * If the child process terminated normally by calling exit(3) or _exit(2),
 * the macro WIFEXITED() returns a nonzero value. Otherwise, it returns 0.
 *
 * @param status The status value to test.
 * @return A nonzero value if the child process exited normally, 0 otherwise.
 */
#define WIFEXITED(status) __WIFEXITED(status)

/**
 * @brief Macro for testing whether a process was terminated by a signal.
 *
 * If the child process was terminated by a signal, the macro WIFSIGNALED()
 * returns a nonzero value. Otherwise, it returns 0.
 *
 * @param status The status value to test.
 * @return A nonzero value if the child process was terminated by a signal, 0 otherwise.
 */
#define WIFSIGNALED(status) __WIFSIGNALED(status)

/**
 * @brief Macro for testing whether a process was stopped by a signal.
 *
 * If the child process was stopped by a signal, the macro WIFSTOPPED()
 * returns a nonzero value. Otherwise, it returns 0.
 *
 * @param status The status value to test.
 * @return A nonzero value if the child process was stopped by a signal, 0 otherwise.
 */
#define WIFSTOPPED(status) __WIFSTOPPED(status)

/**
 * @brief Macro for testing whether a stopped process was continued.
 *
 * If the child process was stopped and has been resumed by delivery of SIGCONT,
 * the macro WIFCONTINUED() returns a nonzero value. Otherwise, it returns 0.
 *
 * @param status The status value to test.
 * @return A nonzero value if the child process was continued, 0 otherwise.
 */
#define WIFCONTINUED(status) __WIFCONTINUED(status)

pid_t wait(int *wstatus);
pid_t waitpid(pid_t pid, int *wstatus, int options);
int waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options);

#endif
