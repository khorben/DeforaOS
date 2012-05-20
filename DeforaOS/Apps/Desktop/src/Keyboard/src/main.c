/* $Id$ */
/* Copyright (c) 2010-2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Keyboard */
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
#include <locale.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include "keyboard.h"
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


/* private */
/* functions */
/* usage */
static int _usage(void)
{
	fputs(_("Usage: keyboard [-d|-p|-w|-x][-f font][-m monitor]\n"),
			stderr);
	return 1;
}


/* public */
/* functions */
/* main */
int main(int argc, char * argv[])
{
	int o;
	Keyboard * keyboard;
	KeyboardPrefs prefs;
	char * p;

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	memset(&prefs, 0, sizeof(prefs));
	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "df:m:pwx")) != -1)
		switch(o)
		{
			case 'd':
				prefs.mode = KEYBOARD_MODE_DOCKED;
				break;
			case 'f':
				prefs.font = optarg;
				break;
			case 'm':
				prefs.monitor = strtol(optarg, &p, 10);
				if(optarg[0] == '\0' || *p != '\0')
					return _usage();
				break;
			case 'p':
				prefs.mode = KEYBOARD_MODE_POPUP;
				break;
			case 'w':
				prefs.mode = KEYBOARD_MODE_WINDOWED;
				break;
			case 'x':
				prefs.mode = KEYBOARD_MODE_EMBEDDED;
				break;
			default:
				return _usage();
		}
	if(optind != argc)
		return _usage();
	keyboard = keyboard_new(&prefs);
	gtk_main();
	keyboard_delete(keyboard);
	return 0;
}
