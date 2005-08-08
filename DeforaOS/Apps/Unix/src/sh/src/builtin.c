/* builtin.c */



#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "sh.h"
#include "builtin.h"

extern char ** environ;


/* builtin_bg */
int builtin_bg(int argc, char * argv[])
{
	/* FIXME */
	return 0;
}


/* builtin_cd */
static int _cd_usage(void);
static int _cd_home(void);
static int _cd_previous(void);
static int _cd_chdir(int * prefs, char * path);
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
	if(argc-optind > 1)
		return _cd_usage();
	if(argc-optind == 0)
		return _cd_home();
	if(strcmp("-", argv[optind]) == 0)
		return _cd_previous();
	return _cd_chdir(&prefs, argv[optind]);
}

static int _cd_usage(void)
{
	fprintf(stderr, "%s", "Usage: cd [-L | -P] directory\n\
       cd -\n\
  -L	resolve symbolic links after parent directories\n\
  -P	resolve symbolic links before parent directories\n");
	return 1;
}

static int _cd_home(void)
{
	char * home;
	int prefs = 0;

	if((home = getenv("HOME")) == NULL)
	{
		fprintf(stderr, "%s", "sh: cd: $HOME is not set\n");
		return 125;
	}
	return _cd_chdir(&prefs, home);
}

static int _cd_previous(void)
{
	char * oldpwd;
	int prefs = 0;
	int ret;

	if((oldpwd = getenv("OLDPWD")) == NULL)
	{
		fprintf(stderr, "%s", "sh: cd: $OLDPWD is not set\n");
		return 125;
	}
	if((ret = _cd_chdir(&prefs, oldpwd)) == 0)
		fprintf(stderr, "%s%s", oldpwd, "\n");
	return ret;
}

static int _cd_chdir(int * prefs, char * path)
{
	char * oldpwd;

	/* FIXME use prefs */
	if(chdir(path) != 0)
		return sh_error(path, 125); /* FIXME */
	if((oldpwd = getenv("PWD")) != NULL)
		if(setenv("OLDPWD", oldpwd, 1) != 0)
			sh_error("setenv OLDPWD", 0);
	/* FIXME else get current directory using getcwd */
	if(setenv("PWD", path, 1) != 0)
		sh_error("setenv PWD", 0);
	return 0;
}


/* builtin_exec */
int builtin_exec(int argc, char * argv[])
{
	if(argc == 0)
		return 0;
	execvp(argv[0], argv);
	exit(127);
}


/* builtin_exit */
static int _exit_usage(void);
int builtin_exit(int argc, char * argv[])
{
	int status = 0;
	char * p;

	if(argc > 2)
		return _exit_usage();
	if(argc == 2)
	{
		status = strtol(argv[1], &p, 10);
		if(*(argv[1]) == '\0' || *p != '\0' || status < 0
				|| status > 255)
			return _exit_usage();
	}
	exit(status);
	return 0;
}

static int _exit_usage(void)
{
	fprintf(stderr, "%s", "Usage: exit [n]\n");
	return 1;
}


/* builtin_export */
static int _export_usage(void);
static void _export_list(void);
static void _export_do(char * arg);
int builtin_export(int argc, char * argv[])
{
	int prefs = 0;
	int o;
	int i;

	optind = 1;
	while((o = getopt(argc, argv, "p")) != -1)
		switch(o)
		{
			case 'p':
				prefs = 1;
				break;
			default:
				return _export_usage();
		}
	if(prefs == 1 && optind == argc)
		_export_list();
	else if(prefs == 1 || optind == argc)
		return _export_usage();
	else
		for(i = optind; i < argc; i++)
			_export_do(argv[i]);
	return 0;
}

static int _export_usage(void)
{
	fprintf(stderr, "%s", "Usage: export name[=value]...\n\
       export -p\n\
  -p	list all variables\n");
}

static void _export_list(void)
{
	char ** e;
	char * p;

	/* FIXME should read exported variables list instead */
	for(e = environ; *e != NULL; e++)
	{
		printf("%s", "export ");
		for(p = *e; *p != '\0' && *p != '='; p++)
			fputc(*p, stdout);
		if(*p == '\0')
		{
			fputc('\n', stdout);
			continue;
		}
		printf("%s", "=\"");
		for(p++; *p != '\0'; p++)
		{
			if(*p == '"')
				fputc('\\', stdout);
			fputc(*p, stdout);
		}
		printf("%s", "\"\n");
	}
	return 0;
}

static void _export_do(char * arg)
{
	/* FIXME */
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
static int _read_usage(void);
static int _read_do(int argc, char * argv[]);
int builtin_read(int argc, char * argv[])
{
	int o;

	optind = 1;
	while((o = getopt(argc, argv, "r")) != -1)
		switch(o)
		{
			case 'r':
				/* FIXME */
				break;
			default:
				return _read_usage();
		}
	return _read_do(argc-optind, &argv[optind]);
}

static int _read_usage(void)
{
	fprintf(stderr, "%s", "Usage: read [-r] var...\n\
  -r	do not escape backslashes\n");
	return 1;
}

static int _read_do(int argc, char * argv[])
{
	int c;
	char ** arg = argv;
	char * value = NULL;
	int value_cnt = 0;
	char * p;
	int ret = 0;

	if(arg == NULL)
		return 0;
	for(c = fgetc(stdin);; c = fgetc(stdin))
	{
		/* FIXME backslash escaping is optional */
		if(c == '\\')
		{
			if((c = fgetc(stdin)) == '\n')
				continue;
		}
		else if(c == EOF || c == '\n' || (isblank(c) && *(arg+1) != NULL))
		{
			value[value_cnt] = '\0';
			if(setenv(*arg, value, 1) != 0)
				ret+=sh_error("setenv", 1);
			value_cnt = 0;
			if(*(arg+1) != NULL)
				arg++;
			if(c == EOF || c == '\n')
				break;
			continue;
		}
		if((p = realloc(value, value_cnt+2)) == NULL)
		{
			free(value);
			return sh_error("malloc", 2);
		}
		value = p;
		value[value_cnt++] = c;
	}
	free(value);
	return ret == 0 ? 0 : 2;
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


/* umask */
static int _umask_usage(void);
static int _umask_get(void);
static int _umask_set(char * mask);
int builtin_umask(int argc, char * argv[])
{
	int o;

	optind = 1;
	while((o = getopt(argc, argv, "S")) != -1)
		switch(o)
		{
			case 'S':
				/* FIXME */
				break;
			default:
				return _umask_usage();
		}
	if(argc > optind+1)
		return _umask_usage();
	if(optind == argc)
		return _umask_get();
	return _umask_set(argv[optind]);
}

static int _umask_usage(void)
{
	fprintf(stderr, "%s", "Usage: umask [-s][mask]\n\
  -S	provide symbolic output\n");
	return 1;
}

static int _umask_get(void)
{
	mode_t mask;

	mask = umask(0);
	printf("%04o%s", mask, "\n");
	umask(mask);
	return 0;
}

static int _umask_set(char * mask)
{
	mode_t mode;
	char * p;

	mode = strtol(mask, &p, 8);
	if(mask[0] == '\0' || *p != '\0')
		return _umask_usage();
	umask(mode);
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
