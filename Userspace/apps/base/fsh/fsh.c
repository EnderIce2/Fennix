#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#define _GNU_SOURCE
#include <signal.h>
#include <termios.h>
#define _POSIX_SOURCE
#include <string.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS 10

void DisableInputBuffering()
{
	struct termios tty_attr;
	tcgetattr(STDIN_FILENO, &tty_attr);
	tty_attr.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &tty_attr);
}

void EnableInputBuffering()
{
	struct termios tty_attr;
	tcgetattr(STDIN_FILENO, &tty_attr);
	tty_attr.c_lflag |= ICANON | ECHO;
	tcsetattr(STDIN_FILENO, TCSANOW, &tty_attr);
}

void ReadLine(char *Buffer, size_t BufferSize)
{
	size_t index = 0;
	int c;
	while (1)
	{
		c = getchar();
		if (c == EOF || c == '\n')
		{
			Buffer[index] = '\0';
			break;
		}
		else if (c == 127 || c == 8) // Backspace
		{
			if (index > 0)
			{
				printf("\b \b");
				index--;
			}
		}
		else if (c == 4) // Ctrl + D
		{
			if (index == 0)
				kill(getpid(), SIGQUIT);
			else
				putchar('\a');
		}
		else
		{
			if (index < BufferSize - 1)
			{
				putchar(c);
				Buffer[index] = (char)c;
				index++;
			}
		}
	}
}

void ExecuteCommand(char *command)
{
	char *args[MAX_ARGS];
	int i = 0;

	char *token = strtok(command, " ");
	while (token != NULL && i < MAX_ARGS - 1)
	{
		args[i++] = token;
		token = strtok(NULL, " ");
	}
	args[i] = NULL;

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
	{
		perror("fork");
		exit(EXIT_FAILURE);
	}
}

void HandleSignal(int signal)
{
	if (signal == SIGQUIT)
	{
		EnableInputBuffering();
		printf("\n");
		exit(EXIT_SUCCESS);
	}
	else if (signal == SIGINT)
	{
		putchar('\n');
	}
	else if (signal == SIGHUP)
	{
		EnableInputBuffering();
		exit(EXIT_SUCCESS);
	}
	else if (signal == SIGCHLD)
		wait(NULL);
	else if (signal == SIGSEGV)
	{
		printf("Segmentation fault\n");
		while (1)
			sleep(1000);
	}
	else
	{
		printf("Signal %s(%d) received\n",
			   strsignal(signal), signal);
	}
}

int main()
{
	char command[MAX_COMMAND_LENGTH];
	for (int i = 0; i < NSIG; ++i)
		signal(i, HandleSignal);

	char hostname[256];
	gethostname(hostname, sizeof(hostname));

	while (1)
	{
		printf("\033[1;32m");
		printf("┌──(%s@%s)-[%s]\n", getenv("USER"), hostname, getenv("PWD"));
		printf("└$ ");
		printf("\033[0m");

		DisableInputBuffering();
		ReadLine(command, sizeof(command));
		EnableInputBuffering();
		putchar('\n');

		if (strcmp(command, "exit") == 0)
			break;

		ExecuteCommand(command);
	}

	return 0;
}
