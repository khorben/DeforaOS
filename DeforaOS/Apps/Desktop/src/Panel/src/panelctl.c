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



#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include <Desktop.h>
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


/* panelctl */
/* private */
/* prototypes */
static int _panelctl(PanelMessageShow what, gboolean show);
static int _usage(void);


/* functions */
static int _panelctl(PanelMessageShow what, gboolean show)
{
	desktop_message_send(PANEL_CLIENT_MESSAGE, PANEL_MESSAGE_SHOW, what,
			show);
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs(_("Usage: panelctl [-B|-S|-T|-b|-t]\n"
"  -B	Show the bottom panel\n"
"  -S	Display or change settings\n"
"  -T	Show the top panel\n"
"  -b	Hide the bottom panel\n"
"  -t	Hide the top panel\n"), stderr);
	return 1;
}


/* public */
/* functions */
/* main */
int main(int argc, char * argv[])
{
	int o;
	int what = -1;
	gboolean show = TRUE;

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "BSTbt")) != -1)
		switch(o)
		{
			case 'B':
				what = PANEL_MESSAGE_SHOW_PANEL_BOTTOM;
				show = TRUE;
				break;
			case 'S':
				what = PANEL_MESSAGE_SHOW_SETTINGS;
				show = TRUE;
				break;
			case 'T':
				what = PANEL_MESSAGE_SHOW_PANEL_TOP;
				show = TRUE;
				break;
			case 'b':
				what = PANEL_MESSAGE_SHOW_PANEL_BOTTOM;
				show = FALSE;
				break;
			case 't':
				what = PANEL_MESSAGE_SHOW_PANEL_TOP;
				show = FALSE;
				break;
			default:
				return _usage();
		}
	if(argc != optind || what < 0)
		return _usage();
	return (_panelctl(what, show) == 0) ? 0 : 2;
}
