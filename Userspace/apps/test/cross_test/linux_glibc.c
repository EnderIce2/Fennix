#define _POSIX_SOURCE
#define _DEFAULT_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pty.h>
#include <termios.h>
#include <sys/ioctl.h>

#ifndef __GLIBC__
#define __GLIBC__ 0
#endif

#ifndef __GLIBC_MINOR__
#define __GLIBC_MINOR__ 0
#endif

int ptmx_test()
{
	int masterFD;
	int slaveFD;
	char slaveName[100];

	masterFD = open("/dev/ptmx", O_RDWR | O_NOCTTY);
	if (masterFD < 0)
	{
		perror("Failed to open /dev/ptmx");
		return 1;
	}

	if (grantpt(masterFD) < 0)
	{
		perror("Failed to grantpt");
		return 1;
	}

	if (unlockpt(masterFD) < 0)
	{
		perror("Failed to unlockpt");
		return 1;
	}

	if (ptsname_r(masterFD, slaveName, sizeof(slaveName)) != 0)
	{
		perror("Failed to get slave name");
		return 1;
	}

	printf("Slave terminal: %s\n", slaveName);

	slaveFD = open(slaveName, O_RDWR | O_NOCTTY);
	if (slaveFD < 0)
	{
		perror("Failed to open slave terminal");
		return 1;
	}

	struct termios t;
	tcgetattr(slaveFD, &t);
	cfmakeraw(&t);
	tcsetattr(slaveFD, TCSANOW, &t);

	char *message = "Hello from master!\n";
	write(masterFD, message, strlen(message));

	char buffer[100];
	int len = read(slaveFD, buffer, sizeof(buffer) - 1);
	if (len > 0)
	{
		buffer[len] = '\0';
		printf("Received from slave: %s\n", buffer);
	}

	close(masterFD);
	close(slaveFD);
	return 0;
}

int main(int argc, char *argv[], char *envp[])
{
	printf("glibc %d.%d: Hello, World!\n", __GLIBC__, __GLIBC_MINOR__);
	fflush(stdout);
	ptmx_test();
	return 0;
}
