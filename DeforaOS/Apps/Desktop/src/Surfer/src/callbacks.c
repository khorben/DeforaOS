/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Surfer */
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
#include <string.h>
#include <errno.h>
#include <libintl.h>
#include "surfer.h"
#include "ghtml.h"
#include "callbacks.h"
#include "common.h"
#define _(string) gettext(string)


/* window */
gboolean on_closex(gpointer data)
{
	Surfer * surfer = data;
	GtkWidget * dialog;
	GtkDialogFlags f = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
	int res;

	if(gtk_notebook_get_n_pages(GTK_NOTEBOOK(surfer->notebook)) > 1)
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(surfer->window), f,
				GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, "%s",
#if GTK_CHECK_VERSION(2, 8, 0)
				_("Question"));
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(
					dialog), "%s",
#endif
				_("There are multiple tabs opened.\n"
					"Do you really want to close every tab"
					" opened in this window?"));
		gtk_dialog_add_buttons(GTK_DIALOG(dialog),
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, NULL);
		gtk_window_set_title(GTK_WINDOW(dialog), _("Question"));
		res = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		if(res != GTK_RESPONSE_CLOSE)
			return TRUE;
	}
	surfer_delete(surfer);
	return FALSE;
}


#ifndef EMBEDDED
/* file menu */
void on_file_close(gpointer data)
{
	on_closex(data);
}


/* on_file_close_tab */
void on_file_close_tab(gpointer data)
{
	Surfer * surfer = data;

	surfer_close_tab(surfer, NULL);
}


/* on_file_new_tab */
void on_file_new_tab(gpointer data)
{
	Surfer * surfer = data;

	surfer_open_tab(surfer, NULL);
}


/* on_file_new_window */
void on_file_new_window(gpointer data)
{
	Surfer * surfer = data;
	
	surfer_new_copy(surfer);
}


/* on_file_open */
void on_file_open(gpointer data)
{
	Surfer * surfer = data;

	surfer_open_dialog(surfer);
}


/* on_file_open_url */
void on_file_open_url(gpointer data)
{
	Surfer * surfer = data;

	surfer_open(surfer, NULL);
}


/* on_file_print */
void on_file_print(gpointer data)
{
	Surfer * surfer = data;

	surfer_print(surfer);
}


/* on_file_save_as */
void on_file_save_as(gpointer data)
{
	Surfer * surfer = data;

	surfer_save(surfer, NULL);
}


/* edit menu */
/* on_edit_undo */
void on_edit_undo(gpointer data)
{
	Surfer * surfer = data;

	surfer_undo(surfer);
}


/* on_edit_redo */
void on_edit_redo(gpointer data)
{
	Surfer * surfer = data;

	surfer_redo(surfer);
}


/* on_edit_copy */
void on_edit_copy(gpointer data)
{
	Surfer * surfer = data;

	surfer_copy(surfer);
}


/* on_edit_cut */
void on_edit_cut(gpointer data)
{
	Surfer * surfer = data;

	surfer_cut(surfer);
}


/* on_edit_find */
void on_edit_find(gpointer data)
{
	Surfer * surfer = data;

	surfer_find(surfer, NULL);
}


/* on_edit_paste */
void on_edit_paste(gpointer data)
{
	Surfer * surfer = data;

	surfer_paste(surfer);
}


/* on_edit_preferences */
void on_edit_preferences(gpointer data)
{
	on_preferences(data);
}


/* on_edit_select_all */
void on_edit_select_all(gpointer data)
{
	Surfer * surfer = data;

	surfer_select_all(surfer);
}


/* on_edit_unselect_all */
void on_edit_unselect_all(gpointer data)
{
	Surfer * surfer = data;

	surfer_unselect_all(surfer);
}


/* view menu */
/* on_view_force_refresh */
void on_view_force_refresh(gpointer data)
{
	Surfer * surfer = data;

	surfer_reload(surfer);
}


/* on_view_javascript_console */
void on_view_javascript_console(gpointer data)
{
	Surfer * surfer = data;

	surfer_show_console(surfer, TRUE);
}


/* on_view_normal_size */
void on_view_normal_size(gpointer data)
{
	Surfer * surfer = data;

	surfer_zoom_reset(surfer);
}


/* on_view_page_source */
void on_view_page_source(gpointer data)
{
	Surfer * surfer = data;

	surfer_view_source(surfer);
}


/* on_view_refresh */
void on_view_refresh(gpointer data)
{
	Surfer * surfer = data;

	surfer_refresh(surfer);
}


/* on_view_stop */
void on_view_stop(gpointer data)
{
	Surfer * surfer = data;

	surfer_stop(surfer);
}


/* on_view_zoom_in */
void on_view_zoom_in(gpointer data)
{
	Surfer * surfer = data;

	surfer_zoom_in(surfer);
}


/* on_view_zoom_out */
void on_view_zoom_out(gpointer data)
{
	Surfer * surfer = data;

	surfer_zoom_out(surfer);
}


/* help menu */
/* on_help_about */
void on_help_about(gpointer data)
{
	Surfer * surfer = data;

	surfer_about(surfer);
}
#endif /* !EMBEDDED */


/* toolbar */
/* on_back */
void on_back(gpointer data)
{
	Surfer * surfer = data;

	surfer_go_back(surfer);
}


/* on_close */
void on_close(gpointer data)
{
	Surfer * surfer = data;

	surfer_close_tab(surfer, NULL);
}


/* on_console_clear */
void on_console_clear(gpointer data)
{
	Surfer * surfer = data;

	surfer_console_clear(surfer);
}


/* on_console_close */
void on_console_close(gpointer data)
{
	Surfer * surfer = data;

	surfer_show_console(surfer, FALSE);
}


/* on_console_closex */
gboolean on_console_closex(gpointer data)
{
	Surfer * surfer = data;

	surfer_show_console(surfer, FALSE);
	return TRUE;
}


/* on_console_execute */
void on_console_execute(gpointer data)
{
	Surfer * surfer = data;

	surfer_console_execute(surfer);
}


/* on_forward */
void on_forward(gpointer data)
{
	Surfer * surfer = data;

	surfer_go_forward(surfer);
}


/* on_view_fullscreen */
void on_view_fullscreen(gpointer data)
{
	Surfer * surfer = data;

	on_fullscreen(surfer);
}


/* on_fullscreen */
void on_fullscreen(gpointer data)
{
	Surfer * surfer = data;
	GdkWindow * window;

#if GTK_CHECK_VERSION(2, 14, 0)
	window = gtk_widget_get_window(surfer->window);
#else
	window = surfer->window->window;
#endif
	if((gdk_window_get_state(window) & GDK_WINDOW_STATE_FULLSCREEN)
			!= GDK_WINDOW_STATE_FULLSCREEN)
	{
#ifndef EMBEDDED
		gtk_widget_hide(surfer->menubar);
#endif
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(
					surfer->tb_fullscreen), TRUE);
		surfer_set_fullscreen(surfer, TRUE);
	}
	else
	{
#ifndef EMBEDDED
		gtk_widget_show(surfer->menubar);
#endif
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(
					surfer->tb_fullscreen), FALSE);
		surfer_set_fullscreen(surfer, FALSE);
	}
}


/* on_home */
void on_home(gpointer data)
{
	Surfer * surfer = data;

	surfer_go_home(surfer);
}


/* on_new_tab */
void on_new_tab(gpointer data)
{
	Surfer * surfer = data;

	surfer_open_tab(surfer, NULL);
}


/* on_new_window */
void on_new_window(gpointer data)
{
	Surfer * surfer = data;

	surfer_new_copy(surfer);
}


/* on_normal_size */
void on_normal_size(gpointer data)
{
	Surfer * surfer = data;

	surfer_zoom_reset(surfer);
}


/* on_notebook_close_tab */
void on_notebook_close_tab(GtkWidget * widget, gpointer data)
{
	Surfer * surfer = data;

	if((widget = g_object_get_data(G_OBJECT(widget), "widget")) == NULL)
		return; /* XXX report error */
	surfer_close_tab(surfer, widget);
}


/* on_notebook_switch_page */
static gboolean _switch_page_idle(gpointer data);

void on_notebook_switch_page(gpointer data)
{
	Surfer * surfer = data;

	g_idle_add(_switch_page_idle, surfer);
}

static gboolean _switch_page_idle(gpointer data)
{
	Surfer * surfer = data;
	gint n;
	GtkWidget * ghtml;

	if((n = gtk_notebook_get_current_page(GTK_NOTEBOOK(surfer->notebook)))
			< 0)
		return FALSE;
	ghtml = gtk_notebook_get_nth_page(GTK_NOTEBOOK(surfer->notebook), n);
	/* FIXME implement:
	 * - change the title (tab)
	 * - update toolbar buttons */
	/* XXX the Surfer fetches the right values by himself => API change? */
	surfer_set_location(surfer, NULL);
	surfer_set_progress(surfer, 0.0);
	surfer_set_security(surfer, SS_NONE);
	surfer_set_status(surfer, NULL);
	surfer_set_title(surfer, NULL);
	return FALSE;
}


/* on_path_activate */
void on_path_activate(gpointer data)
{
	Surfer * surfer = data;
	GtkWidget * entry;
	const gchar * url;

	entry = gtk_bin_get_child(GTK_BIN(surfer->lb_path));
	url = gtk_entry_get_text(GTK_ENTRY(entry));
	surfer_open(surfer, url);
}


/* on_preferences */
void on_preferences(gpointer data)
{
	Surfer * surfer = data;

	surfer_view_preferences(surfer);
}


/* on_refresh */
void on_refresh(gpointer data)
{
	Surfer * surfer = data;

	surfer_refresh(surfer);
}


/* on_security */
void on_security(gpointer data)
{
	Surfer * surfer = data;

	surfer_view_security(surfer);
}


/* on_stop */
void on_stop(gpointer data)
{
	Surfer * surfer = data;

	surfer_stop(surfer);
}


/* on_zoom_in */
void on_zoom_in(gpointer data)
{
	Surfer * surfer = data;

	surfer_zoom_in(surfer);
}


/* on_zoom_out */
void on_zoom_out(gpointer data)
{
	Surfer * surfer = data;

	surfer_zoom_out(surfer);
}
