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


/* panelctl */
/* private */
/* prototypes */
static int _panelctl(PanelMessageShow show);
static int _usage(void);


/* functions */
static int _panelctl(PanelMessageShow show)
{
	GdkEvent event;
	GdkEventClient * client = &event.client;

	memset(&event, 0, sizeof(event));
	client->type = GDK_CLIENT_EVENT;
	client->window = NULL;
	client->send_event = TRUE;
	client->message_type = gdk_atom_intern(PANEL_CLIENT_MESSAGE, FALSE);
	client->data_format = 8;
	client->data.b[0] = PANEL_MESSAGE_SHOW;
	client->data.b[1] = show;
	client->data.b[2] = TRUE;
	gdk_event_send_clientmessage_toall(&event);
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: panelctl -S\n"
"  -S	Display or change settings\n", stderr);
	return 1;
}


/* public */
/* functions */
/* main */
int main(int argc, char * argv[])
{
	int o;
	int show = -1;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "S")) != -1)
		switch(o)
		{
			case 'S':
				show = PANEL_MESSAGE_SHOW_SETTINGS;
				break;
			default:
				return _usage();
		}
	if(argc != optind || show < 0)
		return _usage();
	return (_panelctl(show) == 0) ? 0 : 2;
}
