/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix sh */
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
#include <signal.h>
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

	setenv("PATH", "/usr/bin:/bin", 0);
	export = sh_export();
	if(*prefs & PREFS_c)
		ret = parser(prefs, *argv, NULL, argc - 1, &argv[1]);
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
			ret = parser(prefs, NULL, fp, argc - 1, &argv[1]);
			fclose(fp);
		}
	}
	free(export);
	return ret;
}


/* sh_error */
int sh_error(char const * message, int ret)
{
	fputs("sh: ", stderr);
	perror(message);
	return ret;
}


char ** sh_export(void)
{
	size_t cnt;
	char ** export;
	char ** e;
	size_t i;

	for(cnt = 0, e = environ; *e != NULL; cnt++, e++);
	if((export = malloc((cnt + 1) * sizeof(char*))) == NULL)
	{
		sh_error("malloc", 0);
		return NULL;
	}
	for(i = 0; i < cnt; i++)
		export[i] = environ[i];
	export[i] = NULL;
	return export;
}


/* sh_handler */
static void _handler_sigint(void);

static void _sh_handler(int signum)
{
	switch(signum)
	{
		case SIGINT:
			_handler_sigint();
			break;
	}
}

static void _handler_sigint(void)
{
	job_kill_status(SIGINT, JS_WAIT);
}


/* usage */
static int _usage(void)
{
	fputs("Usage: sh [i][command_file [argument...]]\n\
       sh -c[i]command_string[command_name [argument]]\n\
       sh -s[i][argument]\n", stderr);
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
	if(signal(SIGINT, _sh_handler) == SIG_ERR)
		sh_error("signal", 0); /* ignore error */
	return _sh(&prefs, argc - optind, &argv[optind]);
}
