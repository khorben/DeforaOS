/* main.c */



#include <unistd.h>
extern int optind;
#include <stdio.h>
#include "prefs.h"


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
