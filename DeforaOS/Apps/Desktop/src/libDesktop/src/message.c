/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop libDesktop */
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



#include <string.h>
#include "Desktop.h"


/* Message */
/* desktop_message_send */
int desktop_message_send(char const * destination, uint32_t value1,
		uint32_t value2, uint32_t value3)
{
	GdkEvent event;
	GdkEventClient * client = &event.client;

	memset(&event, 0, sizeof(event));
	client->type = GDK_CLIENT_EVENT;
	client->window = NULL;
	client->send_event = TRUE;
	client->message_type = gdk_atom_intern(destination, FALSE);
	client->data_format = 8;
	client->data.b[0] = value1;
	client->data.b[1] = value2;
	client->data.b[2] = value3;
	gdk_event_send_clientmessage_toall(&event);
	return 0;
}
