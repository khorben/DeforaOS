/* sh.c */



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <libutils/libutils.h>
#include "cmd.h"
#include "tokenlist.h"
#include "parser.h"
#include "prefs.h"


/* sh */
/* sh_file */
static int sh_file(struct prefs * p, char * filename)
{
	File * file;
	char * line;
	TokenList * tokenlist;
	int res;

	if(filename != NULL)
		file = file_new(filename, "r");
	else
		file = file_new_from_pointer(stdin);
	if(file == NULL)
	{
		perror("Couldn't open file");
		return 127;
	}
	do
	{
		if(p->i)
			cmd_prompt();
		line = file_get_line(file);
#ifdef DEBUG
		fprintf(stderr, "line is: \"%s\"\n", line);
#endif
		tokenlist = tokenlist_new(line);
		res = complete_command(tokenlist);
		tokenlist_delete(tokenlist);
		free(line);
	}
	while(res == 0 && line != NULL);
	file_delete(file);
	return res;
}


/* sh_string */
int sh_string(struct prefs * p, char * string, int argc, char * argv[])
{
	TokenList * t;

#ifdef DEBUG
	fprintf(stderr, "sh_string(p, %s, %d, argv)\n",
			string, argc);
#endif
	t = tokenlist_new(string);
	complete_command(t);
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
