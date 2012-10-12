/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
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
#include <stdio.h>
#include <stdlib.h>
#include <Desktop.h>
#include "../include/Panel.h"


/* private */
/* prototypes */
static int _message(unsigned int timeout, GtkMessageType type,
		char const * message);

/* callbacks */
static gboolean _message_on_timeout(gpointer data);

static int _error(char const * message, int ret);
static int _usage(void);


/* functions */
/* message */
static int _message(unsigned int timeout, GtkMessageType type,
		char const * message)
{
	GtkWidget * widget;
	GtkWidget * label;
	uint32_t xid;

	widget = gtk_plug_new(0);
	gtk_container_set_border_width(GTK_CONTAINER(widget), 4);
	label = gtk_label_new(message);
	gtk_container_add(GTK_CONTAINER(widget), label);
	gtk_widget_show_all(widget);
	xid = gtk_plug_get_id(GTK_PLUG(widget));
	desktop_message_send(PANEL_CLIENT_MESSAGE, PANEL_MESSAGE_EMBED, xid, 0);
	if(timeout > 0)
		g_timeout_add(timeout * 1000, _message_on_timeout, NULL);
	gtk_main();
	return 0;
}


/* callbacks */
/* message_on_timeout */
static gboolean _message_on_timeout(gpointer data)
{
	gtk_main_quit();
	return FALSE;
}


/* error */
static int _error(char const * message, int ret)
{
	fprintf(stderr, "%s%s\n", "panel-message: ", message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: panel-message [-T type][-t timeout] message\n", stderr);
	return 1;
}


/* public */
/* functions */
/* main */
int main(int argc, char * argv[])
{
	GtkMessageType type = GTK_MESSAGE_INFO;
	unsigned int timeout = 3;
	int o;
	char * p;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "t:T:")) != -1)
		switch(o)
		{
			case 'T':
				/* FIXME implement */
				break;
			case 't':
				timeout = strtoul(optarg, &p, 10);
				if(optarg[0] == '\0' || *p != '\0')
					return _usage();
				break;
			default:
				return _usage();
		}
	if(argc - optind != 1)
		return _usage();
	return (_message(timeout, type, argv[optind]) == 0) ? 0 : 2;
}
