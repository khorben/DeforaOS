/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix utils */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>


/* more */
/* private */
/* types */
typedef struct _Prefs
{
	int lines;
} Prefs;


/* prototypes */
static int _more_error(char const * message, int ret);


/* functions */
/* more */
static int _more_do(Prefs * prefs, char const * filename);

static int _more(Prefs * prefs, int filec, char * filev[])
{
	int ret = 0;
	int i;

	if(filec == 0)
		return _more_do(prefs, NULL);
	for(i = 0; i < filec; i++)
		ret |= _more_do(prefs, filev[i]);
	return ret;
}

static int _more_do(Prefs * prefs, char const * filename)
{
	FILE * fp = stdin;
	char buf[1024];
	size_t len;
	int n = 1;
	int c;

	if(filename != NULL && strcmp(filename, "-") != 0
			&& (fp = fopen(filename, "r")) == NULL)
		return _more_error(filename, 1);
	while(fgets(buf, sizeof(buf), fp) != NULL)
	{
		len = strlen(buf);
		if(fwrite(buf, 1, len, stdout) != len)
			break; /* XXX report error */
		if(prefs->lines <= 0)
			continue;
		if(len > 0 && buf[len - 1] != '\n')
			continue;
		if(++n % prefs->lines != 0)
			continue;
		n = 1;
		fputs("--more--", stderr);
		while((c = getchar()) != EOF && c != '\n');
	}
	if(filename != NULL && strcmp(filename, "-") != 0)
		fclose(fp);
	return 0;
}


/* more_error */
static int _more_error(char const * message, int ret)
{
	fputs("more: ", stderr);
	perror(message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: more [file...]\n", stderr);
	return 1;
}


/* public */
/* functions */
/* main */
int main(int argc, char * argv[])
{
	Prefs prefs;
	int o;
	char * p;

	memset(&prefs, 0, sizeof(prefs));
	while((o = getopt(argc, argv, "n:")) != -1)
		switch(o)
		{
			case 'n':
				prefs.lines = strtol(optarg, &p, 10);
				if(optarg[0] == '\0' || *p != '\0'
						|| prefs.lines <= 0)
					return _usage();
				break;
			default:
				return _usage();
		}
	if(!isatty(0))
		prefs.lines = 0;
	else if(prefs.lines <= 0 && (p = getenv("LINES")) != NULL)
		prefs.lines = strtol(p, NULL, 10);
	return _more(&prefs, argc - optind, &argv[optind]);
}
