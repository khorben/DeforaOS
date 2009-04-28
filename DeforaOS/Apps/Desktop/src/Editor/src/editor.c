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
#include "callbacks.h"
#include "editor.h"
#include "../config.h"


/* types */
#ifndef FOR_EMBEDDED
struct _menu
{
	char * name;
	GtkSignalFunc callback;
	char * stock;
	unsigned int accel;
};

struct _menubar
{
	char * name;
	struct _menu * menu;
};
#endif


/* constants */
#ifndef FOR_EMBEDDED
static struct _menu _menu_file[] =
{
	{ "_New", G_CALLBACK(on_file_new), GTK_STOCK_NEW, GDK_N },
	{ "_Open", G_CALLBACK(on_file_open), GTK_STOCK_OPEN, GDK_O },
	{ "", NULL, NULL, 0 },
	{ "_Save", G_CALLBACK(on_file_save), GTK_STOCK_SAVE, GDK_S },
	{ "_Save as...", G_CALLBACK(on_file_save_as), GTK_STOCK_SAVE_AS,
		GDK_A },
	{ "", NULL, NULL, 0 },
	{ "_Close", G_CALLBACK(on_file_close), GTK_STOCK_CLOSE, 0 },
	{ NULL, NULL, NULL, 0 }
};

static struct _menu _menu_edit[] =
{
	{ "_Undo", NULL, GTK_STOCK_UNDO, GDK_Z }, /* FIXME implement */
	{ "_Redo", NULL, GTK_STOCK_REDO, GDK_R }, /* FIXME implement */
	{ "", NULL, NULL, 0 },
	{ "_Cut", NULL, GTK_STOCK_CUT, 0 }, /* FIXME implement */
	{ "_Copy", NULL, GTK_STOCK_COPY, 0 }, /* FIXME implement */
	{ "_Paste", NULL, GTK_STOCK_PASTE, 0 }, /* FIXME implement */
	{ "", NULL, NULL, 0 },
	{ "_Preferences", G_CALLBACK(on_edit_preferences),
		GTK_STOCK_PREFERENCES, GDK_P },
	{ NULL, NULL, NULL, 0 }
};

static struct _menu _menu_help[] =
{
#if GTK_CHECK_VERSION(2, 6, 0)
	{ "_About", G_CALLBACK(on_help_about), GTK_STOCK_ABOUT, 0 },
#else
	{ "_About", G_CALLBACK(on_help_about), NULL, 0 },
#endif
	{ NULL, NULL, NULL, 0 }
};

static struct _menubar _menubar[] =
{
	{ "_File", _menu_file },
	{ "_Edit", _menu_edit },
	{ "_Help", _menu_help },
	{ NULL, NULL }
};
#endif


/* Editor */
/* editor_new */
static void _new_set_title(Editor * editor);
#ifndef FOR_EMBEDDED
static GtkWidget * _new_menubar(Editor * editor);
#endif

Editor * editor_new(void)
{
	Editor * editor;
	GtkWidget * vbox;
	GtkWidget * toolbar;
	GtkToolItem * tb_button;
	GtkWidget * widget;
	PangoFontDescription * desc;

	if((editor = malloc(sizeof(*editor))) == NULL)
		return NULL;
	editor->filename = NULL;
	/* widgets */
	editor->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(editor->window), 600, 400);
	_new_set_title(editor);
	g_signal_connect(G_OBJECT(editor->window), "delete-event", G_CALLBACK(
			on_closex), editor);
	vbox = gtk_vbox_new(FALSE, 0);
	/* menubar */
#ifndef FOR_EMBEDDED
	gtk_box_pack_start(GTK_BOX(vbox), _new_menubar(editor), FALSE, FALSE,
			0);
#endif
	/* toolbar */
	toolbar = gtk_toolbar_new();
	tb_button = gtk_tool_button_new_from_stock(GTK_STOCK_NEW);
	g_signal_connect(G_OBJECT(tb_button), "clicked", G_CALLBACK(on_new),
			editor);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tb_button, -1);
	tb_button = gtk_tool_button_new_from_stock(GTK_STOCK_OPEN);
	g_signal_connect(G_OBJECT(tb_button), "clicked", G_CALLBACK(on_open),
			editor);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tb_button, -1);
	tb_button = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tb_button, -1);
	tb_button = gtk_tool_button_new_from_stock(GTK_STOCK_SAVE);
	g_signal_connect(G_OBJECT(tb_button), "clicked", G_CALLBACK(on_save),
			editor);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tb_button, -1);
	tb_button = gtk_tool_button_new_from_stock(GTK_STOCK_SAVE_AS);
	g_signal_connect(G_OBJECT(tb_button), "clicked", G_CALLBACK(
				on_save_as), editor);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tb_button, -1);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
	/* view */
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	editor->view = gtk_text_view_new();
	/* FIXME make it an option */
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(editor->view),
			GTK_WRAP_WORD_CHAR);
	desc = pango_font_description_new();
	pango_font_description_set_family(desc, "monospace");
	gtk_widget_modify_font(editor->view, desc);
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

#ifndef FOR_EMBEDDED
static GtkWidget * _new_menubar(Editor * editor)
{
	GtkWidget * tb_menubar;
	GtkAccelGroup * group;
	GtkWidget * menu;
	GtkWidget * menubar;
	GtkWidget * menuitem;
	unsigned int i;
	unsigned int j;
	struct _menu * p;

	tb_menubar = gtk_menu_bar_new();
	group = gtk_accel_group_new();
	for(i = 0; _menubar[i].name != NULL; i++)
	{
		menubar = gtk_menu_item_new_with_mnemonic(_menubar[i].name);
		menu = gtk_menu_new();
		for(j = 0; _menubar[i].menu[j].name != NULL; j++)
		{
			p = &_menubar[i].menu[j];
			if(p->name[0] == '\0')
				menuitem = gtk_separator_menu_item_new();
			else if(p->stock == 0)
				menuitem = gtk_menu_item_new_with_mnemonic(
						p->name);
			else
				menuitem = gtk_image_menu_item_new_from_stock(
						p->stock, NULL);
			if(p->callback != NULL)
				g_signal_connect(G_OBJECT(menuitem), "activate",
						G_CALLBACK(p->callback),
						editor);
			else
				gtk_widget_set_sensitive(menuitem, FALSE);
			if(p->accel != 0)
				gtk_widget_add_accelerator(menuitem, "activate",
						group, p->accel,
						GDK_CONTROL_MASK,
						GTK_ACCEL_VISIBLE);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		}
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(menubar), menu);
		gtk_menu_bar_append(GTK_MENU_BAR(tb_menubar), menubar);
	}
	gtk_window_add_accel_group(GTK_WINDOW(editor->window), group);
	return tb_menubar;
}
#endif


/* editor_delete */
void editor_delete(Editor * editor)
{
	free(editor);
}


/* useful */
/* editor_error */
int editor_error(Editor * editor, char const * message, int ret)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(editor->window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", message);
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

	if(gtk_text_buffer_get_modified(gtk_text_view_get_buffer(GTK_TEXT_VIEW(
						editor->view))) == FALSE)
	{
		gtk_main_quit();
		return FALSE;
	}
	dialog = gtk_message_dialog_new(GTK_WINDOW(editor->window),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO, "%s",
			"There are unsaved changes.\n"
			"Are you sure you want to close?");
	gtk_window_set_title(GTK_WINDOW(dialog), "Warning");
	res = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	if(res == GTK_RESPONSE_NO)
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

	if(gtk_text_buffer_get_modified(gtk_text_view_get_buffer(GTK_TEXT_VIEW(
						editor->view))) == TRUE)
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(editor->window),
				GTK_DIALOG_MODAL
				| GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO, "%s",
				"There are unsaved changes.\n"
				"Are you sure you want to lose them?");
		gtk_window_set_title(GTK_WINDOW(dialog), "Warning");
		res = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		if(res == GTK_RESPONSE_NO)
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
		gtk_text_buffer_insert(tbuf, &iter, buf, len);
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
