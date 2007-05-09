/* $Id$ */
/* Copyright (c) 2007 The DeforaOS Project */



#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/* pr */
/* types */
typedef struct _Prefs
{
	int flags;
	int lines;
	int width;
	int offset;
} Prefs;
#define PREFS_d 1
#define PREFS_t 2

/* functions */
static int _pr_error(char const * message, int ret);
static int _pr_do(Prefs * prefs, FILE * fp, char const * filename);

static int _pr(Prefs * prefs, int filec, char * filev[])
{
	int ret = 0;
	int i;
	FILE * fp;

	if(filec == 0)
		return _pr_do(prefs, stdin, "Standard input");
	for(i = 0; i < filec; i++)
	{
		if(strcmp(filev[i], "-") == 0)
		{
			ret |= _pr_do(prefs, stdin, "Standard input");
			continue;
		}
		if((fp = fopen(filev[i], "r")) == NULL)
		{
			ret |= _pr_error(filev[i], 1);
			continue;
		}
		ret |= _pr_do(prefs, fp, filev[i]);
		if(fclose(fp) != 0)
			ret |= _pr_error(filev[i], 1);
	}
	return ret;
}

static int _pr_error(char const * message, int ret)
{
	fputs("pr: ", stderr);
	perror(message);
	return ret;
}

/* _pr_do */
static void _do_offset(int offset);
static void _do_header(Prefs * prefs, time_t const mtime, char const * filename,
		size_t page);
static void _do_footer(Prefs * prefs);

static int _pr_do(Prefs * prefs, FILE * fp, char const * filename)
{
	struct stat st;
	char * buf;
	size_t len;
	int nb = 0;
	size_t page = 1;

	if(fstat(fileno(fp), &st) != 0)
	{
		st.st_mtime = 0;
		_pr_error(filename, 0);
	}
	if((buf = malloc(prefs->width + 1)) == NULL)
		return _pr_error("malloc", 1);
	while(fgets(buf, prefs->width, fp) != NULL)
	{
		if(nb == 0 && !(prefs->flags & PREFS_t) && prefs->lines > 10)
		{
			_do_header(prefs, st.st_mtime, filename, page++);
			nb = 10;
		}
		_do_offset(prefs->offset); /* FIXME not if truncated line */
		if((len = strlen(buf)) > 0 && buf[len - 1] == '\n'
				&& prefs->flags & PREFS_d)
			buf[len++] = '\n'; /* XXX with offset? */
		if(fwrite(buf, sizeof(char), len, stdout) != len)
		{
			free(buf);
			return _pr_error("stdout", 1);
		}
		if(nb++ == prefs->lines && prefs->lines > 10
				&& !(prefs->flags & PREFS_t))
		{
			_do_footer(prefs);
			nb = 0;
		}
	}
	if(prefs->lines > 10 && !(prefs->flags & PREFS_t))
		for(; nb != prefs->lines; nb++)
			fputc('\n', stdout);
	free(buf);
	return 0;
}

static void _do_offset(int offset)
{
	while(offset-- > 0)
		fputc(' ', stdout);
}

static void _do_header(Prefs * prefs, time_t const mtime, char const * filename,
		size_t page)
{
	struct tm tm;
	char buf[19];
	int nb;

	for(nb = 0; nb < 5; nb++)
	{
		_do_offset(prefs->offset);
		if(nb == 2)
		{
			localtime_r(&mtime, &tm);
			strftime(buf, sizeof(buf) - 1, "%b %e %Y  %H:%M", &tm);
			buf[sizeof(buf) - 1] = '\0';
			printf("%s  %s%s%u", buf, filename, "  Page ", page);
		}
		fputc('\n', stdout);
	}
}

static void _do_footer(Prefs * prefs)
{
	int i;

	for(i = 0; i < 5; i++)
	{
		_do_offset(prefs->offset);
		fputc('\n', stdout);
	}
}


/* usage */
static int _usage(void)
{
	fputs("Usage: pr [+page][-column][-adFmrt][-e [char][ gap]]"
			"[-h header][-i[char][gap]]\n"
			"[-l lines][-n [char][width]][-o offset][-s[char]]"
			"[-w width] file...\n",
			stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs;
	int o;
	char * p;

	memset(&prefs, 0, sizeof(prefs));
	prefs.lines = 66;
	prefs.width = 72;
	while((o = getopt(argc, argv, "dl:o:tw:")) != -1)
		switch(o)
		{
			case 'd':
				prefs.flags |= PREFS_d;
				break;
			case 'l':
				prefs.lines = strtol(optarg, &p, 10);
				if(optarg[0] == '\0' || *p != '\0'
						|| prefs.lines <= 0)
					return _usage();
				break;
			case 'o':
				prefs.offset = strtol(optarg, &p, 10);
				if(optarg[0] == '\0' || *p != '\0'
						|| prefs.lines < 0)
					return _usage();
				break;
			case 't':
				prefs.flags |= PREFS_t;
				break;
			case 'w':
				prefs.width = strtol(optarg, &p, 10);
				if(optarg[0] == '\0' || *p != '\0'
						|| prefs.width <= 0)
					return _usage();
				break;
			default:
				return _usage();
		}
	return _pr(&prefs, argc - optind, &argv[optind]) == 0 ? 0 : 2;
}
