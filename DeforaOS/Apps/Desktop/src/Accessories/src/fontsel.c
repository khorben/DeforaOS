/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Accessories */
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



#include <gtk/gtk.h>


/* Font */
static void _fontsel_on_exitx(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static int _fontsel(void)
{
	GtkWidget * window;
	GtkWidget * fontsel;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(window), 4);
	gtk_window_set_title(GTK_WINDOW(window), "Font browser");
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(
				_fontsel_on_exitx), NULL);
	fontsel = gtk_font_selection_new();
	gtk_container_add(GTK_CONTAINER(window), fontsel);
	gtk_widget_show_all(window);
	gtk_main();
	return 0;
}

static void _fontsel_on_exitx(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	gtk_widget_hide(widget);
	gtk_main_quit();
}


/* main */
int main(int argc, char * argv[])
{
	gtk_init(&argc, &argv);
	return _fontsel() == 0 ? 0 : 2;
}
