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
#include <gtk/gtk.h>
#include "common.h"


/* usage */
static int _usage(void)
{
	fputs("Usage: panel-settings\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	GdkEvent event;
	GdkEventClient * client = &event.client;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	memset(&event, 0, sizeof(event));
	client->type = GDK_CLIENT_EVENT;
	client->window = NULL;
	client->send_event = TRUE;
	client->message_type = gdk_atom_intern(PANEL_CLIENT_MESSAGE, FALSE);
	client->data_format = 8;
	client->data.b[0] = PANEL_MESSAGE_SHOW;
	client->data.b[1] = PANEL_MESSAGE_SHOW_SETTINGS;
	client->data.b[2] = TRUE;
	gdk_event_send_clientmessage_toall(&event);
	return 0;
}
