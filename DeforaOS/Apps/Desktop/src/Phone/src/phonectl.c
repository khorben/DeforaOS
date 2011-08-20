/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Phone */
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
#include "phone.h"
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
	fputs(_("Usage: phonectl -C\n"
"       phonectl -D\n"
"       phonectl -L\n"
"       phonectl -M\n"
"       phonectl -S\n"
"       phonectl -r\n"
"       phonectl -s\n"
"  -C	Open the contacts window\n"
"  -D	Dial a new call\n"
"  -L	Open the phone log window\n"
"  -M	Open the messages window\n"
"  -S	Display or change settings\n"
"  -W	Write a new message\n"
"  -r	Resume telephony operation\n"
"  -s	Suspend telephony operation\n"), stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	int type = PHONE_MESSAGE_SHOW;
	int action = -1;
	GdkEvent event;
	GdkEventClient * client = &event.client;

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "CDLMSrs")) != -1)
		switch(o)
		{
			case 'C':
				if(action != -1)
					return _usage();
				action = PHONE_MESSAGE_SHOW_CONTACTS;
				break;
			case 'D':
				if(action != -1)
					return _usage();
				action = PHONE_MESSAGE_SHOW_DIALER;
				break;
			case 'L':
				if(action != -1)
					return _usage();
				action = PHONE_MESSAGE_SHOW_LOGS;
				break;
			case 'M':
				if(action != -1)
					return _usage();
				action = PHONE_MESSAGE_SHOW_MESSAGES;
				break;
			case 'S':
				if(action != -1)
					return _usage();
				action = PHONE_MESSAGE_SHOW_SETTINGS;
				break;
			case 'r':
				if(action != -1)
					return _usage();
				type = PHONE_MESSAGE_POWER_MANAGEMENT;
				action = PHONE_MESSAGE_POWER_MANAGEMENT_RESUME;
				break;
			case 's':
				if(action != -1)
					return _usage();
				type = PHONE_MESSAGE_POWER_MANAGEMENT;
				action = PHONE_MESSAGE_POWER_MANAGEMENT_SUSPEND;
				break;
			default:
				return _usage();
		}
	if(action < 0)
		return _usage();
	memset(&event, 0, sizeof(event));
	client->type = GDK_CLIENT_EVENT;
	client->window = NULL;
	client->send_event = TRUE;
	client->message_type = gdk_atom_intern(PHONE_CLIENT_MESSAGE, FALSE);
	client->data_format = 8;
	client->data.b[0] = type;
	client->data.b[1] = action;
	client->data.b[2] = TRUE;
	gdk_event_send_clientmessage_toall(&event);
	return 0;
}
