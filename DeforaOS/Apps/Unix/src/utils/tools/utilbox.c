/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
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
#include <libgen.h>


/* utilbox */
/* private */
/* types */
typedef struct _Call
{
	char const * name;
	int (*call)(int argc, char * argv[]);
} Call;


/* variables */
#include "utils.c"


/* prototypes */
static int _error(char const * message, int ret);
static int _list(Call * calls);
static int _usage(void);


/* functions */
/* error */
static int _error(char const * message, int ret)
{
	fputs("utilbox: ", stderr);
	perror(message);
	return ret;
}


/* list */
static int _list(Call * calls)
{
	size_t i;

	for(i = 0; calls[i].name != NULL; i++)
		printf("%s\n", calls[i].name);
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: utilbox program [arguments...]\n"
"       utilbox -l\n"
"  -l	List available programs\n", stderr);
	return 1;
}


/* public */
/* functions */
/* main */
int main(int argc, char * argv[])
{
	char * p;
	char const * q;
	size_t i;
	int o;

	if((p = strdup(argv[0])) == NULL)
		return _error(NULL, 2);
	q = basename(p);
	for(i = 0; _calls[i].name != NULL; i++)
		if(strcmp(_calls[i].name, q) == 0)
		{
			free(p);
			return _calls[i].call(argc, argv);
		}
	free(p);
	while((o = getopt(argc, argv, "l")) != -1)
		switch(o)
		{
			case 'l':
				return _list(_calls);
			default:
				return _usage();
		}
	if(optind == argc)
		return _usage();
	for(i = 0; _calls[i].name != NULL; i++)
		if(strcmp(_calls[i].name, argv[optind]) == 0)
			return _calls[i].call(argc - optind, &argv[optind]);
	return 0;
}
