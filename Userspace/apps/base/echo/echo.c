#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

int main(int argc, char *argv[])
{
	int enableEscapes = 0;
	int disableNewline = 0;

	int opt;
	while ((opt = getopt(argc, argv, "neE")) != -1)
	{
		switch (opt)
		{
		case 'n':
			disableNewline = 1;
			break;
		case 'e':
			enableEscapes = 1;
			break;
		case 'E':
			enableEscapes = 0;
			break;
		case '?':
		default:
			exit(EXIT_FAILURE);
		}
	}

	(void)enableEscapes;

	size_t totalLength = 0;
	for (int i = optind; i < argc; ++i)
		totalLength += strlen(argv[i]) + 1;

	char *result = (char *)malloc(totalLength);
	if (!result)
	{
		perror("Memory allocation error");
		exit(EXIT_FAILURE);
	}
	result[0] = '\0';

	for (int i = optind; i < argc; ++i)
	{
		strcat(result, argv[i]);
		if (i < argc - 1)
			strcat(result, " ");
	}

	printf("%s", result);

	if (!disableNewline)
		printf("\n");

	free(result);
	return 0;
}
