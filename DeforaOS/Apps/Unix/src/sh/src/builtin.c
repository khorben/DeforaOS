/* builtin.c */



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sh.h"
#include "builtin.h"


/* builtin_cd */
static int _cd_usage(void);
static int _cd_previous(void);
static int _cd_chdir(int prefs, char * path);
int builtin_cd(int argc, char * argv[])
{
	int o;
	int prefs = 0;

	while((o = getopt(argc, argv, "LP")) != -1)
		switch(o)
		{
			case 'L':
				prefs = 0;
				break;
			case 'P':
				prefs = 1;
				break;
			default:
				return _cd_usage();
		}
	if(argc - optind != 1)
		return _cd_usage();
	if(strcmp("-", argv[optind]) == 0)
		return _cd_previous();
	return _cd_chdir(prefs, argv[optind]);
}

static int _cd_usage(void)
{
	fprintf(stderr, "%s", "Usage: cd [-L | -P] directory\n\
	cd -\n\
  -L	resolve symbolic links after parent directories\n\
  -P	resolve symbolic links before parent directories\n");
	return 1;
}

static int _cd_previous(void)
{
	/* FIXME set $OLDPWD */
	/* FIXME everything else */
	return 0;
}

static int _cd_chdir(int prefs, char * path)
{
	/* FIXME use prefs */
	/* FIXME set $PWD correctly */
	if(chdir(path) == 0)
		return 0;
	return sh_error(path, 125); /* FIXME */
}


/* builtin_exec */
int builtin_exec(int argc, char * argv[])
{
	if(argc == 0)
		return 0;
	execvp(argv[0], argv);
	exit(127);
}


/* builtin_read */
int builtin_read(int argc, char * argv[])
{
	/* FIXME fgets/realloc */
	return 0;
}


/* set */
static int _set_usage(void);
static int _set_list(void);
int builtin_set(int argc, char * argv[])
{
	if(argc == 0)
		return _set_list();
	/* FIXME */
	return _set_usage();
}

static int _set_usage(void)
{
	/* FIXME */
	fprintf(stderr, "%s", "Usage: set\n");
	return 1;
}

static int _set_list(void)
{
	char ** e;

	for(e = environ; *e != NULL; e++)
		printf("%s\n", *e);
	return 0;
}
