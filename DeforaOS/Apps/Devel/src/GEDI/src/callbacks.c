/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel GEDI */
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



#include "gedi.h"
#include "callbacks.h"
#include "../config.h"


/* functions */
/* callbacks */
/* _on_closex */
gboolean on_closex(gpointer data)
{
	on_file_exit(data);
	return FALSE;
}


/* on_file_new */
void on_file_new(gpointer data)
{
	GEDI * gedi = data;

	gedi_file_open(gedi, NULL); /* XXX hack */
}


/* on_file_open */
void on_file_open(gpointer data)
{
	GEDI * gedi = data;
	GtkWidget * dialog;
	char * file;

	dialog = gtk_file_chooser_dialog_new ("Open file...", NULL,
			GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
			GTK_RESPONSE_ACCEPT, NULL);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		gedi_file_open(gedi, file);
		g_free(file);
	}
	gtk_widget_destroy(dialog);
}


/* on_file_preferences */
static void _file_preferences_new(GEDI * g);
static gboolean _on_preferences_closex(gpointer data);
static void _on_preferences_apply(GtkWidget * widget, gpointer data);
static void _on_preferences_cancel(GtkWidget * widget, gpointer data);
static void _on_preferences_ok(GtkWidget * widget, gpointer data);

void on_file_preferences(gpointer data)
{
	GEDI * g = data;

	if(g->pr_window == NULL)
		_file_preferences_new(g);
	gtk_widget_show_all(g->pr_window);
}

static void _file_preferences_new(GEDI * g)
{
	GtkWidget * vbox;
	GtkWidget * nb;
	GtkWidget * nb_vbox;
	GtkWidget * hbox;
	GtkWidget * b_ok;
	GtkWidget * b_apply;
	GtkWidget * b_cancel;

	g->pr_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(g->pr_window), 4);
	gtk_window_set_title(GTK_WINDOW(g->pr_window), "Preferences");
	g_signal_connect_swapped(G_OBJECT(g->pr_window), "delete-event",
			G_CALLBACK(_on_preferences_closex), g->pr_window);
	vbox = gtk_vbox_new(FALSE, 4);
	nb = gtk_notebook_new();
	/* notebook page editor */
	nb_vbox = gtk_vbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(nb_vbox), gtk_label_new("Choose editor"),
			FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(nb_vbox), gtk_label_new(
				"Program executable"), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(nb_vbox), gtk_label_new(
				"Runs in a terminal"), FALSE, FALSE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(nb), nb_vbox, gtk_label_new(
				"Editor"));
	/* notebook page plug-ins */
	nb_vbox = gtk_vbox_new(FALSE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(nb), nb_vbox, gtk_label_new(
				"Plug-ins"));
	gtk_box_pack_start(GTK_BOX(vbox), nb, TRUE, TRUE, 0);
	/* buttons */
	hbox = gtk_hbox_new(TRUE, 4);
	b_ok = gtk_button_new_from_stock(GTK_STOCK_OK);
	g_signal_connect(G_OBJECT(b_ok), "clicked", G_CALLBACK(
				_on_preferences_ok), g);
	gtk_box_pack_end(GTK_BOX(hbox), b_ok, FALSE, TRUE, 0);
	b_apply = gtk_button_new_from_stock(GTK_STOCK_APPLY);
	g_signal_connect(G_OBJECT(b_apply), "clicked", G_CALLBACK(
				_on_preferences_apply), g);
	gtk_box_pack_end(GTK_BOX(hbox), b_apply, FALSE, TRUE, 0);
	b_cancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	g_signal_connect(G_OBJECT(b_cancel), "clicked", G_CALLBACK(
				_on_preferences_cancel), g);
	gtk_box_pack_end(GTK_BOX(hbox), b_cancel, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(g->pr_window), vbox);
}


/* _on_preferences_apply */
static void _on_preferences_apply(GtkWidget * widget, gpointer data)
{
	/* FIXME implement */
}


/* _on_preferences_cancel */
static void _on_preferences_cancel(GtkWidget * widget, gpointer data)
{
	GEDI * gedi = data;

	/* FIXME implement */
	gtk_widget_hide(gedi->pr_window);
}


/* _on_preferences_ok */
static void _on_preferences_ok(GtkWidget * widget, gpointer data)
{
	GEDI * g = data;

	_on_preferences_apply(widget, data);
	gtk_widget_hide(g->pr_window);
}


/* _on_preferences_closex */
static gboolean _on_preferences_closex(gpointer data)
{
	GtkWidget * widget = data;

	gtk_widget_hide(widget);
	return TRUE;
}


/* on_file_exit */
void on_file_exit(gpointer data)
{
	/* FIXME check that everything is properly saved */
	gtk_main_quit();
}


/* on_help_about */
void on_help_about(gpointer data)
{
	GEDI * gedi = data;

	gedi_about(gedi);
}


/* on_project_new */
void on_project_new(gpointer data)
{
	/* FIXME implement: assistant, templates... */
}


/* on_project_open */
void on_project_open(gpointer data)
{
	GEDI * gedi = data;
	GtkWidget * dialog;
	char * filename = NULL;

	dialog = gtk_file_chooser_dialog_new("Open project...",
			GTK_WINDOW(gedi->tb_window),
			GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
			GTK_RESPONSE_ACCEPT, NULL);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
					dialog));
	gtk_widget_destroy(dialog);
	if(filename == NULL)
		return;
	gedi_project_open(gedi, filename);
	g_free(filename);
}


/* on_project_properties */
void on_project_properties(gpointer data)
{
	GEDI * gedi = data;

	if(gedi->cur == NULL)
	{
		/* FIXME should not happen (disable callback action) */
		gedi_error(gedi, "No project opened", 0);
		return;
	}
	project_properties(gedi->cur);
}


/* on_project_save */
void on_project_save(gpointer data)
{
	GEDI * g = data;

	/* FIXME implement */
	gedi_project_save(g);
}


/* on_project_save_as */
void on_project_save_as(gpointer data)
{
	GEDI * g = data;
	GtkWidget * dialog;
	char * file;

	dialog = gtk_file_chooser_dialog_new("Save project as...", NULL,
			GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
			GTK_RESPONSE_ACCEPT, NULL);
	/* FIXME add options? (recursive save) */
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		gedi_project_save_as(g, file);
		g_free(file);
	}
	gtk_widget_destroy(dialog);
}
