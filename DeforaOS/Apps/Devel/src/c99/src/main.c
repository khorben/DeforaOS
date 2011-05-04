/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel c99 */
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



#include <sys/utsname.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "c99.h"
#include "../config.h"


/* c99 */
static int _c99(C99Prefs * prefs, int filec, char * filev[])
{
	int ret = 0;
	size_t len;
	int i;
	C99 * c99;

	if(filec == 1 && (len = strlen(filev[0])) > 2
			&& filev[0][len - 2] == '.' && filev[0][len - 1] == 'c')
	{
		if((c99 = c99_new(prefs, filev[0])) == NULL)
			return error_print(PACKAGE);
		ret = c99_parse(c99);
		c99_delete(c99);
		return ret;
	}
	/* link time */
	for(i = 0; i < filec; i++)
	{
		/* FIXME implement */
	}
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: c99 [-c][-D name[=value]]...[-E][-g][-I directory]"
"[-L directory][-m name[=value]]...[-o outfile][-Ooptlevel][-s][-U name]..."
" operand ...\n", stderr);
	return 1;
}


/* public */
/* main */
static int _main_default_defines(C99Prefs * prefs);
static int _main_default_paths(C99Prefs * prefs);
static int _main_add_define(C99Prefs * prefs, char * define);
static int _main_add_path(C99Prefs * prefs, char const * path);
static int _main_add_undefine(C99Prefs * prefs, char const * undefine);
static int _main_add_option(C99Prefs * prefs, char const * option);

int main(int argc, char * argv[])
{
	int ret;
	C99Prefs prefs;
	int o;

	memset(&prefs, 0, sizeof(prefs));
	if(_main_default_defines(&prefs) != 0
			|| _main_default_paths(&prefs) != 0)
		return 2;
	while((o = getopt(argc, argv, "cD:EgI:L:m:M:o:O123sU:W")) != -1)
		switch(o)
		{
			case 'c':
				prefs.flags |= C99PREFS_c;
				break;
			case 'D':
				if(_main_add_define(&prefs, optarg) != 0)
					return 2;
				break;
			case 'E':
				prefs.flags |= C99PREFS_E;
				break;
			case 'I':
				if(_main_add_path(&prefs, optarg) != 0)
					return 2;
				break;
			case 'g':
				prefs.flags |= C99PREFS_g;
				break;
			case 'm':
				if(_main_add_option(&prefs, optarg) != 0)
					return 2;
				break;
			case 'M':
				prefs.target = optarg;
				break;
			case 'o':
				prefs.outfile = optarg;
				break;
			case 'O':
				prefs.optlevel++;
				break;
			case 's':
				prefs.flags |= C99PREFS_s;
				break;
			case 'U':
				if(_main_add_undefine(&prefs, optarg) != 0)
					return 2;
				break;
			case 'W':
				prefs.warn = 1;
				break;
			case '1':
			case '2':
			case '3':
				prefs.optlevel = o - '0';
				break;
			default:
				return _usage();
		}
	if(optind == argc)
		ret = _usage();
	else if(prefs.flags & C99PREFS_c
			&& prefs.outfile != NULL
			&& optind + 1 != argc)
		ret = _usage();
	else
	{
		ret = _c99(&prefs, argc - optind, &argv[optind]);
		ret = (ret == 0) ? 0 : 2;
	}
	free(prefs.paths);
	free(prefs.defines);
	free(prefs.undefines);
	return ret;
}

static int _main_default_defines(C99Prefs * prefs)
	/* FIXME define these in the as plug-in instead */
{
	struct utsname uts;
	static char sysname[sizeof(uts.sysname) + 7];
	static char machine[sizeof(uts.machine) + 7];

	if(uname(&uts) != 0)
		return error_set_print(PACKAGE, 1, "%s", strerror(errno));
	snprintf(sysname, sizeof(sysname), "__%s__=1", uts.sysname);
	snprintf(machine, sizeof(machine), "__%s__=1", uts.machine);
	if(_main_add_define(prefs, sysname) != 0
			|| _main_add_define(prefs, machine) != 0)
		return 1;
	return 0;
}

static int _main_default_paths(C99Prefs * prefs)
{
	char * paths[] = { "/usr/include" };
	size_t i;
	char * cpath;

	for(i = 0; i < (sizeof(paths) / sizeof(*paths)); i++)
		if(_main_add_path(prefs, paths[i]) != 0)
			return 2;
	if((cpath = getenv("CPATH")) == NULL)
		return 0;
	while((cpath = strtok(cpath, ":")) != NULL)
		if(_main_add_path(prefs, cpath) != 0)
			return 2;
		else
			cpath = NULL;
	return 0;
}

static int _main_add_define(C99Prefs * prefs, char * define)
{
	char ** p;
	char * value;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, define);
#endif
	if(strlen(define) == 0)
		return _usage();
	value = strtok(define, "=");
	if((p = realloc(prefs->defines, sizeof(*p) * (prefs->defines_cnt + 1)))
			== NULL)
		return error_set_print(PACKAGE, 1, "%s", strerror(errno));
	prefs->defines = p;
	prefs->defines[prefs->defines_cnt++] = define;
	return 0;
}

static int _main_add_path(C99Prefs * prefs, char const * path)
{
	const char ** p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, path);
#endif
	if((p = realloc(prefs->paths, sizeof(*p) * (prefs->paths_cnt + 1)))
			== NULL)
		return error_set_print(PACKAGE, 1, "%s", strerror(errno));
	prefs->paths = p;
	prefs->paths[prefs->paths_cnt++] = path;
	return 0;
}

static int _main_add_undefine(C99Prefs * prefs, char const * undefine)
{
	const char ** p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, undefine);
#endif
	if(strlen(undefine) == 0)
		return 1;
	if((p = realloc(prefs->undefines, sizeof(*p)
					* (prefs->undefines_cnt + 1))) == NULL)
		return error_set_print(PACKAGE, 1, "%s", strerror(errno));
	prefs->undefines = p;
	prefs->undefines[prefs->undefines_cnt++] = undefine;
	return 0;
}

static int _main_add_option(C99Prefs * prefs, char const * option)
{
	C99Option * p;
	char * q;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, option);
#endif
	if(strlen(option) == 0)
		return 1;
	if((p = realloc(prefs->options, sizeof(*p)
					* (prefs->options_cnt + 1))) == NULL)
		return error_set_print(PACKAGE, 1, "%s", strerror(errno));
	prefs->options = p;
	p = &prefs->options[prefs->options_cnt];
	if((p->name = string_new(option)) == NULL)
		return 1;
	p->value = NULL;
	if((q = strstr(p->name, "=")) != NULL)
	{
		*q = '\0';
		p->value = q + 1;
	}
	prefs->options_cnt++;
	return 0;
}
