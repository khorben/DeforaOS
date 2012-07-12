/* $Id$ */
/* Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Locker */
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
#include <gtk/gtk.h>
#include "locker.h"
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


/* functions */
/* usage */
static int _usage(void)
{
	fputs(_("Usage: locker [-d demo][-p plug-in][-s]\n"
"  -d	Demo sub-system to load\n"
"  -p	Authentication plug-in to load\n"
"  -s	Suspend automatically when locked\n"), stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	int suspend = 0;
	char const * demo = NULL;
	char const * auth = NULL;
	Locker * locker;

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "d:p:s")) != -1)
		switch(o)
		{
			case 'd':
				demo = optarg;
				break;
			case 's':
				suspend = 1;
				break;
			case 'p':
				auth = optarg;
				break;
			default:
				return _usage();
		}
	if(optind != argc)
		return _usage();
	if((locker = locker_new(suspend, demo, auth)) == NULL)
		return 2;
	gtk_main();
	locker_delete(locker);
	return 0;
}
