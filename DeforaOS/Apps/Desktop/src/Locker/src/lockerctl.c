/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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
#include <string.h>
#include <libintl.h>
#include "Locker.h"
#define _(string) gettext(string)


/* usage */
static int _usage(void)
{
	fputs(_("Usage: lockerctl -l\n"
"       lockerctl -p\n"
"       lockerctl -s\n"
"       lockerctl -u\n"
"  -l	Lock the screen\n"
"  -p	Show the preferences window\n"
"  -s	Enable the screen saver\n"
"  -u	Unlock the screen\n"), stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	int action = -1;
	GdkEvent event;
	GdkEventClient * client = &event.client;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "lpsu")) != -1)
		switch(o)
		{
			case 'l':
				if(action != -1)
					return _usage();
				action = LOCKER_ACTION_LOCK;
				break;
			case 'p':
				if(action != -1)
					return _usage();
				action = LOCKER_ACTION_SHOW_PREFERENCES;
				break;
			case 's':
				if(action != -1)
					return _usage();
				action = LOCKER_ACTION_ACTIVATE;
				break;
			case 'u':
				if(action != -1)
					return _usage();
				action = LOCKER_ACTION_UNLOCK;
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
	client->message_type = gdk_atom_intern(LOCKER_CLIENT_MESSAGE, FALSE);
	client->data_format = 8;
	client->data.b[0] = LOCKER_MESSAGE_ACTION;
	if(action != -1)
	{
		client->data.b[1] = action;
		client->data.b[2] = TRUE;
		gdk_event_send_clientmessage_toall(&event);
	}
	return 0;
}
