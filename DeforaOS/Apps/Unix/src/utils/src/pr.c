/* $Id$ */
/* Copyright (c) 2007 The DeforaOS Project */



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
} Prefs;
#define PREFS_t 1

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

static int _pr_do(Prefs * prefs, FILE * fp, char const * filename)
{
	char * buf;
	size_t len;
	int nb = 0;
	size_t page = 1;

	if((buf = malloc(prefs->width + 1)) == NULL)
		return _pr_error("malloc", 1);
	while(fgets(buf, prefs->width, fp) != NULL)
	{
		if(nb == 0 && !(prefs->flags & PREFS_t) && prefs->lines > 10)
		{
			printf("\n\n%s%s%u\n\n\n", filename, " Page ", page++);
			nb = 5;
		}
		if((len = strlen(buf)) > 0 && buf[len - 1] == '\n')
			buf[len - 1] = '\0';
		fputs(buf, stdout);
		fputc('\n', stdout);
		if(nb++ == prefs->lines && prefs->lines > 10
				&& !(prefs->flags & PREFS_t))
		{
			fputs("\n\n\n\n\n\n", stdout);
			nb = 0;
		}
	}
	if(prefs->lines > 10 && !(prefs->flags & PREFS_t))
		for(; nb != prefs->lines; nb++)
			fputc('\n', stdout);
	free(buf);
	return 0;
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
	while((o = getopt(argc, argv, "l:tw:")) != -1)
		switch(o)
		{
			case 'l':
				prefs.lines = strtol(optarg, &p, 10);
				if(optarg[0] == '\0' || *p != '\0'
						|| prefs.lines <= 0)
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
