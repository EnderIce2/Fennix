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

#define MAX_LINE_LEN 1024
#define MAX_HISTORY 128
#define CTRL_KEY(k) ((k) & 0x1f)

typedef struct
{
	char *items[MAX_HISTORY];
	int count;
	int index;
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
	int len;
	InputMode mode;
	History history;
	struct termios orig_termios;
} ShellState;

void HandleSignalInterrupt(int sig)
{
	(void)sig;
	write(STDOUT_FILENO, "\n", 1);
}

void EnableRawMode(ShellState *state)
{
	tcgetattr(STDIN_FILENO, &state->orig_termios);
	struct termios raw = state->orig_termios;
	raw.c_lflag &= ~(ECHO | ICANON);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void DisableRawMode(ShellState *state)
{
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &state->orig_termios);
}

void AddHistory(ShellState *state, const char *line)
{
	if (state->history.count < MAX_HISTORY)
		state->history.items[state->history.count++] = strdup(line);
	else
	{
		free(state->history.items[0]);
		memmove(state->history.items, state->history.items + 1,
				(MAX_HISTORY - 1) * sizeof(char *));
		state->history.items[MAX_HISTORY - 1] = strdup(line);
	}
	state->history.index = state->history.count;
}

void LoadHistory(ShellState *state)
{
	char *home = getenv("HOME");
	if (!home)
		return;

	char path[256];
	snprintf(path, sizeof(path), "%s/.sh_history", home);

	FILE *fp = fopen(path, "r");
	if (!fp)
		return;

	char line[MAX_LINE_LEN];
	while (fgets(line, sizeof(line), fp))
	{
		line[strcspn(line, "\n")] = '\0';
		AddHistory(state, line);
	}
	fclose(fp);
}

void SaveHistory(ShellState *state)
{
	char *home = getenv("HOME");
	if (!home)
		return;

	char path[256];
	snprintf(path, sizeof(path), "%s/.sh_history", home);

	FILE *fp = fopen(path, "w");
	if (!fp)
		return;

	for (int i = 0; i < state->history.count; i++)
	{
		fprintf(fp, "%s\n", state->history.items[i]);
	}
	fclose(fp);
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
		if (state->cursor < state->len)
			state->cursor++;
		break;
	case 'k':
	{
		if (state->history.index > 0)
		{
			state->history.index--;
			strcpy(state->line, state->history.items[state->history.index]);
			state->len = strlen(state->line);
			state->cursor = state->len;
		}
		break;
	}
	case 'j':
	{
		if (state->history.index < state->history.count - 1)
		{
			state->history.index++;
			strcpy(state->line, state->history.items[state->history.index]);
			state->len = strlen(state->line);
			state->cursor = state->len;
		}
		break;
	}
	case 'i':
		state->mode = MODE_INSERT;
		break;
	case 27:
		state->mode = MODE_COMMAND;
		break;
	}
}

void ReadLine(ShellState *state)
{
	state->len = 0;
	state->cursor = 0;
	state->line[0] = '\0';
	state->mode = MODE_INSERT;

	printf("$ ");
	fflush(stdout);

	EnableRawMode(state);

	while (1)
	{
		char c = '\0';
		if (read(STDIN_FILENO, &c, 1) != 1)
			break;

		if (state->mode == MODE_COMMAND)
			ProcessViCommand(state, c);
		else
		{
			if (c == CTRL_KEY('c'))
			{
				printf("\n");
				return;
			}
			else if (c == '\n')
			{
				printf("\n");
				break;
			}
			else if (c == 127)
			{
				if (state->cursor > 0)
				{
					memmove(&state->line[state->cursor - 1],
							&state->line[state->cursor],
							state->len - state->cursor + 1);
					state->cursor--;
					state->len--;
				}
			}
			else if (state->len < MAX_LINE_LEN - 1)
			{
				memmove(&state->line[state->cursor + 1],
						&state->line[state->cursor],
						state->len - state->cursor + 1);
				state->line[state->cursor] = c;
				state->cursor++;
				state->len++;
			}
		}

		printf("\r\x1B[2K$ %s", state->line);
		fflush(stdout);
		printf("\r\x1B[%dC", state->cursor + 3);
	}

	DisableRawMode(state);
	AddHistory(state, state->line);
}

void ExecuteCommand(char **args)
{
	if (args[0] == NULL)
		return;

	if (strcmp(args[0], "exit") == 0)
		exit(0);
	if (strcmp(args[0], "cd") == 0)
	{
		if (args[1] == NULL)
			fprintf(stderr, "cd: missing argument\n");
		else if (chdir(args[1]))
			perror("cd");
		return;
	}

	pid_t pid = fork();
	if (pid == 0)
	{
		execvp(args[0], args);
		perror("execvp");
		exit(EXIT_FAILURE);
	}
	else if (pid > 0)
		wait(NULL);
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
		if (state->len == 0)
			continue;

		char *args[MAX_LINE_LEN / 2 + 1];
		char *token = strtok(state->line, " ");
		int i = 0;
		while (token != NULL)
		{
			args[i++] = token;
			token = strtok(NULL, " ");
		}
		args[i] = NULL;

		ExecuteCommand(args);
	}

	SaveHistory(state);
}

void InitializeShell(ShellState *state)
{
	memset(state, 0, sizeof(ShellState));
	LoadHistory(state);
	signal(SIGINT, HandleSignalInterrupt);
}

int main(int argc, char *argv[])
{
	ShellState state = {0};
	InitializeShell(&state);

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
				exit(EXIT_FAILURE);
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
		ShellLoop(&state);

	return EXIT_SUCCESS;
}
