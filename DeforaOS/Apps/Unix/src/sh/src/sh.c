/* sh.c */



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
