/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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
#include <gtk/gtk.h>


/* prototypes */
/* callbacks */
static gboolean _on_delete_event(gpointer data);


/* functions */
/* on_delete_event */
static gboolean _on_delete_event(gpointer data)
{
	GtkWidget * widget = data;

	gtk_widget_hide(widget);
	gtk_main_quit();
	return TRUE;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: plug id\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	unsigned long u;
	GtkWidget * window;
	GtkWidget * socket;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind + 1 != argc)
		return _usage();
	u = strtoul(argv[optind], NULL, 10);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
	g_signal_connect_swapped(G_OBJECT(window), "delete-event", G_CALLBACK(
				_on_delete_event), window);
	socket = gtk_socket_new();
	gtk_container_add(GTK_CONTAINER(window), socket);
	gtk_socket_add_id(GTK_SOCKET(socket), u);
	gtk_widget_show_all(window);
	gtk_main();
	return 0;
}
