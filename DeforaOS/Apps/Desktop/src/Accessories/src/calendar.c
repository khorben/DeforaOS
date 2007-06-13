/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Accessories */
/* Accessories is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * Accessories is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Accessories; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA */



#include <gtk/gtk.h>


/* Calendar */
static void _calendar_on_exitx(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static int _calendar(void)
{
	GtkWidget * window;
	GtkWidget * calendar;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "Calendar");
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(
				_calendar_on_exitx), NULL);
	calendar = gtk_calendar_new();
	gtk_container_add(GTK_CONTAINER(window), calendar);
	gtk_widget_show_all(window);
	gtk_main();
	return 0;
}

static void _calendar_on_exitx(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	gtk_widget_hide(widget);
	gtk_main_quit();
}


/* main */
int main(int argc, char * argv[])
{
	gtk_init(&argc, &argv);
	return _calendar() == 0 ? 0 : 2;
}
