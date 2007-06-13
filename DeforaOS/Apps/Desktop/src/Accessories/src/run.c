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



#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <gtk/gtk.h>


/* run */
static int _run_error(char const * message, int ret);
/* callbacks */
static gboolean _on_run_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static void _on_run_cancel(GtkWidget * widget, gpointer data);
static void _on_run_choose_activate(GtkWidget * widget, gint arg1,
		gpointer data);
static void _on_run_execute(GtkWidget * widget, gpointer data);
static void _on_run_path_activate(GtkWidget * widget, gpointer data);
static void _run(void)
{
	GtkWidget * window;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;
	GtkSizeGroup * group;
	GtkWidget * entry;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "Run program...");
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(
				_on_run_closex), NULL);
	group = gtk_size_group_new(GTK_SIZE_GROUP_BOTH);
	vbox = gtk_vbox_new(FALSE, 0);
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new("Run: ");
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 4);
	entry = gtk_entry_new();
	g_signal_connect(G_OBJECT(entry), "activate", G_CALLBACK(
				_on_run_path_activate), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 4);
	widget = gtk_file_chooser_dialog_new("Run program...", GTK_WINDOW(
				window), GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	g_signal_connect(G_OBJECT(widget), "response", G_CALLBACK(
				_on_run_choose_activate), entry);
	widget = gtk_file_chooser_button_new_with_dialog(widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, FALSE, 4);
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_EXECUTE);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				_on_run_execute), entry);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_size_group_add_widget(group, widget);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				_on_run_cancel), window);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 4);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show_all(window);
}

static int _run_error(char const * message, int ret)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE, "%s", message);
	gtk_widget_show(dialog);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return ret;
}

static gboolean _on_run_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	gtk_widget_hide(widget);
	gtk_main_quit();
	return FALSE;
}

static void _on_run_cancel(GtkWidget * widget, gpointer data)
{
	GtkWidget * window = data;

	gtk_widget_hide(window);
	gtk_main_quit();
}

static void _on_run_choose_activate(GtkWidget * widget, gint arg1,
		gpointer data)
{
	GtkWidget * entry = data;

	if(arg1 != GTK_RESPONSE_ACCEPT)
		return;
	gtk_entry_set_text(GTK_ENTRY(entry), gtk_file_chooser_get_filename(
				GTK_FILE_CHOOSER(widget)));
}

static void _on_run_execute(GtkWidget * widget, gpointer data)
{
	char const * path;

	widget = data;
	path = gtk_entry_get_text(GTK_ENTRY(widget));
	execlp("/bin/sh", "run", "-c", path, NULL);
	_run_error(strerror(errno), 0);
	exit(0);
}

static void _on_run_path_activate(GtkWidget * widget, gpointer data)
{
	_on_run_execute(NULL, widget);
}


/* main */
int main(int argc, char * argv[])
{
	gtk_init(&argc, &argv);
	_run();
	gtk_main();
	return 0;
}
