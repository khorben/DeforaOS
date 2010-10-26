/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Editor */
/* This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>. */



#include <stdlib.h>
#include <libintl.h>
#include "editor.h"
#include "callbacks.h"
#include "../config.h"
#define _(string) gettext(string)


/* public */
/* functions */
gboolean on_closex(gpointer data)
{
	Editor * editor = data;

	return editor_close(editor);
}


/* on_edit_find */
void on_edit_find(gpointer data)
{
	Editor * editor = data;

	editor_find(editor, NULL);
}


/* on_edit_preferences */
static void _preferences_set(Editor * editor);
static gboolean _preferences_on_closex(gpointer data);
static void _preferences_on_response(GtkWidget * widget, gint response,
		gpointer data);
static void _preferences_on_cancel(gpointer data);
static void _preferences_on_ok(gpointer data);

void on_edit_preferences(gpointer data)
{
	Editor * editor = data;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;
	GtkSizeGroup * group;

	if(editor->pr_window != NULL)
	{
		gtk_widget_show(editor->pr_window);
		return;
	}
	editor->pr_window = gtk_dialog_new_with_buttons(
			_("Text editor preferences"),
			GTK_WINDOW(editor->window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
	g_signal_connect_swapped(G_OBJECT(editor->pr_window), "delete-event",
			G_CALLBACK(_preferences_on_closex), editor);
	g_signal_connect(G_OBJECT(editor->pr_window), "response",
			G_CALLBACK(_preferences_on_response), editor);
#if GTK_CHECK_VERSION(2, 14, 0)
	vbox = gtk_dialog_get_content_area(GTK_DIALOG(editor->pr_window));
#else
	vbox = GTK_DIALOG(editor->pr_window)->vbox;
#endif
	hbox = gtk_hbox_new(FALSE, 0);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	/* font */
	widget = gtk_label_new(_("Font:"));
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	editor->pr_font = gtk_font_button_new();
	gtk_font_button_set_use_font(GTK_FONT_BUTTON(editor->pr_font), TRUE);
	gtk_size_group_add_widget(group, editor->pr_font);
	gtk_box_pack_start(GTK_BOX(hbox), editor->pr_font, TRUE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 4);
	_preferences_set(editor);
	gtk_widget_show_all(editor->pr_window);
}

static void _preferences_set(Editor * editor)
{
	gtk_font_button_set_font_name(GTK_FONT_BUTTON(editor->pr_font),
			editor_get_font(editor));
}

static gboolean _preferences_on_closex(gpointer data)
{
	Editor * editor = data;

	_preferences_on_cancel(editor);
	return TRUE;
}

static void _preferences_on_response(GtkWidget * widget, gint response,
		gpointer data)
{
	gtk_widget_hide(widget);
	if(response == GTK_RESPONSE_OK)
		_preferences_on_ok(data);
	else if(response == GTK_RESPONSE_CANCEL)
		_preferences_on_cancel(data);
}

static void _preferences_on_cancel(gpointer data)
{
	Editor * editor = data;

	gtk_widget_hide(editor->pr_window);
	_preferences_set(editor);
}

static void _preferences_on_ok(gpointer data)
{
	Editor * editor = data;
	char const * font;

	gtk_widget_hide(editor->pr_window);
	font = gtk_font_button_get_font_name(GTK_FONT_BUTTON(editor->pr_font));
	editor_set_font(editor, font);
	editor_config_save(editor);
}


/* on_edit_select_all */
void on_edit_select_all(gpointer data)
{
	Editor * editor = data;

	editor_select_all(editor);
}


/* on_edit_unselect_all */
void on_edit_unselect_all(gpointer data)
{
	Editor * editor = data;

	editor_unselect_all(editor);
}


/* on_file_close */
void on_file_close(gpointer data)
{
	Editor * editor = data;

	editor_close(editor);
}


/* on_file_new */
void on_file_new(gpointer data)
{
	Editor * editor = data;

	editor_open(editor, NULL);
}


/* on_file_open */
void on_file_open(gpointer data)
{
	Editor * editor = data;

	editor_open_dialog(editor);
}


/* on_file_save */
void on_file_save(gpointer data)
{
	Editor * editor = data;

	editor_save(editor);
}


/* on_file_save_as */
void on_file_save_as(gpointer data)
{
	Editor * editor = data;

	editor_save_as_dialog(editor);
}


/* on_help_about */
void on_help_about(gpointer data)
{
	Editor * editor = data;

	editor_about(editor);
}


/* toolbar */
/* on_close */
void on_close(gpointer data)
{
	Editor * editor = data;

	editor_close(editor);
}


/* on_new */
void on_new(gpointer data)
{
	Editor * editor = data;

	editor_open(editor, NULL);
}


/* on_open */
void on_open(gpointer data)
{
	Editor * editor = data;

	editor_open_dialog(editor);
}


/* on_save */
void on_save(gpointer data)
{
	Editor * editor = data;

	editor_save(editor);
}


/* on_save_as */
void on_save_as(gpointer data)
{
	Editor * editor = data;

	editor_save_as_dialog(editor);
}


/* on_preferences */
void on_preferences(gpointer data)
{
	Editor * editor = data;

	on_edit_preferences(editor);
}
