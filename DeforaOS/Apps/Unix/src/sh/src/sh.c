/* sh.c */



#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"


/* Prefs */
typedef struct _Prefs {
	/* FIXME optimize with enums */
	char c;
	char s;
	char i;
} Prefs;

static int _prefs_parse(Prefs * prefs, int argc, char * argv[])
{
	int o;

	memset(prefs, 0, sizeof(Prefs));
	while((o = getopt(argc, argv, "csi")) != -1)
		switch(o)
		{
			case 'c':
				prefs->s = 0;
				prefs->c = 1;
				break;
			case 's':
				prefs->c = 0;
				prefs->s = 1;
				break;
			case 'i':
				prefs->i = 1;
				break;
			default:
				return -1;
		}
	return 0;
}


/* sh */
static void _sh_prompt(void);
static int _sh_file(Prefs * prefs, char const * filename, int argc,
		char * argv[])
{
	Parser * p;
	FILE * fp = stdin;
	int res = 0;
	int i;

#ifdef DEBUG
	fprintf(stderr, "sh_file: %s", filename == NULL ? "stdin" : filename);
	if(argc >= 1)
	{
		fprintf(stderr, "%s%d%s", ", with ", argc, " arguments");
	}
	fprintf(stderr, "\n");
#endif
	if(filename != NULL)
		if((fp = fopen(filename, "r")) == NULL)
		{
			fprintf(stderr, "%s", "sh: ");
			perror(filename);
			return 127;
		}
	if((p = parser_new(fp)) == NULL)
	{
		if(filename != NULL)
			fclose(fp);
		return 1;
	}
	if(prefs->i)
		_sh_prompt();
	for(; (i = parser_parse(p)) >= 0; res = i)
		if(prefs->i)
			_sh_prompt();
	parser_delete(p);
	if(filename != NULL)
		fclose(fp);
	return -res;
}

static void _sh_prompt(void)
{
	fprintf(stderr, "$ ");
}

static int _sh_string(Prefs * prefs, char const * string,
		int argc, char * argv[])
{
	Parser * p;
	int res;

#ifdef DEBUG
	fprintf(stderr, "%s%s",
			"sh_string: ", string);
	if(argc >= 1)
	{
		fprintf(stderr, "%s%s", ", name: ", argv[0]);
		if(argc >= 2)
		{
			fprintf(stderr, "%s%d%s", ", with ",
					argc-1, " arguments");
		}
	}
	fprintf(stderr, "\n");
#endif
	if((p = parser_new_from_string(string)) == NULL)
		return 1;
	res = parser_parse(p);
	parser_delete(p);
	return res;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: sh    [-i] [command_file [argument...]]\n\
       sh -c [-i] command_string [command_name [argument...]]\n\
       sh -s [-i] [argument...]\n\
  -c    read commands from command_string\n\
  -s    read commands from standard input\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs p;

	if(_prefs_parse(&p, argc, argv) != 0)
		return _usage();
	if(p.c == 1)
	{
		if(optind == argc)
			return _usage();
		return _sh_string(&p, argv[optind], argc - optind - 1,
				&argv[optind+1]);
	}
	if(p.s == 0 && optind != argc)
		return _sh_file(&p, argv[optind], argc - optind - 1,
				&argv[optind+1]);
	p.s = 1;
	if(optind == argc)
		if(isatty(0) && isatty(2))
			p.i = 1;
	return _sh_file(&p, NULL, argc - optind, &argv[optind]);
}
