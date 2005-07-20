/* sh.c */



#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"


/* Prefs */
typedef int Prefs;
#define PREFS_c 01
#define PREFS_i 02
#define PREFS_s 03

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
static int _sh_error(char const * message, int ret);
static int _sh_file(Prefs * prefs, char const * filename, int argc,
		char * argv[]);
static int _sh_string(Prefs * prefs, char const * string, int argc,
		char * argv[]);
static int _sh(Prefs * prefs, int argc, char * argv[])
{
	if(*prefs & PREFS_c)
		return _sh_string(prefs, *argv, argc, argv);
	if(!(*prefs & PREFS_s) && argc != 0)
		return _sh_file(prefs, *argv, argc, argv);
	*prefs |= PREFS_s;
	if(optind == argc)
		if(isatty(0) && isatty(2))
			*prefs |= PREFS_i;
	return _sh_file(prefs, NULL, argc, argv);
}

static void _sh_prompt(void);
static int _sh_file(Prefs * prefs, char const * filename, int argc,
		char * argv[])
{
	Parser * p;
	FILE * fp = stdin;
	int res = 0;
	int i;

	if(filename != NULL)
		if((fp = fopen(filename, "r")) == NULL)
			return _sh_error(filename, 127);
	if((p = parser_new(fp)) == NULL)
	{
		if(filename != NULL)
			fclose(fp);
		return 1;
	}
	if(*prefs & PREFS_i)
		_sh_prompt();
	for(; (i = parser_parse(p)) >= 0; res = i)
		if(*prefs & PREFS_i)
			_sh_prompt();
	parser_delete(p);
	if(filename != NULL)
		fclose(fp);
	return -res;
}

static int _sh_error(char const * message, int ret)
{
	fprintf(stderr, "%s", "sh: ");
	perror(message);
	return ret;
}

static void _sh_prompt(void)
{
	fprintf(stderr, "%s", "$ ");
}

static int _sh_string(Prefs * prefs, char const * string, int argc,
		char * argv[])
{
	Parser * p;
	int res;

#ifdef DEBUG
	fprintf(stderr, "%s%s", "sh_string: ", string);
	if(argc >= 1)
	{
		fprintf(stderr, "%s%s", ", name: ", argv[0]);
		if(argc >= 2)
			fprintf(stderr, "%s%d%s", ", with ", argc-1,
					" arguments");
	}
	fputc('\n', stderr);
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
  -c	read commands from command_string\n\
  -s	read commands from standard input\n");
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
