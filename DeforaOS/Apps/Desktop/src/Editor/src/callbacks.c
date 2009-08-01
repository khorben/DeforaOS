/* $Id$ */
static char const _copyright[] =
"Copyright (c) 2009 Pierre Pronchery <khorben@defora.org>";
/* This file is part of DeforaOS Desktop Editor */
static char const _license[] =
"Editor is free software; you can redistribute it and/or modify it under the\n"
"terms of the GNU General Public License version 3 as published by the Free\n"
"Software Foundation.\n"
"\n"
"Editor is distributed in the hope that it will be useful, but WITHOUT ANY\n"
"WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS\n"
"FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more\n"
"details.\n"
"\n"
"You should have received a copy of the GNU General Public License along with\n"
"Editor; if not, see <http://www.gnu.org/licenses/>.\n";



#include <stdlib.h>
#include "editor.h"
#include "callbacks.h"
#include "../config.h"


/* private */
/* constants */
static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};


/* public */
/* functions */
gboolean on_closex(GtkWidget * widget, GdkEvent * event, gpointer data)
{
	Editor * editor = data;

	return editor_close(editor);
}


/* on_edit_preferences */
static void _preferences_set(Editor * editor);
static void _preferences_on_cancel(GtkWidget * widget, gpointer data);
static gboolean _preferences_on_close(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static void _preferences_on_ok(GtkWidget * widget, gpointer data);

void on_edit_preferences(GtkWidget * widget, gpointer data)
{
	Editor * editor = data;
	PangoFontDescription * desc;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkSizeGroup * group;

	if(editor->pr_window != NULL)
	{
		gtk_widget_show(editor->pr_window);
		return;
	}
	desc = pango_font_description_new();
	pango_font_description_set_weight(desc, PANGO_WEIGHT_BOLD);
	editor->pr_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable(GTK_WINDOW(editor->pr_window), FALSE);
	gtk_window_set_title(GTK_WINDOW(editor->pr_window),
			"Text editor preferences");
	gtk_window_set_transient_for(GTK_WINDOW(editor->pr_window), GTK_WINDOW(
				editor->window));
	g_signal_connect(G_OBJECT(editor->pr_window), "delete_event",
			G_CALLBACK(_preferences_on_close), editor);
	vbox = gtk_vbox_new(FALSE, 0);
	/* dialog */
	hbox = gtk_hbox_new(FALSE, 0);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	widget = gtk_label_new("Font:");
	gtk_widget_modify_font(widget, desc);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	editor->pr_font = gtk_font_button_new();
	gtk_font_button_set_use_font(GTK_FONT_BUTTON(editor->pr_font), TRUE);
	gtk_size_group_add_widget(group, editor->pr_font);
	gtk_box_pack_start(GTK_BOX(hbox), editor->pr_font, TRUE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 4);
	hbox = gtk_hbox_new(FALSE, 0);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	widget = gtk_button_new_from_stock(GTK_STOCK_OK);
	gtk_size_group_add_widget(group, widget);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				_preferences_on_ok), editor);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_size_group_add_widget(group, widget);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				_preferences_on_cancel), editor);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 4);
	gtk_container_add(GTK_CONTAINER(editor->pr_window), vbox);
	_preferences_set(editor);
	gtk_widget_show_all(editor->pr_window);
}

static void _preferences_set(Editor * editor)
{
	gtk_font_button_set_font_name(GTK_FONT_BUTTON(editor->pr_font),
			editor->font);
}

static void _preferences_on_cancel(GtkWidget * widget, gpointer data)
{
	Editor * editor = data;

	gtk_widget_hide(editor->pr_window);
	_preferences_set(editor);
}

static gboolean _preferences_on_close(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	Editor * editor = data;

	_preferences_on_cancel(widget, editor);
	return FALSE;
}

static void _preferences_on_ok(GtkWidget * widget, gpointer data)
{
	Editor * editor = data;
	char const * font;

	gtk_widget_hide(editor->pr_window);
	font = gtk_font_button_get_font_name(GTK_FONT_BUTTON(editor->pr_font));
	editor_set_font(editor, font);
}


/* on_file_close */
void on_file_close(GtkWidget * widget, gpointer data)
{
	Editor * editor = data;

	editor_close(editor);
}


/* on_file_new */
void on_file_new(GtkWidget * widget, gpointer data)
{
	Editor * editor = data;

	editor_open(editor, NULL);
}


/* on_file_open */
void on_file_open(GtkWidget * widget, gpointer data)
{
	Editor * editor = data;

	editor_open_dialog(editor);
}


/* on_file_save */
void on_file_save(GtkWidget * widget, gpointer data)
{
	Editor * editor = data;

	editor_save(editor);
}


/* on_file_save_as */
void on_file_save_as(GtkWidget * widget, gpointer data)
{
	Editor * editor = data;

	editor_save_as_dialog(editor);
}


/* on_help_about */
static gboolean _about_on_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);
#if !GTK_CHECK_VERSION(2, 6, 0)
static void _about_on_close(GtkWidget * widget, gpointer data);
static void _about_on_credits(GtkWidget * widget, gpointer data);
static void _about_on_license(GtkWidget * widget, gpointer data);
#endif
void on_help_about(GtkWidget * widget, gpointer data)
{
	Editor * editor = data;
	static GtkWidget * window = NULL;
#if GTK_CHECK_VERSION(2, 6, 0)
	gsize cnt = 65536;
	gchar * buf;

	if(window != NULL)
	{
		gtk_widget_show(window);
		return;
	}
	if((buf = malloc(sizeof(*buf) * cnt)) == NULL)
	{
		editor_error(editor, "malloc", 0);
		return;
	}
	window = gtk_about_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(
				editor->window));
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(
				_about_on_closex), NULL);
	g_signal_connect(G_OBJECT(window), "response", G_CALLBACK(
				gtk_widget_hide), NULL);
	gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(window), PACKAGE);
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(window), VERSION);
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(window), _copyright);
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(window), _authors);
	if(g_file_get_contents("/usr/share/common-licenses/GPL-2", &buf, &cnt,
				NULL) == TRUE)
		gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(window), buf);
	else
		gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(window),
				_license);
	free(buf);
	gtk_widget_show(window);
}
#else
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * button;

	if(window != NULL)
	{
		gtk_widget_show(window);
		return;
	}
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(window), 4);
	gtk_window_set_title(GTK_WINDOW(window), "About Editor");
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(
				editor->window));
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(
				_about_on_closex), window);
	vbox = gtk_vbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(PACKAGE " " VERSION),
			FALSE, FALSE, 2); 
	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(_copyright), FALSE,
			FALSE, 2);
	hbox = gtk_hbox_new(TRUE, 4);
	button = gtk_button_new_with_mnemonic("C_redits");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(
				_about_on_credits), window);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, TRUE, 4);
	button = gtk_button_new_with_mnemonic("_License");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(
				_about_on_license), window);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, TRUE, 4);
	button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(
				_about_on_close), window);
	gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 4);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show_all(window);
}
#endif /* !GTK_CHECK_VERSION(2, 6, 0) */

static gboolean _about_on_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	gtk_widget_hide(widget);
	return TRUE;
}

#if !GTK_CHECK_VERSION(2, 6, 0)
static void _about_on_close(GtkWidget * widget, gpointer data)
{
        GtkWidget * window = data;

        gtk_widget_hide(window);
}

static void _about_on_credits(GtkWidget * widget, gpointer data)
{
	static GtkWidget * window = NULL;
	GtkWidget * about = data;
	GtkWidget * vbox;
	GtkWidget * notebook;
	GtkWidget * textview;
	GtkTextBuffer * tbuf;
	GtkTextIter iter;
	GtkWidget * hbox;
	size_t i;

	if(window != NULL)
	{
		gtk_widget_show(window);
		return;
	}
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
	gtk_container_set_border_width(GTK_CONTAINER(window), 4);
	gtk_window_set_title(GTK_WINDOW(window), "Credits");
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(about));
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(
				_about_on_closex), NULL);
	vbox = gtk_vbox_new(FALSE, 0);
	textview = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), FALSE);
	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	gtk_text_buffer_set_text(tbuf, "", 0);
	for(i = 0; _authors[i] != NULL; i++)
	{
		gtk_text_buffer_get_end_iter(tbuf, &iter);
		gtk_text_buffer_insert(tbuf, &iter, _authors[i], strlen(
					_authors[i]));
	}
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(widget),
			GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(widget), textview);
	notebook = gtk_notebook_new();
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget,
			gtk_label_new("Written by"));
	gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 4);
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				_about_on_close), window);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 4);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show_all(window);
}

static void _about_on_license(GtkWidget * widget, gpointer data)
{
	static GtkWidget * window = NULL;
	GtkWidget * about = data;
	GtkWidget * vbox;
	GtkWidget * textview;
	GtkTextBuffer * tbuf;
	GtkWidget * hbox;

	if(window != NULL)
	{
		gtk_widget_show(window);
		return;
	}
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
	gtk_container_set_border_width(GTK_CONTAINER(window), 4);
	gtk_window_set_title(GTK_WINDOW(window), "License");
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(about));
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(
				_about_on_closex), NULL);
	vbox = gtk_vbox_new(FALSE, 0);
	textview = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), FALSE);
	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	gtk_text_buffer_set_text(tbuf, _license, strlen(_license));
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(widget),
			GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(widget), textview);
	gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 4);
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	g_signal_connect(G_OBJECT(widget), "clicked",
			G_CALLBACK(_about_on_close), window);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 4);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show_all(window);
}
#endif /* !GTK_CHECK_VERSION(2, 6, 0) */


/* on_new */
void on_new(GtkWidget * widget, gpointer data)
{
	Editor * editor = data;

	editor_open(editor, NULL);
}


/* on_open */
void on_open(GtkWidget * widget, gpointer data)
{
	Editor * editor = data;

	editor_open_dialog(editor);
}


/* on_save */
void on_save(GtkWidget * widget, gpointer data)
{
	Editor * editor = data;

	editor_save(editor);
}


/* on_save_as */
void on_save_as(GtkWidget * widget, gpointer data)
{
	Editor * editor = data;

	editor_save_as_dialog(editor);
}
