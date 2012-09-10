/* $Id$ */
/* Copyright (c) 2006-2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Browser */
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
#include <stdio.h>
#include <locale.h>
#include <libintl.h>
#include <Desktop.h>
#include "../config.h"
#define _(string) gettext(string)

/* constants */
#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif
#ifndef DATADIR
# define DATADIR	PREFIX "/share"
#endif
#ifndef LOCALEDIR
# define LOCALEDIR	DATADIR "/locale"
#endif


/* open */
/* private */
/* prototypes */
static int _open(char const * type, char const * action, int filec,
		char * filev[]);
static int _open_error(char const * message, int ret);

static int _usage(void);


/* functions */
/* open */
static int _open(char const * type, char const * action, int filec,
		char * filev[])
{
	int i;
	Mime * mime;
	int ret = 0;

	if((mime = mime_new(NULL)) == NULL)
		return 1;
	for(i = 0; i < filec; i++)
	{
		if(type == NULL)
		{
			if(mime_action(mime, action, filev[i]) == 0)
				continue;
		}
		else if(mime_action_type(mime, action, filev[i], type) == 0)
			continue;
		fprintf(stderr, "%s%s%s%s%s", "open: ", filev[i],
				_(": Could not perform action \""), action,
				"\"\n");
		ret = 1;
	}
	mime_delete(mime);
	return ret;
}


/* open_error */
static int _open_error(char const * message, int ret)
{
	fputs("open: ", stderr);
	perror(message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs(_("Usage: open [-m mime type][-a action] file...\n"
"  -m	MIME type to force (default: auto-detected)\n"
"  -a	action to call (default: \"open\")\n"), stderr);
	return 1;
}


/* public */
/* functions */
/* main */
int main(int argc, char * argv[])
{
	int o;
	char const * action = "open";
	char const * type = NULL;

	if(setlocale(LC_ALL, "") == NULL)
		_open_error("setlocale", 1);
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "a:m:")) != -1)
		switch(o)
		{
			case 'a':
				action = optarg;
				break;
			case 'm':
				type = optarg;
				break;
			default:
				return _usage();
		}
	if(optind == argc)
		return _usage();
	return (_open(type, action, argc - optind, &argv[optind]) == 0) ? 0 : 2;
}
