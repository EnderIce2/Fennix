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
#include <stdbool.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_WHITE "\x1b[37m"
#define ANSI_COLOR_BLACK "\x1b[30m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define ANSI_BG_BLACK "\x1b[40m"
#define ANSI_BG_RED "\x1b[41m"
#define ANSI_BG_GREEN "\x1b[42m"
#define ANSI_BG_YELLOW "\x1b[43m"
#define ANSI_BG_BLUE "\x1b[44m"
#define ANSI_BG_MAGENTA "\x1b[45m"
#define ANSI_BG_CYAN "\x1b[46m"
#define ANSI_BG_GRAY "\x1b[47m"
#define ANSI_BG_DARK_GRAY "\x1b[100m"

#define ANSI_BOLD "\x1b[1m"
#define ANSI_UNDERLINE "\x1b[4m"
#define ANSI_STRIKETHROUGH "\x1b[9m"
#define ANSI_HIGHLIGHT "\x1b[43m"

void print_horizontal_rule()
{
	int width;
	struct winsize ws;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
	width = ws.ws_col;
	for (int i = 0; i < width - 2; i++)
		printf("-");
}

void print_admonition(const char *type, const char *title, const char *content)
{
	if (strcmp(type, "warning") == 0)
	{
		printf(ANSI_BG_YELLOW);
		printf(ANSI_COLOR_BLACK);
		printf("! Warning: %s %s\n", title, content);
	}
	else if (strcmp(type, "note") == 0)
	{
		printf(ANSI_BG_BLUE);
		printf(ANSI_COLOR_WHITE);
		printf("i Note: %s %s\n", title, content);
	}
	else if (strcmp(type, "tip") == 0)
	{
		printf(ANSI_BG_GREEN);
		printf(ANSI_COLOR_WHITE);
		printf("? Tip: %s %s\n", title, content);
	}
	else if (strcmp(type, "important") == 0)
	{
		printf(ANSI_BG_RED);
		printf(ANSI_COLOR_BLACK);
		printf("! Important: %s %s\n", title, content);
	}
	else if (strcmp(type, "caution") == 0)
	{
		printf(ANSI_BG_MAGENTA);
		printf(ANSI_COLOR_WHITE);
		printf("! Caution: %s %s\n", title, content);
	}
	else if (strcmp(type, "danger") == 0)
	{
		printf(ANSI_BG_RED);
		printf(ANSI_COLOR_BLACK);
		printf("! Danger: %s %s\n", title, content);
	}
	printf(ANSI_COLOR_RESET);
}

void print_formatted_text(const char *text)
{
	bool bold = false;
	bool underline = false;
	bool inline_code = false;
	bool strikethrough = false;
	bool highlight = false;

	for (int i = 0; text[i] != '\0'; ++i)
	{
		if (strncmp(&text[i], "!!!", 3) == 0)
		{
			char admonition_type[20] = "";
			char admonition_title[256] = "";
			int type_start = i + 4;
			int type_end = -1;
			int title_start = -1;
			int title_end = -1;
			int content_start = -1;

			for (int j = type_start; text[j] != '\0' && text[j] != ' ' && text[j] != '"'; ++j)
			{
				type_end = j;
			}
			if (type_end != -1)
			{
				strncpy(admonition_type, &text[type_start], type_end - type_start + 1);
				admonition_type[type_end - type_start + 1] = '\0';
				i = type_end + 1;
			}

			if (text[i] == ' ' && text[i + 1] == '"')
			{
				title_start = i + 2;
				for (int j = title_start; text[j] != '\0' && text[j] != '"'; ++j)
				{
					title_end = j;
				}
				if (title_end != -1)
				{
					strncpy(admonition_title, &text[title_start], title_end - title_start + 1);
					admonition_title[title_end - title_start + 1] = '\0';
					i = title_end + 1;
				}
			}

			content_start = i + 1;
			print_admonition(admonition_type, admonition_title, text + content_start);
			return;
		}

		if (text[i] == '`')
		{
			if (!inline_code)
			{
				printf(ANSI_BG_MAGENTA);
				printf(ANSI_UNDERLINE);
				printf(ANSI_BOLD);
				inline_code = true;
			}
			else
			{
				printf(ANSI_COLOR_RESET);
				inline_code = false;
			}
		}
		else if (text[i] == '*' && text[i + 1] == '*')
		{
			if (!bold)
			{
				printf(ANSI_BOLD);
				bold = true;
				i++;
			}
			else
			{
				printf(ANSI_COLOR_RESET);
				bold = false;
				i++;
			}
		}
		else if (text[i] == '_' && text[i + 1] == '_')
		{
			if (!underline)
			{
				printf(ANSI_UNDERLINE);
				underline = true;
				i++;
			}
			else
			{
				printf(ANSI_COLOR_RESET);
				underline = false;
				i++;
			}
		}
		else if (text[i] == '~' && text[i + 1] == '~')
		{
			if (!strikethrough)
			{
				printf(ANSI_STRIKETHROUGH);
				strikethrough = true;
				i++;
			}
			else
			{
				printf(ANSI_COLOR_RESET);
				strikethrough = false;
				i++;
			}
		}
		else if (text[i] == '=' && text[i + 1] == '=')
		{
			if (!highlight)
			{
				printf(ANSI_HIGHLIGHT);
				highlight = true;
				i++;
			}
			else
			{
				printf(ANSI_COLOR_RESET);
				highlight = false;
				i++;
			}
		}
		else if (text[i] == '[')
		{
			int start_link_text = i + 1;
			int end_link_text = -1;
			int start_link_url = -1;
			int end_link_url = -1;

			for (int j = start_link_text; text[j] != '\0'; ++j)
			{
				if (text[j] == ']')
				{
					end_link_text = j;
					if (text[j + 1] == '(')
					{
						start_link_url = j + 2;

						for (int k = start_link_url; text[k] != '\0'; ++k)
						{
							if (text[k] == ')')
							{
								end_link_url = k;
								break;
							}
						}
					}
					break;
				}
			}

			if (end_link_text != -1 && start_link_url != -1 && end_link_url != -1)
			{

				char link_text[256];
				strncpy(link_text, &text[start_link_text], end_link_text - start_link_text);
				link_text[end_link_text - start_link_text] = '\0';

				char link_url[256];
				strncpy(link_url, &text[start_link_url], end_link_url - start_link_url);
				link_url[end_link_url - start_link_url] = '\0';

				printf(ANSI_UNDERLINE);
				printf(link_text);
				printf(ANSI_COLOR_RESET);
				printf("(%s)", link_url);
				i = end_link_url;
				continue;
			}
		}
		else if (text[i] == '<')
		{
			while (text[i] != '\0' && text[i] != '>')
			{
				i++;
			}
			if (text[i] == '>')
			{
				continue;
			}
		}
		else if (strncmp(&text[i], "---\n", 4) == 0 || strncmp(&text[i], "***\n", 4) == 0 || strncmp(&text[i], "_________________\n", 18) == 0)
		{
			print_horizontal_rule();
			i += 2;
			continue;
		}
		else if (strncmp(&text[i], "- [x] ", 6) == 0)
		{
			printf("[X] ");
			i += 5;
			continue;
		}
		else if (strncmp(&text[i], "- [ ] ", 6) == 0)
		{
			printf("[ ] ");
			i += 5;
			continue;
		}
		else
		{
			printf("%c", text[i]);
		}
	}
	printf(ANSI_COLOR_RESET);
}

void process_markdown_file(const char *filename)
{
	FILE *file = fopen(filename, "r");
	if (file == NULL)
	{
		perror("fopen");
		return;
	}

	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	while ((read = getline(&line, &len, file)) != -1)
	{
		if (strncmp(line, "# ", 2) == 0)
		{
			printf(ANSI_BOLD);
			printf(ANSI_UNDERLINE);
			printf("%s", line);
			printf(ANSI_COLOR_RESET);
		}
		else if (strncmp(line, "## ", 3) == 0)
		{
			printf(ANSI_BOLD);
			printf("%s", line);
			printf(ANSI_COLOR_RESET);
		}
		else if (strncmp(line, "### ", 4) == 0)
		{
			printf(ANSI_UNDERLINE);
			printf("%s", line);
			printf(ANSI_COLOR_RESET);
		}
		else if (strncmp(line, "#### ", 5) == 0)
		{
			printf(ANSI_BOLD);
			printf(ANSI_UNDERLINE);
			printf(ANSI_COLOR_RED);
			printf("%s", line);
			printf(ANSI_COLOR_RESET);
		}
		else if (strncmp(line, "##### ", 6) == 0)
		{
			printf(ANSI_BOLD);
			printf(ANSI_COLOR_MAGENTA);
			printf("%s", line);
			printf(ANSI_COLOR_RESET);
		}
		else if (strncmp(line, "###### ", 7) == 0)
		{
			printf(ANSI_UNDERLINE);
			printf(ANSI_COLOR_BLUE);
			printf("%s", line);
			printf(ANSI_COLOR_RESET);
		}
		else if (strncmp(line, "```", 3) == 0)
		{
			printf(ANSI_BG_DARK_GRAY);
			printf(ANSI_COLOR_WHITE);
			printf("%s", line);
			while ((read = getline(&line, &len, file)) != -1 && strncmp(line, "```", 3) != 0)
			{
				printf("%s", line);
			}
			printf("%s", line);
			printf(ANSI_COLOR_RESET);
		}
		else if (strncmp(line, "> ", 2) == 0 && strncmp(line, ">> ", 3) != 0)
		{
			printf(ANSI_COLOR_WHITE);
			printf(ANSI_BG_GRAY " " ANSI_BG_DARK_GRAY "%s", line + (strncmp(line, ">>", 2) == 0 ? 2 : 1));
			printf(ANSI_COLOR_RESET);
		}
		else if (strncmp(line, ">\n", 2) == 0)
		{
			printf(ANSI_COLOR_WHITE);
			printf(ANSI_BG_GRAY " \n");
			printf(ANSI_COLOR_RESET);
		}
		else
		{
			print_formatted_text(line);
		}
	}

	fclose(file);
	if (line)
		free(line);
}

void print_usage()
{
	printf("Usage: mdview <markdown_file>\n");
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "mdview: invalid arguments\n");
		print_usage();
		exit(EXIT_FAILURE);
	}

	process_markdown_file(argv[1]);
	return 0;
}
