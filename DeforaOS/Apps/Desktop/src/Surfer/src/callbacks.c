/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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
/* on_edit_find */
void on_edit_find(gpointer data)
{
	Surfer * surfer = data;

	surfer_find(surfer, NULL);
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

	on_closex(surfer);
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
static void _preferences_set(Surfer * surfer);
/* callbacks */
static gboolean _preferences_on_closex(gpointer data);
static void _preferences_on_response(GtkWidget * widget, gint response,
		gpointer data);
static void _preferences_on_cancel(gpointer data);
static void _preferences_on_ok(gpointer data);
static void _preferences_on_proxy_http_toggled(gpointer data);

void on_preferences(gpointer data)
{
	Surfer * surfer = data;
	GtkWidget * vbox;
	GtkWidget * widget;
	GtkWidget * notebook;
	GtkWidget * page;
	GtkWidget * hbox;

	if(surfer->pr_window != NULL)
	{
		gtk_widget_show(surfer->pr_window);
		return;
	}
	surfer->pr_window = gtk_dialog_new_with_buttons(
			_("Web surfer preferences"), GTK_WINDOW(surfer->window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
	g_signal_connect_swapped(G_OBJECT(surfer->pr_window), "delete-event",
			G_CALLBACK(_preferences_on_closex), surfer);
	g_signal_connect(G_OBJECT(surfer->pr_window), "response",
			G_CALLBACK(_preferences_on_response), surfer);
#if GTK_CHECK_VERSION(2, 14, 0)
	vbox = gtk_dialog_get_content_area(GTK_DIALOG(surfer->pr_window));
#else
	vbox = GTK_DIALOG(surfer->pr_window)->vbox;
#endif
	/* notebook */
	notebook = gtk_notebook_new();
	/* general tab */
	page = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(page), 4);
	/* homepage */
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new(_("Homepage:"));
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	surfer->pr_homepage = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), surfer->pr_homepage, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(page), hbox, FALSE, TRUE, 0);
	/* focus new tabs */
	hbox = gtk_hbox_new(FALSE, 4);
	surfer->pr_focus_tabs = gtk_check_button_new_with_label(
			_("Focus new tabs"));
	gtk_box_pack_start(GTK_BOX(hbox), surfer->pr_focus_tabs, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(page), hbox, FALSE, TRUE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page,
			gtk_label_new(_("General")));
	/* network tab */
	page = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(page), 4);
	widget = gtk_radio_button_new_with_label(NULL, _("Direct connection"));
	surfer->pr_proxy_radio_direct = widget;
	gtk_box_pack_start(GTK_BOX(page), widget, FALSE, TRUE, 0);
	widget = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(
				widget), _("HTTP proxy:"));
	surfer->pr_proxy_radio_http = widget;
	g_signal_connect_swapped(G_OBJECT(widget), "toggled", G_CALLBACK(
				_preferences_on_proxy_http_toggled), surfer);
	gtk_box_pack_start(GTK_BOX(page), widget, FALSE, TRUE, 0);
	/* http proxy */
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new(_("Hostname:"));
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	surfer->pr_proxy_http = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), surfer->pr_proxy_http, TRUE, TRUE, 0);
	widget = gtk_label_new(_("Port:"));
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	surfer->pr_proxy_http_port = gtk_spin_button_new_with_range(0, 65535,
			1.0);
	gtk_box_pack_start(GTK_BOX(hbox), surfer->pr_proxy_http_port, FALSE,
			TRUE, 0);
	gtk_box_pack_start(GTK_BOX(page), hbox, FALSE, TRUE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page,
			gtk_label_new(_("Network")));
	gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);
	_preferences_set(surfer);
	gtk_widget_show_all(surfer->pr_window);
}

static void _preferences_set(Surfer * surfer)
{
	char const * p;
	unsigned long port;
	char * q = NULL;

	gtk_entry_set_text(GTK_ENTRY(surfer->pr_homepage), surfer->homepage
			!= NULL ? surfer->homepage : "");
	if((p = config_get(surfer->config, "", "focus_new_tabs")) != NULL
			&& strcmp(p, "1") == 0)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					surfer->pr_focus_tabs), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					surfer->pr_focus_tabs), FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					surfer->pr_proxy_radio_http),
			surfer->proxy_type == SPT_HTTP);
	_preferences_on_proxy_http_toggled(surfer);
	if((p = config_get(surfer->config, "proxy", "http")) != NULL)
		gtk_entry_set_text(GTK_ENTRY(surfer->pr_proxy_http), p);
	if((p = config_get(surfer->config, "proxy", "http_port")) != NULL
			&& p[0] != '\0')
	{
		port = strtoul(p, &q, 10);
		if(q != NULL && *q == '\0')
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(
						surfer->pr_proxy_http_port),
					port);
	}
}

static gboolean _preferences_on_closex(gpointer data)
{
	Surfer * surfer = data;

	_preferences_on_cancel(surfer);
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
	Surfer * surfer = data;

	gtk_widget_hide(surfer->pr_window);
	_preferences_set(surfer);
}

static void _preferences_on_ok(gpointer data)
{
	Surfer * surfer = data;
	SurferProxyType type = SPT_NONE;

	gtk_widget_hide(surfer->pr_window);
	surfer_set_homepage(surfer, gtk_entry_get_text(GTK_ENTRY(
					surfer->pr_homepage)));
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
					surfer->pr_proxy_radio_http)))
		type = SPT_HTTP;
	surfer_set_proxy(surfer, type, gtk_entry_get_text(GTK_ENTRY(
					surfer->pr_proxy_http)),
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(
					surfer->pr_proxy_http_port)));
	surfer_config_save(surfer);
}

static void _preferences_on_proxy_http_toggled(gpointer data)
{
	Surfer * surfer = data;
	gboolean active;

	active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
				surfer->pr_proxy_radio_http));
	gtk_widget_set_sensitive(surfer->pr_proxy_http, active);
	gtk_widget_set_sensitive(surfer->pr_proxy_http_port, active);
}


/* on_refresh */
void on_refresh(gpointer data)
{
	Surfer * surfer = data;

	surfer_refresh(surfer);
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
