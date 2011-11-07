/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Panel */
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



#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include "panel.h"
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


/* usage */
static int _usage(void)
{
	fputs(_("Usage: panel [-m monitor][-lsx]\n"
"  -l	Use icons the size of a large toolbar\n"
"  -m	Monitor to use (default: 0)\n"
"  -s	Use icons the size of a small toolbar\n"
"  -x	Use icons the size of menus\n"), stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	Panel * panel;
	PanelPrefs prefs;
	char * p;

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	gtk_init(&argc, &argv);
	prefs.iconsize = PANEL_ICON_SIZE_UNSET;
	prefs.monitor = -1;
	while((o = getopt(argc, argv, "lm:sx")) != -1)
		switch(o)
		{
			case 'l':
				prefs.iconsize = PANEL_ICON_SIZE_LARGE;
				break;
			case 'm':
				prefs.monitor = strtol(optarg, &p, 10);
				if(optarg[0] == '\0' || *p != '\0')
					return _usage();
				break;
			case 's':
				prefs.iconsize = PANEL_ICON_SIZE_SMALL;
				break;
			case 'x':
				prefs.iconsize = PANEL_ICON_SIZE_SMALLER;
				break;
			default:
				return _usage();
		}
	if(optind != argc)
		return _usage();
	if((panel = panel_new(&prefs)) == NULL)
		return 2;
	gtk_main();
	panel_delete(panel);
	return 0;
}
