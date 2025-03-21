/*
	This file is part of Fennix Core Utilities.

	Fennix Core Utilities is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Core Utilities is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Core Utilities. If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

typedef struct AliasData
{
	char *name;
	char *value;
	struct AliasData *next;
} AliasData;

AliasData *Aliases = NULL;

void FreeAliases()
{
	AliasData *current = Aliases;
	while (current != NULL)
	{
		AliasData *next = current->next;
		free(current->name);
		free(current->value);
		free(current);
		current = next;
	}
	Aliases = NULL;
}

AliasData *FindAlias(const char *name)
{
	AliasData *current = Aliases;
	while (current != NULL)
	{
		if (strcmp(current->name, name) == 0)
			return current;
		current = current->next;
	}
	return NULL;
}

void AddAlias(const char *name, const char *value)
{
	AliasData *existing = FindAlias(name);
	if (existing)
	{
		free(existing->value);
		existing->value = strdup(value);
		return;
	}

	AliasData *new_alias = malloc(sizeof(AliasData));
	new_alias->name = strdup(name);
	new_alias->value = strdup(value);
	new_alias->next = Aliases;
	Aliases = new_alias;
}

int ReadAliases(const char *filename)
{
	FILE *file = fopen(filename, "r");
	if (!file)
		return -1;

	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	while ((read = getline(&line, &len, file)) != -1)
	{
		if (line[read - 1] == '\n')
			line[read - 1] = '\0';

		char *eq = strchr(line, '=');
		if (!eq)
			continue;

		*eq = '\0';
		AddAlias(line, eq + 1);
	}

	free(line);
	fclose(file);
	return 0;
}

int WriteAliases(const char *filename)
{
	FILE *file = fopen(filename, "w");
	if (!file)
	{
		perror("Error writing aliases");
		return -1;
	}

	AliasData *current = Aliases;
	while (current)
	{
		fprintf(file, "%s=%s\n", current->name, current->value);
		current = current->next;
	}

	fclose(file);
	return 0;
}

char *QuoteValue(const char *value)
{
	size_t quotes = 0;
	for (const char *p = value; *p; p++)
	{
		if (*p == '\'')
			quotes++;
	}

	char *quoted = malloc(strlen(value) + quotes * 4 + 3);
	if (!quoted)
		return NULL;

	char *dest = quoted;
	*dest++ = '\'';
	for (const char *p = value; *p; p++)
	{
		if (*p == '\'')
		{
			strcpy(dest, "'\\''");
			dest += 4;
		}
		else
			*dest++ = *p;
	}
	*dest++ = '\'';
	*dest = '\0';
	return quoted;
}

int main(int argc, char *argv[])
{
	const char *home = getenv("HOME");
	if (!home)
	{
		fprintf(stderr, "alias: HOME not set\n");
		return 2;
	}

	char path[1024];
	snprintf(path, sizeof(path), "%s/.aliases", home);

	if (ReadAliases(path) == -1 && errno != ENOENT)
	{
		perror("Error reading aliases");
		return 2;
	}

	int status = 0;
	if (argc == 1)
	{
		AliasData *current = Aliases;
		while (current)
		{
			char *q = QuoteValue(current->value);
			printf("alias %s=%s\n", current->name, q);
			free(q);
			current = current->next;
		}
	}
	else
	{
		for (int i = 1; i < argc; i++)
		{
			char *arg = argv[i];
			char *eq = strchr(arg, '=');

			if (eq)
			{
				*eq = '\0';
				char *name = arg;
				char *value = eq + 1;
				AddAlias(name, value);

				char *q = QuoteValue(value);
				printf("alias %s=%s\n", name, q);
				free(q);
			}
			else
			{
				AliasData *a = FindAlias(arg);
				if (a)
				{
					char *q = QuoteValue(a->value);
					printf("alias %s=%s\n", a->name, q);
					free(q);
				}
				else
				{
					fprintf(stderr, "alias: %s: not found\n", arg);
					status = 1;
				}
			}
		}
	}

	if (WriteAliases(path) == -1)
		status = 1;
	FreeAliases();
	return status;
}
