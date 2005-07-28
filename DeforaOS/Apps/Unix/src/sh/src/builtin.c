/* builtin.c */



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sh.h"
#include "builtin.h"


/* builtin_bg */
int builtin_bg(int argc, char * argv[])
{
	/* FIXME */
	return 0;
}


/* builtin_cd */
static int _cd_usage(void);
static int _cd_previous(void);
static int _cd_chdir(int prefs, char * path);
int builtin_cd(int argc, char * argv[])
{
	int o;
	int prefs = 0;

	optind = 1;
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
	if(argc-optind != 1)
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
	/* FIXME error codes */
{
	char * oldpwd;
	char buf[256];

	if((oldpwd = getenv("OLDPWD")) == NULL)
		return sh_error("getenv", 125);
	if(getcwd(buf, sizeof(buf)) == NULL)
		return sh_error("getcwd", 125);
	if(chdir(oldpwd) != 0)
		return sh_error("chdir", 125);
	if(setenv("OLDPWD", buf, 1) != 0)
		return sh_error("setenv", 125);
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


/* builtin_fg */
int builtin_fg(int argc, char * argv[])
{
	/* FIXME */
	return 0;
}


/* builtin_jobs */
int builtin_jobs(int argc, char * argv[])
{
	/* FIXME */
	return 0;
}


/* builtin_read */
int builtin_read(int argc, char * argv[])
{
	/* FIXME fgets/realloc */
	return 0;
}


/* set */
static int _set_usage(void);
static int _set_do(int argc, char * argv[]);
static int _set_list(void);
static int _set_unset(void);
int builtin_set(int argc, char * argv[])
{
	int o;

	optind = 1;
	while((o = getopt(argc, argv, "o")) != -1)
		switch(o)
		{
			case 'o':
				break;
			default:
				return _set_usage();
		}
	if(argc == optind)
	{
		if(optind > 1 && strcmp(argv[optind-1], "--") == 0)
			return _set_unset();
		return _set_list();
	}
	return _set_do(argc, argv);
}

static int _set_usage(void)
{
	/* FIXME */
	fprintf(stderr, "%s", "Usage: set -- [argument...]\n\
       set -o\n\
       set +o\n");
	return 1;
}

static int _set_do(int argc, char * argv[])
{
	char * p;
	int ret = 0;

	for(; optind < argc; optind++)
	{
		for(p = argv[optind]; *p != '\0' && *p != '='; p++);
		if(*p != '=')
			continue;
		*p = '\0';
		if(setenv(argv[optind], p+1, 1) != 0)
			ret+=sh_error("setenv", 1);
		*p = '=';
	}
	return ret;
}

static int _set_list(void)
{
	char ** e;

	for(e = environ; *e != NULL; e++)
		printf("%s\n", *e);
	return 0;
}

static int _set_unset(void)
{
	char * e;
	char buf[256];
	unsigned int pos;

#ifdef DEBUG
	fprintf(stderr, "%s", "_set_unset()\n");
#endif
	for(e = *environ; e != NULL; e = *environ)
	{
		for(pos = 0; pos < sizeof(buf)-1 && e[pos] != '\0'
				&& e[pos] != '='; pos++);
		if(e[pos] != '=')
			continue;
		strncpy(buf, e, pos);
		buf[pos] = '\0';
		unsetenv(buf);
	}
	return 0;
}


/* unset */
static int _unset_usage(void);
int builtin_unset(int argc, char * argv[])
	/* FIXME */
{
	int o;

	optind = 1;
	while((o = getopt(argc, argv, "fv")) != -1)
		switch(o)
		{
			case 'f':
			case 'v':
				/* FIXME */
				break;
			default:
				return _unset_usage();
		}
	if(optind == argc)
		return _unset_usage();
	for(; optind < argc; optind++)
		unsetenv(argv[optind]);
	return 0;
}

static int _unset_usage(void)
{
	fprintf(stderr, "%s", "Usage: unset [-fv] name [...]\n\
  -f	\n\
  -v	\n");
	return 1;
}
