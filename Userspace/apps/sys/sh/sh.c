/*
	This file is part of Fennix Userspace.

	Fennix Userspace is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Userspace is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Userspace. If not, see <https://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARG_SIZE 100

void print_prompt()
{
	printf("$ ");
	fflush(stdout);
}

void read_input(char *input)
{
	if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL)
	{
		perror("fgets");
		exit(EXIT_FAILURE);
	}
	input[strcspn(input, "\n")] = '\0';
}

void parse_input(char *input, char **args)
{
	int i = 0;
	args[i] = strtok(input, " ");
	while (args[i] != NULL)
	{
		i++;
		args[i] = strtok(NULL, " ");
	}
}

void execute_command(char **args)
{
	pid_t pid = fork();
	if (pid < 0)
	{
		perror("fork");
		exit(EXIT_FAILURE);
	}
	else if (pid == 0)
	{
		if (execvp(args[0], args) < 0)
		{
			perror("execvp");
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		int status;
		waitpid(pid, &status, 0);
		if (WIFEXITED(status))
		{
			printf("Child exited with status %d\n", WEXITSTATUS(status));
		}
		else if (WIFSIGNALED(status))
		{
			printf("Child terminated by signal %d\n", WTERMSIG(status));
		}
	}
}

void handle_signal(int sig)
{
	if (sig == SIGINT)
	{
		printf("\nCaught signal %d (SIGINT). Type 'exit' to quit the shell.\n", sig);
		print_prompt();
		fflush(stdout);
	}
}

int main()
{
	char input[MAX_INPUT_SIZE];
	char *args[MAX_ARG_SIZE];

	signal(SIGINT, handle_signal);

	while (1)
	{
		print_prompt();
		read_input(input);
		if (strcmp(input, "exit") == 0)
			break;
		parse_input(input, args);
		if (args[0] != NULL)
			execute_command(args);
	}

	return 0;
}
