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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <pwd.h>
#include <coreutils.h>

#define MAX_LINE_LEN 1024
#define MAX_HISTORY 128
#define CTRL_KEY(k) ((k) & 0x1F)
#define MAX_COMPLETIONS 128

typedef struct
{
	char *items[MAX_HISTORY];
	int count;
	int Index;
} History;

typedef enum
{
	MODE_INSERT,
	MODE_COMMAND
} InputMode;

typedef struct
{
	char line[MAX_LINE_LEN];
	int cursor;
	int length;
	InputMode mode;
	History history;
	struct termios origTermios;
	char prompt[PATH_MAX + 128];
	int promptLength;
} ShellState;

typedef struct
{
	char *items[MAX_COMPLETIONS];
	int count;
} CompletionList;

static ShellState GlobalShellState;
static int progIsFsh = 0;

void DisableRawMode(ShellState *state);
void SaveHistory(ShellState *state);
void InitializeShell(ShellState *state);
void FreeCompletionList(CompletionList *list);
CompletionList GetCompletions(const char *partial);
void DisplayCompletions(ShellState *state, CompletionList *list);
void UpdatePrompt(ShellState *state);
int GetVisibleLength(const char *str);

void CleanupAndExit(int code)
{
	SaveHistory(&GlobalShellState);
	DisableRawMode(&GlobalShellState);
	exit(code);
}

void HandleSignalInterrupt(int sig)
{
	(void)sig;
	write(STDOUT_FILENO, "\n\r", 2);
	CleanupAndExit(130);
}

void EnableRawMode(ShellState *state)
{
	tcgetattr(STDIN_FILENO, &state->origTermios);
	struct termios raw = state->origTermios;
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 1;
	raw.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void DisableRawMode(ShellState *state)
{
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &state->origTermios);
}

void AddHistory(ShellState *state, const char *line)
{
	if (!line || strlen(line) == 0)
		return;

	char *newEntry = strdup(line);
	if (!newEntry)
		return;

	if (state->history.count < MAX_HISTORY)
		state->history.items[state->history.count++] = newEntry;
	else
	{
		free(state->history.items[0]);
		memmove(state->history.items, state->history.items + 1,
				(MAX_HISTORY - 1) * sizeof(char *));
		state->history.items[MAX_HISTORY - 1] = newEntry;
	}
	state->history.Index = state->history.count;

	SaveHistory(state);
}

void LoadHistory(ShellState *state)
{
	char *home = getenv("HOME");
	if (!home)
		return;

	char path[PATH_MAX];
	snprintf(path, sizeof(path), "%s/.fsh_history", home);

	FILE *fp = fopen(path, "r");
	if (!fp)
		return;

	char **tmpHistory = malloc(MAX_HISTORY * sizeof(char *));
	int tmpCount = 0;

	char line[MAX_LINE_LEN];
	while (fgets(line, sizeof(line), fp))
	{
		size_t len = strlen(line);
		if (len > 0 && line[len - 1] == '\n')
			line[len - 1] = '\0';

		if (strlen(line) > 0)
		{
			if (tmpCount < MAX_HISTORY)
				tmpHistory[tmpCount++] = strdup(line);
			else
			{
				free(tmpHistory[0]);
				memmove(tmpHistory, tmpHistory + 1, (MAX_HISTORY - 1) * sizeof(char *));
				tmpHistory[MAX_HISTORY - 1] = strdup(line);
			}
		}
	}
	fclose(fp);

	for (int i = 0; i < tmpCount; i++)
		state->history.items[i] = tmpHistory[i];
	state->history.count = tmpCount;
	state->history.Index = tmpCount;
	free(tmpHistory);
}

void SaveHistory(ShellState *state)
{
	char *home = getenv("HOME");
	if (!home)
		return;

	char path[PATH_MAX];
	snprintf(path, sizeof(path), "%s/.fsh_history", home);

	int fd = open(path, O_WRONLY | O_APPEND | O_CREAT, 0600);
	if (fd < 0)
		return;

	struct flock fl = {
		.l_type = F_WRLCK,
		.l_whence = SEEK_SET,
		.l_start = 0,
		.l_len = 0};

	if (fcntl(fd, F_SETLKW, &fl) == -1)
	{
		close(fd);
		return;
	}

	if (state->history.count > 0)
	{
		int last = state->history.count - 1;
		if (state->history.items[last])
		{
			char *cmd = state->history.items[last];
			write(fd, cmd, strlen(cmd));
			write(fd, "\n", 1);
		}
	}

	fl.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &fl);
	close(fd);
}

void ProcessViCommand(ShellState *state, char c)
{
	switch (c)
	{
	case 'h':
		if (state->cursor > 0)
			state->cursor--;
		break;
	case 'l':
		if (state->cursor < state->length)
			state->cursor++;
		break;
	case 'k':
		if (state->history.Index > 0)
		{
			state->history.Index--;
			strncpy(state->line, state->history.items[state->history.Index], MAX_LINE_LEN);
			state->length = strlen(state->line);
			state->cursor = state->length;
		}
		break;
	case 'j':
		if (state->history.Index < state->history.count - 1)
		{
			state->history.Index++;
			strncpy(state->line, state->history.items[state->history.Index], MAX_LINE_LEN);
			state->length = strlen(state->line);
			state->cursor = state->length;
		}
		break;
	case 'i':
		state->mode = MODE_INSERT;
		break;
	case 27:
		state->mode = MODE_COMMAND;
		break;
	}
}

void FreeCompletionList(CompletionList *list)
{
	for (int i = 0; i < list->count; i++)
		free(list->items[i]);
	list->count = 0;
}

CompletionList GetCompletions(const char *partial)
{
	CompletionList list = {0};
	char *path = getenv("PATH");
	if (!path)
		return list;

	char *pathCopy = strdup(path);
	char *dir = strtok(pathCopy, ":");

	while (dir && list.count < MAX_COMPLETIONS)
	{
		DIR *d = opendir(dir);
		if (!d)
		{
			dir = strtok(NULL, ":");
			continue;
		}

		struct dirent *entry;
		while ((entry = readdir(d)) && list.count < MAX_COMPLETIONS)
		{
			if (strncmp(entry->d_name, partial, strlen(partial)) == 0)
			{
				char fullPath[PATH_MAX];
				snprintf(fullPath, sizeof(fullPath), "%s/%s", dir, entry->d_name);

				struct stat st;
				if (stat(fullPath, &st) == 0 && (st.st_mode & S_IXUSR))
					list.items[list.count++] = strdup(entry->d_name);
			}
		}
		closedir(d);
		dir = strtok(NULL, ":");
	}

	free(pathCopy);
	return list;
}

void DisplayCompletions(ShellState *state, CompletionList *list)
{
	if (list->count == 0)
		return;

	write(STDOUT_FILENO, "\n\r", 2);

	for (int i = 0; i < list->count; i++)
	{
		write(STDOUT_FILENO, list->items[i], strlen(list->items[i]));
		write(STDOUT_FILENO, "  ", 2);
	}
	write(STDOUT_FILENO, "\n\r", 2);

	write(STDOUT_FILENO, state->prompt, strlen(state->prompt));
	write(STDOUT_FILENO, state->line, state->length);
}

void ReadLine(ShellState *state)
{
	state->length = 0;
	state->cursor = 0;
	state->line[0] = '\0';
	state->mode = MODE_INSERT;

	UpdatePrompt(state);

	write(STDOUT_FILENO, "\r", 1);
	write(STDOUT_FILENO, state->prompt, strlen(state->prompt));

	EnableRawMode(state);

	while (1)
	{
		char c;
		ssize_t nread = read(STDIN_FILENO, &c, 1);
		if (nread <= 0)
			continue;

		if (state->mode == MODE_COMMAND)
			ProcessViCommand(state, c);
		else
		{
			if (c == '\t')
			{

				char *wordStart = state->line;
				for (int i = state->cursor - 1; i >= 0; i--)
				{
					if (state->line[i] == ' ')
					{
						wordStart = &state->line[i + 1];
						break;
					}
				}

				char partial[MAX_LINE_LEN];
				int len = state->cursor - (wordStart - state->line);
				strncpy(partial, wordStart, len);
				partial[len] = '\0';

				CompletionList completions = GetCompletions(partial);

				if (completions.count == 1)
				{

					int restLength = strlen(completions.items[0]) - len;
					if (restLength > 0)
					{
						memmove(&state->line[state->cursor + restLength],
								&state->line[state->cursor],
								state->length - state->cursor + 1);
						memcpy(&state->line[state->cursor],
							   &completions.items[0][len],
							   restLength);
						state->cursor += restLength;
						state->length += restLength;
					}
				}
				else if (completions.count > 1)
				{

					int prefixLength = len;
					int canExtend = 1;

					while (canExtend)
					{
						char nextChar = completions.items[0][prefixLength];
						if (nextChar == '\0')
							break;

						for (int i = 1; i < completions.count; i++)
						{
							if (completions.items[i][prefixLength] != nextChar)
							{
								canExtend = 0;
								break;
							}
						}
						if (canExtend)
							prefixLength++;
					}

					if (prefixLength > len)
					{

						int restLength = prefixLength - len;
						memmove(&state->line[state->cursor + restLength],
								&state->line[state->cursor],
								state->length - state->cursor + 1);
						memcpy(&state->line[state->cursor],
							   &completions.items[0][len],
							   restLength);
						state->cursor += restLength;
						state->length += restLength;
					}

					DisplayCompletions(state, &completions);
				}

				FreeCompletionList(&completions);
				continue;
			}
			else if (c == 0x1B)
			{
				char seq[3];
				if (read(STDIN_FILENO, &seq[0], 1) != 1)
					continue;

				if (read(STDIN_FILENO, &seq[1], 1) != 1)
					continue;

				if (seq[0] == '[')
				{
					switch (seq[1])
					{
					case 'A':
						if (state->history.Index > 0)
						{
							state->history.Index--;
							strncpy(state->line, state->history.items[state->history.Index], MAX_LINE_LEN - 1);
							state->length = strlen(state->line);
							state->cursor = state->length;
						}
						break;
					case 'B':
						if (state->history.Index < state->history.count)
						{
							state->history.Index++;
							if (state->history.Index == state->history.count)
							{
								state->line[0] = '\0';
								state->length = 0;
								state->cursor = 0;
							}
							else
							{
								strncpy(state->line, state->history.items[state->history.Index], MAX_LINE_LEN - 1);
								state->length = strlen(state->line);
								state->cursor = state->length;
							}
						}
						break;
					case 'C':
						if (state->cursor < state->length)
							state->cursor++;
						break;
					case 'D':
						if (state->cursor > 0)
							state->cursor--;
						break;
					case 'H':
						state->cursor = 0;
						break;
					case 'F':
						state->cursor = state->length;
						break;
					case '3':
					{
						if (read(STDIN_FILENO, &seq[2], 1) != 1)
							continue;

						if (seq[2] == '~' && state->cursor < state->length)
						{
							memmove(&state->line[state->cursor], &state->line[state->cursor + 1],
									state->length - state->cursor);
							state->length--;
						}
						break;
					}
					}
				}
				else if (seq[0] == 'O')
				{
					switch (seq[1])
					{
					case 'H':
						state->cursor = 0;
						break;
					case 'F':
						state->cursor = state->length;
						break;
					}
				}
			}
			else if (c == CTRL_KEY('c'))
			{
				write(STDOUT_FILENO, "\n\r", 2);
				write(STDOUT_FILENO, state->prompt, strlen(state->prompt));
				state->length = 0;
				state->cursor = 0;
				state->line[0] = '\0';
			}
			else if (c == '\r' || c == '\n')
			{
				write(STDOUT_FILENO, "\n", 1);
				state->line[state->length] = '\0';
				DisableRawMode(state);
				if (state->length > 0)
				{
					AddHistory(state, state->line);
					state->history.Index = state->history.count;
				}
				return;
			}
			else if (c == 0x7F)
			{
				if (state->cursor > 0)
				{
					memmove(&state->line[state->cursor - 1], &state->line[state->cursor],
							state->length - state->cursor + 1);
					state->cursor--;
					state->length--;
				}
			}
			else if (c == CTRL_KEY('d') && state->length == 0)
			{
				write(STDOUT_FILENO, "\n", 1);
				DisableRawMode(state);
				exit(EXIT_SUCCESS);
			}
			else if (isprint(c) && state->length < MAX_LINE_LEN - 1)
			{
				memmove(&state->line[state->cursor + 1], &state->line[state->cursor],
						state->length - state->cursor + 1);
				state->line[state->cursor] = c;
				state->cursor++;
				state->length++;
			}
		}

		write(STDOUT_FILENO, "\r", 1);
		write(STDOUT_FILENO, "\x1b[K", 3);
		write(STDOUT_FILENO, state->prompt, strlen(state->prompt));
		write(STDOUT_FILENO, state->line, state->length);

		int promptVisibleLength = GetVisibleLength(state->prompt);
		char buf[32];
		snprintf(buf, sizeof(buf), "\r\x1b[%dC", promptVisibleLength + state->cursor);
		write(STDOUT_FILENO, buf, strlen(buf));
	}
}

int GetVisibleLength(const char *str)
{
	int len = 0;
	int inEscape = 0;

	while (*str)
	{
		if (*str == '\x1b')
			inEscape = 1;
		else if (inEscape)
		{
			if ((*str >= 'A' && *str <= 'Z') || (*str >= 'a' && *str <= 'z'))
				inEscape = 0;
		}
		else
			len++;
		str++;
	}
	return len;
}

void UpdatePrompt(ShellState *state)
{
	char hostname[256] = {0};
	gethostname(hostname, sizeof(hostname));

	struct passwd *pw = getpwuid(getuid());
	char *username = pw ? pw->pw_name : "user";

	char cwd[PATH_MAX];
	if (!getcwd(cwd, sizeof(cwd)))
		strcpy(cwd, "~");

	if (pw && pw->pw_dir && strncmp(cwd, pw->pw_dir, strlen(pw->pw_dir)) == 0)
	{
		size_t home_len = strlen(pw->pw_dir);
		if (strlen(cwd) == home_len)
			strcpy(cwd, "~");
		else if (cwd[home_len] == '/')
		{
			memmove(cwd + 1, cwd + home_len, strlen(cwd) - home_len + 1);
			cwd[0] = '~';
		}
	}

	char *customPrompt = getenv("SHELL_PROMPT");
	if (customPrompt)
	{
		snprintf(state->prompt, sizeof(state->prompt), "%s", customPrompt);
	}
	else
	{
		// snprintf(state->prompt, sizeof(state->prompt),
		// 		 "\x1b[;32m┌──(%s@%s)-[\x1b[0;1m%s\x1b[;32m]\n\r└─\x1b[;32m$\x1b[00m ",
		// 		 username, hostname, cwd);

		// snprintf(state->prompt, sizeof(state->prompt),
		// 		 "\x1b[;32m%s@%s\x1b[0;1m:\x1b[01;34m%s\x1b[0;1m$\x1b[00m ",
		// 		 username, hostname, cwd);

		if (progIsFsh)
		{
			snprintf(state->prompt, sizeof(state->prompt),
					 "\x1b[1;34m%s\x1b[0;1m:\x1b[01;35m%s\x1b[0;1m$\x1b[00m ",
					 username, cwd);
		}
		else
		{
			snprintf(state->prompt, sizeof(state->prompt),
					 "$ ");
		}
	}

	state->promptLength = GetVisibleLength(state->prompt);
}

void ExecuteCommand(char **args)
{
	if (!args[0])
		return;

	for (int i = 0; args[i] != NULL; i++)
	{
		char *arg = args[i];
		size_t len = strlen(arg);
		if (len >= 2 && arg[0] == '"' && arg[len - 1] == '"')
		{
			arg[len - 1] = '\0';
			memmove(arg, arg + 1, len - 1);
		}
	}

	if (strcmp(args[0], "exit") == 0)
		exit(0);
	else if (strcmp(args[0], "cd") == 0)
	{
		char *targetDirectory = args[1];
		if (!targetDirectory || strcmp(targetDirectory, "~") == 0)
		{
			struct passwd *pw = getpwuid(getuid());
			if (pw && pw->pw_dir)
				targetDirectory = pw->pw_dir;
			else
			{
				fprintf(stderr, "cd: HOME not set and no password directory available\n");
				return;
			}
		}
		else if (targetDirectory[0] == '~')
		{
			struct passwd *pw = getpwuid(getuid());
			if (pw && pw->pw_dir)
			{
				char newPath[PATH_MAX];
				snprintf(newPath, sizeof(newPath), "%s%s", pw->pw_dir, targetDirectory + 1);
				targetDirectory = newPath;
			}
		}

		if (chdir(targetDirectory))
			perror("cd");
		else
			UpdatePrompt(&GlobalShellState);
		return;
	}

	pid_t pid = fork();
	if (pid == 0)
	{
		struct termios term;
		tcgetattr(STDIN_FILENO, &term);
		term.c_lflag |= (ECHO | ICANON | IEXTEN | ISIG);
		term.c_oflag |= (OPOST | ONLCR);
		tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
		write(STDOUT_FILENO, "\r", 1);

		execvp(args[0], args);
		if (errno == ENOENT)
		{
			write(STDERR_FILENO, "\r", 1);
			fprintf(stderr, "%s: command not found\n", args[0]);
		}
		else
		{
			write(STDERR_FILENO, "\r", 1);
			perror(args[0]);
		}
		exit(EXIT_FAILURE);
	}
	else if (pid > 0)
	{
		int status;
		waitpid(pid, &status, 0);
		write(STDOUT_FILENO, "\r\n", 2);
	}
	else
		perror("fork");
}

void ShellLoop(ShellState *state)
{
	signal(SIGINT, HandleSignalInterrupt);
	LoadHistory(state);

	while (1)
	{
		ReadLine(state);
		if (state->length == 0)
			continue;

		char *trimmed = state->line;
		while (*trimmed && isspace(*trimmed))
			trimmed++;

		char *end = trimmed + strlen(trimmed) - 1;
		while (end > trimmed && isspace(*end))
			*end-- = '\0';

		if (*trimmed == '\0')
			continue;

		memmove(state->line, trimmed, strlen(trimmed) + 1);
		state->length = strlen(state->line);

		char *args[MAX_LINE_LEN / 2 + 1];
		int i = 0;
		char *p = state->line;
		int inQuotes = 0;
		char *start = p;

		while (*p)
		{
			if (*p == '"')
				inQuotes = !inQuotes;
			else if ((*p == ' ' || *p == '\t') && !inQuotes)
			{
				if (p > start)
				{
					*p = '\0';
					args[i++] = start;
				}
				start = p + 1;
			}
			p++;
		}

		if (p > start)
			args[i++] = start;
		args[i] = NULL;

		if (i > 0)
			ExecuteCommand(args);
	}
}

void InitializeShell(ShellState *state)
{
	memset(state, 0, sizeof(ShellState));
	UpdatePrompt(state);
	LoadHistory(state);
	signal(SIGINT, HandleSignalInterrupt);

	printf("\x1b[01;35mFennix Shell v%s\n\r", PROGRAM_VERSION);
	printf("\x1b[;31mEarly development version!\x1b[0m\n\r");
}

void DisableRawModeAtExit(void)
{
	DisableRawMode(&GlobalShellState);
}

void PrintHelp()
{
	printf("Usage: sh [OPTION]... [SCRIPT]\n");
	printf("A simple shell implementation.\n\n");
	printf("Options:\n");
	printf("  -c COMMAND    execute COMMAND and exit\n");
	printf("  --help        display this help and exit\n");
	printf("  --version     output version information and exit\n\n");
	printf("If SCRIPT is provided, execute commands from the script file.\n");
	printf("Otherwise, run in interactive mode.\n\n");
	printf("Environment variables:\n");
	printf("  SHELL_PROMPT  custom prompt format (default: user@host:path$ )\n");
}

int main(int argc, char *argv[])
{
	if (argc > 1)
	{
		if (strcmp(argv[1], "--help") == 0)
		{
			PrintHelp();
			exit(EXIT_SUCCESS);
		}
		else if (strcmp(argv[1], "--version") == 0)
		{
			PRINTF_VERSION;
			exit(EXIT_SUCCESS);
		}
	}

	char *basename = strrchr(argv[0], '/');
	if (basename == NULL)
		basename = argv[0];
	else
		basename++;
	if (strcmp(basename, "fsh") == 0)
		progIsFsh = 1;

	memset(&GlobalShellState, 0, sizeof(ShellState));
	InitializeShell(&GlobalShellState);

	signal(SIGINT, HandleSignalInterrupt);
	atexit(DisableRawModeAtExit);

	if (argc > 1)
	{
		if (strcmp(argv[1], "-c") == 0)
			ExecuteCommand(&argv[2]);
		else
		{
			FILE *fp = fopen(argv[1], "r");
			if (!fp)
			{
				perror("fopen");
				CleanupAndExit(EXIT_FAILURE);
			}

			char line[MAX_LINE_LEN];
			while (fgets(line, sizeof(line), fp))
			{
				line[strcspn(line, "\n")] = '\0';
				char *args[] = {"/bin/sh", "-c", line, NULL};
				ExecuteCommand(args);
			}
			fclose(fp);
		}
	}
	else
		ShellLoop(&GlobalShellState);

	CleanupAndExit(EXIT_SUCCESS);
	return EXIT_SUCCESS;
}
