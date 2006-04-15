/* sh.c */



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "job.h"
#include "sh.h"

#define min(a, b) (a) < (b) ? (a) : (b)

extern char ** environ;


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
char ** export;
static int _sh(Prefs * prefs, int argc, char * argv[])
{
	int ret;
	FILE * fp;

	export = sh_export();
	if(*prefs & PREFS_c)
		ret = parser(prefs, *argv, NULL, argc-1, &argv[1]);
	else if(!(*prefs & PREFS_s) && argc != 0)
		ret = parser(prefs, NULL, stdin, argc-1, &argv[1]);
	/* *prefs |= PREFS_s; FIXME necessary? */
	else if(argc == 0)
	{
		if(isatty(0) && isatty(2))
			*prefs |= PREFS_i;
		ret = parser(prefs, NULL, stdin, 0, NULL);
	}
	else
	{
		if((fp = fopen(argv[0], "r")) == NULL)
			ret = sh_error(argv[0], 127);
		else
		{
			ret = parser(prefs, NULL, fp, argc-1, &argv[1]);
			fclose(fp);
		}
	}
	free(export);
	return ret;
}


int sh_error(char * message, int ret)
{
	fprintf(stderr, "%s", "sh: ");
	perror(message);
	return ret;
}


char ** sh_export(void)
{
	int cnt;
	char ** export;
	char ** e;
	int i;

	for(cnt = 0, e = environ; *e != NULL; cnt++, e++);
	if((export = malloc((cnt+1) * sizeof(char*))) == NULL)
	{
		sh_error("malloc", 0);
		return NULL;
	}
	for(i = 0; i < cnt; i++)
		export[i] = environ[i];
	export[i] = NULL;
	return export;
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


#ifdef DEBUG
# undef malloc
# undef realloc
# undef free
void * dbg_malloc(size_t size, char * file, int line)
{
	void * p;

	p = malloc(size);
	fprintf(stderr, "%p = malloc(%u) %s:%d\n", p, size, file, line);
	return p;
}

void * dbg_realloc(void * ptr, size_t size, char * file, int line)
{
	void * p;

	p = realloc(ptr, size);
	fprintf(stderr, "%p = realloc(%p, %u) %s:%d\n", p, ptr, size, file,
			line);
	return p;
}

void dbg_free(void * ptr, char * file, int line)
{
	fprintf(stderr, "free(%p) %s:%d\n", ptr, file, line);
	free(ptr);
}
#endif
