/* sh.c */



#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "sh.h"


/* Prefs */
static int _prefs_parse(Prefs * prefs, int argc, char * argv[])
{
	int o;

	memset(prefs, 0, sizeof(Prefs));
	while((o = getopt(argc, argv, "csi")) != -1)
		switch(o)
		{
			case 'c':
				*prefs -= *prefs & PREFS_s;
				*prefs |= PREFS_c;
				break;
			case 's':
				*prefs -= *prefs & PREFS_c;
				*prefs |= PREFS_s;
				break;
			case 'i':
				*prefs |= PREFS_i;
				break;
			default:
				return -1;
		}
	return 0;
}


/* sh */
static int _sh(Prefs * prefs, int argc, char * argv[])
{
	int ret;
	FILE * fp;

	if(*prefs & PREFS_c)
		return parser(prefs, *argv, NULL, argc-1, &argv[1]);
	if(!(*prefs & PREFS_s) && argc != 0)
		return parser(prefs, NULL, stdin, argc-1, &argv[1]);
	/* *prefs |= PREFS_s; FIXME necessary? */
	if(argc == 0)
	{
		if(isatty(0) && isatty(2))
			*prefs |= PREFS_i;
		return parser(prefs, NULL, stdin, 0, NULL);
	}
	if((fp = fopen(argv[0], "r")) == NULL)
		return sh_error(argv[0], 127);
	ret = parser(prefs, NULL, fp, argc-1, &argv[1]);
	fclose(fp);
	return ret;
}

int sh_error(char * message, int ret)
{
	fprintf(stderr, "%s", "sh: ");
	perror(message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: sh [i][command_file [argument...]]\n\
       sh -c[i]command_string[command_name [argument]]\n\
       sh -s[i][argument]\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs;

	if(_prefs_parse(&prefs, argc, argv) != 0)
		return _usage();
	if(prefs & PREFS_c && optind == argc)
		return _usage();
	return _sh(&prefs, argc-optind, &argv[optind]);
}
