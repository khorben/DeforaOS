/* sh.c */



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "cmd.h"
#include "tokenlist.h"
#include "parser.h"
#include "sh.h"


/* sh */
/* sh_file */
static char * file_get_line(FILE * fp);
int sh_file(struct prefs * p, char * file)
{
	FILE * fp = stdin;
	char * line;
	TokenList * t;

	if(file != NULL)
		if((fp = fopen(file, "r")) == NULL)
		{
			perror("fopen");
			return 127;
		}
	do
	{
		if(p->i)
			cmd_prompt();
		line = file_get_line(fp);
		t = tokenlist_new_from_string(line);
		complete_command(t, 0, NULL);
		tokenlist_delete(t);
		free(line);
	}
	while(line != NULL);
	if(p->i)
		fprintf(stderr, "\n");
	if(file != NULL)
		fclose(fp);
	return 0;
}

static char * file_get_line(FILE * fp)
{
#define FGL_BS 80
	char * str;
	char * p;
	int n = 0;

	if((str = malloc(sizeof(char) * FGL_BS + 1)) == NULL)
		return NULL;
	while(fgets(&str[n], FGL_BS + 1, fp))
	{
		p = &str[n];
		while(*p)
		{
			if(*p == '\n')
			{
				*p = '\0';
				return str;
			}
			p++;
		}
		n += FGL_BS;
		if((p = realloc(str, sizeof(char) * FGL_BS + 1)) == NULL)
			break;
		str = p;
	}
	free(str);
	return NULL;
}


/* sh_string */
int sh_string(struct prefs * p, char * string, int argc, char * argv[])
{
	TokenList * t;

#ifdef DEBUG
	fprintf(stderr, "sh_string(p, %s, %d, argv)\n",
			string, argc);
#endif
	t = tokenlist_new_from_string(string);
	complete_command(t, argc, argv);
	tokenlist_delete(t);
	return 1;
}


/* usage */
static int usage(void)
{
	fprintf(stderr, "Usage: sh [-abCefhimnuvx][-o option]\n\
		[command_file [argument...]]\n\
	sh -c[-abCefhimnuvx][-o option]command_string\n\
		[command_name [argument...]]\n\
	sh -s[-abCefhimnuvx][-o option][argument]\n\
  -c    read commands from the command_string operand\n\
  -i    specify that the shell is interactive\n\
  -s    read commands from the standard input\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	struct prefs p;

	if(prefs_parse(&p, argc, argv) != 0)
		return usage();
	if(p.c == 1)
	{
		if(optind == argc)
			return usage();
		return sh_string(&p, argv[optind], argc - optind, &argv[optind+1]);
	}
	if(p.s == 1)
		if(argc > optind + 1)
			return usage();
	if(isatty(0) && isatty(2))
		p.i = 1;
	return sh_file(&p, NULL);
}
