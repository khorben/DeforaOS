/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Editor */
/* Editor is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 3 as published by the Free
 * Software Foundation.
 *
 * Editor is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Editor; if not, see <http://www.gnu.org/licenses/>. */



#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <gdk/gdkkeysyms.h>
#include <Desktop.h>
#include "callbacks.h"
#include "editor.h"
#include "../config.h"


/* constants */
#ifdef EMBEDDED
static DesktopAccel _editor_accel[] =
{
	{ G_CALLBACK(on_close), GDK_CONTROL_MASK, GDK_w },
	{ G_CALLBACK(on_new), GDK_CONTROL_MASK, GDK_n },
	{ G_CALLBACK(on_open), GDK_CONTROL_MASK, GDK_o },
	{ G_CALLBACK(on_preferences), GDK_CONTROL_MASK, GDK_p },
	{ G_CALLBACK(on_save), GDK_CONTROL_MASK, GDK_s },
	{ G_CALLBACK(on_save_as), GDK_CONTROL_MASK, GDK_S },
	{ NULL, 0, 0 }
};
#endif


#ifndef EMBEDDED
static DesktopMenu _editor_menu_file[] =
{
	{ "_New", G_CALLBACK(on_file_new), GTK_STOCK_NEW, GDK_n },
	{ "_Open", G_CALLBACK(on_file_open), GTK_STOCK_OPEN, GDK_o },
	{ "", NULL, NULL, 0 },
	{ "_Save", G_CALLBACK(on_file_save), GTK_STOCK_SAVE, GDK_s },
	{ "_Save as...", G_CALLBACK(on_file_save_as), GTK_STOCK_SAVE_AS,
		GDK_S },
	{ "", NULL, NULL, 0 },
	{ "_Close", G_CALLBACK(on_file_close), GTK_STOCK_CLOSE, 0 },
	{ NULL, NULL, NULL, 0 }
};

static DesktopMenu _editor_menu_edit[] =
{
	{ "_Undo", NULL, GTK_STOCK_UNDO, GDK_z }, /* FIXME implement */
	{ "_Redo", NULL, GTK_STOCK_REDO, GDK_r }, /* FIXME implement */
	{ "", NULL, NULL, 0 },
	{ "_Cut", NULL, GTK_STOCK_CUT, 0 }, /* FIXME implement */
	{ "_Copy", NULL, GTK_STOCK_COPY, 0 }, /* FIXME implement */
	{ "_Paste", NULL, GTK_STOCK_PASTE, 0 }, /* FIXME implement */
	{ "", NULL, NULL, 0 },
	{ "_Preferences", G_CALLBACK(on_edit_preferences),
		GTK_STOCK_PREFERENCES, GDK_p },
	{ NULL, NULL, NULL, 0 }
};

static DesktopMenu _editor_menu_help[] =
{
#if GTK_CHECK_VERSION(2, 6, 0)
	{ "_About", G_CALLBACK(on_help_about), GTK_STOCK_ABOUT, 0 },
#else
	{ "_About", G_CALLBACK(on_help_about), NULL, 0 },
#endif
	{ NULL, NULL, NULL, 0 }
};

static DesktopMenubar _editor_menubar[] =
{
	{ "_File", _editor_menu_file },
	{ "_Edit", _editor_menu_edit },
	{ "_Help", _editor_menu_help },
	{ NULL, NULL }
};
#endif

static DesktopToolbar _editor_toolbar[] =
{
	{ "New", G_CALLBACK(on_new), GTK_STOCK_NEW, 0, NULL },
	{ "Open", G_CALLBACK(on_open), GTK_STOCK_OPEN, 0, NULL },
	{ "", NULL, NULL, 0, NULL },
	{ "Save", G_CALLBACK(on_save), GTK_STOCK_SAVE, 0, NULL },
	{ "Save as", G_CALLBACK(on_save_as), GTK_STOCK_SAVE_AS, 0, NULL },
#ifdef EMBEDDED
	{ "", NULL, NULL, 0, NULL },
	{ "Preferences", G_CALLBACK(on_preferences), GTK_STOCK_PREFERENCES, 0,
		NULL },
#endif
	{ NULL, NULL, NULL, 0, NULL }
};


/* Editor */
/* editor_new */
static void _new_set_title(Editor * editor);

Editor * editor_new(void)
{
	Editor * editor;
	GtkAccelGroup * group;
	GtkSettings * settings;
	GtkWidget * vbox;
	GtkWidget * widget;
	PangoFontDescription * desc;

	if((editor = malloc(sizeof(*editor))) == NULL)
		return NULL;
	editor->font = NULL;
	settings = gtk_settings_get_default();
	g_object_get(G_OBJECT(settings), "gtk-font-name", &editor->font, NULL);
	if(editor->font == NULL)
		editor->font = EDITOR_DEFAULT_FONT;
	editor->filename = NULL;
	/* widgets */
	group = gtk_accel_group_new();
	editor->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_add_accel_group(GTK_WINDOW(editor->window), group);
	gtk_window_set_default_size(GTK_WINDOW(editor->window), 600, 400);
	_new_set_title(editor);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(editor->window), "text-editor");
#endif
	g_signal_connect(G_OBJECT(editor->window), "delete-event", G_CALLBACK(
			on_closex), editor);
	vbox = gtk_vbox_new(FALSE, 0);
	/* menubar */
#ifndef EMBEDDED
	widget = desktop_menubar_create(_editor_menubar, editor, group);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, FALSE, 0);
#else
	desktop_accel_create(_editor_accel, editor, group);
#endif
	/* toolbar */
	widget = desktop_toolbar_create(_editor_toolbar, editor, group);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, FALSE, 0);
	/* view */
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	editor->view = gtk_text_view_new();
	/* FIXME make it an option */
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(editor->view),
			GTK_WRAP_WORD_CHAR);
	desc = pango_font_description_from_string(editor->font);
	pango_font_description_set_family(desc, "monospace");
	editor_set_font(editor, pango_font_description_to_string(desc));
	pango_font_description_free(desc);
	gtk_container_add(GTK_CONTAINER(widget), editor->view);
	gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	/* statusbar */
	editor->statusbar = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(vbox), editor->statusbar, FALSE, FALSE, 0);
	/* preferences */
	editor->pr_window = NULL;
	gtk_container_add(GTK_CONTAINER(editor->window), vbox);
	gtk_window_set_focus(GTK_WINDOW(editor->window), editor->view);
	gtk_widget_show_all(editor->window);
	return editor;
}

static void _new_set_title(Editor * editor)
{
	char buf[256];

	snprintf(buf, sizeof(buf), "%s%s", "Text editor - ", editor->filename
		       	== NULL ? "(Untitled)" : editor->filename);
	gtk_window_set_title(GTK_WINDOW(editor->window), buf);
}


/* editor_delete */
void editor_delete(Editor * editor)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	free(editor);
}


/* accessors */
void editor_set_font(Editor * editor, char const * font)
{
	PangoFontDescription * desc;

	editor->font = font;
	desc = pango_font_description_from_string(font);
	gtk_widget_modify_font(editor->view, desc);
	pango_font_description_free(desc);
}


/* useful */
/* editor_error */
int editor_error(Editor * editor, char const * message, int ret)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(editor->window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s",
#if GTK_CHECK_VERSION(2, 6, 0)
			"Error");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s",
#endif
			message);
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(
				gtk_widget_destroy), NULL);
	gtk_widget_show(dialog);
	return ret;
}


/* editor_close */
gboolean editor_close(Editor * editor)
{
	GtkWidget * dialog;
	int res;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(gtk_text_buffer_get_modified(gtk_text_view_get_buffer(GTK_TEXT_VIEW(
						editor->view))) == FALSE)
	{
		gtk_main_quit();
		return FALSE;
	}
	dialog = gtk_message_dialog_new(GTK_WINDOW(editor->window),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_WARNING, GTK_BUTTONS_NONE, "%s",
#if GTK_CHECK_VERSION(2, 6, 0)
			"Warning");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s",
#endif
			"There are unsaved changes.\n"
			"Are you sure you want to close?");
	gtk_dialog_add_buttons(GTK_DIALOG(dialog), GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, GTK_STOCK_CLOSE,
			GTK_RESPONSE_CLOSE, NULL);
	gtk_window_set_title(GTK_WINDOW(dialog), "Warning");
	res = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	if(res != GTK_RESPONSE_CLOSE)
		return TRUE;
	gtk_main_quit();
	return FALSE;
}


/* editor_open */
void editor_open(Editor * editor, char const * filename)
{
	GtkWidget * dialog;
	int res;
	FILE * fp;
	GtkTextBuffer * tbuf;
	GtkTextIter iter;
	char buf[BUFSIZ];
	size_t len;
	char * p;
	size_t rlen;
	size_t wlen;
	GError * error = NULL;

	if(gtk_text_buffer_get_modified(gtk_text_view_get_buffer(GTK_TEXT_VIEW(
						editor->view))) == TRUE)
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(editor->window),
				GTK_DIALOG_MODAL
				| GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_WARNING, GTK_BUTTONS_NONE, "%s",
#if GTK_CHECK_VERSION(2, 6, 0)
				"Warning");
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(
					dialog), "%s",
#endif
				"There are unsaved changes.\n"
				"Are you sure you want to discard them?");
		gtk_dialog_add_buttons(GTK_DIALOG(dialog), GTK_STOCK_CANCEL,
				GTK_RESPONSE_CANCEL,
#if GTK_CHECK_VERSION(2, 12, 0)
				GTK_STOCK_DISCARD,
#else
				"Discard",
#endif
				GTK_RESPONSE_CLOSE, NULL);
		gtk_window_set_title(GTK_WINDOW(dialog), "Warning");
		res = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		if(res != GTK_RESPONSE_CLOSE)
			return;
	}
	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->view));
	gtk_text_buffer_set_text(tbuf, "", 0);
	if(filename == NULL)
	{
		gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(
					gtk_text_view_get_buffer(GTK_TEXT_VIEW(
							editor->view))), FALSE);
		return;
	}
	if((fp = fopen(filename, "r")) == NULL)
	{
		snprintf(buf, sizeof(buf), "%s: %s", filename, strerror(errno));
		editor_error(editor, buf, 1);
		return;
	}
	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->view));
	gtk_text_buffer_set_text(tbuf, "", 0);
	while((len = fread(buf, sizeof(char), sizeof(buf), fp)) > 0)
	{
		gtk_text_buffer_get_end_iter(tbuf, &iter);
#if 0
		if((p = g_convert(buf, len, "UTF-8", "ISO-8859-15", &rlen, &wlen, NULL)) != NULL)
		{
			gtk_text_buffer_insert(tbuf, &iter, p, wlen);
			g_free(p);
		}
		else
			gtk_text_buffer_insert(tbuf, &iter, buf, len);
#else
		if((p = g_locale_to_utf8(buf, len, &rlen, &wlen, &error))
				!= NULL)
			/* FIXME may lose characters */
			gtk_text_buffer_insert(tbuf, &iter, p, wlen);
		else
			gtk_text_buffer_insert(tbuf, &iter, buf, len);
#endif
	}
	fclose(fp);
	gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(gtk_text_view_get_buffer(
					GTK_TEXT_VIEW(editor->view))), FALSE);
	editor->filename = g_strdup(filename);
	_new_set_title(editor); /* XXX make it a generic private function */
}


/* editor_open_dialog */
void editor_open_dialog(Editor * editor)
{
	GtkWidget * dialog;
	char * filename = NULL;

	dialog = gtk_file_chooser_dialog_new("Open file...",
			GTK_WINDOW(editor->window),
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
					dialog));
	gtk_widget_destroy(dialog);
	if(filename == NULL)
		return;
	editor_open(editor, filename);
	g_free(filename);
}


/* editor_save */
gboolean editor_save(Editor * editor)
{
	FILE * fp;
	GtkTextBuffer * tbuf;
	GtkTextIter start;
	GtkTextIter end;
	char * buf;
	size_t len;

	if(editor->filename == NULL)
	{
		editor_save_as_dialog(editor);
		return FALSE;
	}
	if((fp = fopen(editor->filename, "w")) == NULL)
	{
		buf = g_strdup_printf("%s: %s", editor->filename, strerror(
					errno));
		editor_error(editor, buf, 1);
		g_free(buf);
		return FALSE;
	}
	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->view));
	/* FIXME allocating the complete file is not optimal */
	gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(tbuf), &start);
	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(tbuf), &end);
	buf = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(tbuf), &start, &end,
			FALSE);
	len = strlen(buf);
	if(fwrite(buf, sizeof(char), len, fp) != len)
	{
		g_free(buf);
		fclose(fp);
		editor_error(editor, "Partial write", 0);
		return FALSE;
	}
	g_free(buf);
	fclose(fp);
	gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(tbuf), FALSE);
	return TRUE;
}


/* editor_save_as */
gboolean editor_save_as(Editor * editor, char const * filename)
{
	struct stat st;
	GtkWidget * dialog;
	int ret;

	if(stat(filename, &st) == 0)
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(editor->window),
				GTK_DIALOG_MODAL
				| GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO, "%s",
#if GTK_CHECK_VERSION(2, 6, 0)
				"Warning");
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(
					dialog), "%s",
#endif
				"File exists. Overwrite?");
		gtk_window_set_title(GTK_WINDOW(dialog), "Warning");
		ret = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		if(ret == GTK_RESPONSE_NO)
			return FALSE;
	}
	g_free(editor->filename);
	if((editor->filename = g_strdup(filename)) == NULL)
		return editor_error(editor, "Allocation error", FALSE);
	if(editor_save(editor) != TRUE)
		return FALSE;
	_new_set_title(editor);
	return TRUE;
}


/* editor_save_as_dialog */
gboolean editor_save_as_dialog(Editor * editor)
{
	GtkWidget * dialog;
	char * filename = NULL;
	gboolean ret;

	dialog = gtk_file_chooser_dialog_new("Save as...",
			GTK_WINDOW(editor->window),
			GTK_FILE_CHOOSER_ACTION_SAVE,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
					dialog));
	gtk_widget_destroy(dialog);
	if(filename == NULL)
		return FALSE;
	ret = editor_save_as(editor, filename);
	g_free(filename);
	return ret;
}
