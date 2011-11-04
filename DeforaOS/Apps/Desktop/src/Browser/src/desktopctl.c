/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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
#include <string.h>
#include <locale.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include "desktop.h"
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
	fputs(_("Usage: desktopctl [-H|-S|-V|-a|-c|-f|-h|-n]\n"
"  -H	Place icons horizontally\n"
"  -S	Display or change settings\n"
"  -V	Place icons vertically\n"
"  -a	Display the applications registered\n"
"  -c	Sort the applications registered by category\n"
"  -f	Display contents of the desktop folder\n"
"  -h	Display the homescreen\n"
"  -n	Do not display icons on the desktop\n"), stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	int action = -1;
	int what = 0;
	GdkEvent event;
	GdkEventClient * client = &event.client;

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "HSVacfhn")) != -1)
		switch(o)
		{
			case 'H':
				if(action != -1)
					return _usage();
				action = DESKTOP_MESSAGE_SET_ALIGNMENT;
				what = DESKTOP_ALIGNMENT_HORIZONTAL;
				break;
			case 'S':
				if(action != -1)
					return _usage();
				action = DESKTOP_MESSAGE_SHOW;
				what = DESKTOP_SHOW_SETTINGS;
				break;
			case 'V':
				if(action != -1)
					return _usage();
				action = DESKTOP_MESSAGE_SET_ALIGNMENT;
				what = DESKTOP_ALIGNMENT_VERTICAL;
				break;
			case 'a':
				if(action != -1)
					return _usage();
				action = DESKTOP_MESSAGE_SET_LAYOUT;
				what = DESKTOP_LAYOUT_APPLICATIONS;
				break;
			case 'c':
				if(action != -1)
					return _usage();
				action = DESKTOP_MESSAGE_SET_LAYOUT;
				what = DESKTOP_LAYOUT_CATEGORIES;
				break;
			case 'f':
				if(action != -1)
					return _usage();
				action = DESKTOP_MESSAGE_SET_LAYOUT;
				what = DESKTOP_LAYOUT_FILES;
				break;
			case 'h':
				if(action != -1)
					return _usage();
				action = DESKTOP_MESSAGE_SET_LAYOUT;
				what = DESKTOP_LAYOUT_HOMESCREEN;
				break;
			case 'n':
				if(action != -1)
					return _usage();
				action = DESKTOP_MESSAGE_SET_LAYOUT;
				what = DESKTOP_LAYOUT_NONE;
				break;
			default:
				return _usage();
		}
	if(action == -1 || optind != argc)
		return _usage();
	memset(&event, 0, sizeof(event));
	client->type = GDK_CLIENT_EVENT;
	client->window = NULL;
	client->send_event = TRUE;
	client->message_type = gdk_atom_intern(DESKTOP_CLIENT_MESSAGE, FALSE);
	client->data_format = 8;
	client->data.b[0] = action;
	client->data.b[1] = what;
	client->data.b[2] = TRUE;
	gdk_event_send_clientmessage_toall(&event);
	return 0;
}
