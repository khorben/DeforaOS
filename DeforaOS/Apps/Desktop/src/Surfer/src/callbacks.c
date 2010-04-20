/* $Id$ */
static char const _copyright[] =
"Copyright (c) 2010 Pierre Pronchery <khorben@defora.org>";
/* This file is part of DeforaOS Desktop Surfer */
static char const _license[] =
"Surfer is free software; you can redistribute it and/or modify it\n"
"under the terms of the GNU General Public License version 2 as\n"
"published by the Free Software Foundation.\n"
"\n"
"Surfer is distributed in the hope that it will be useful, but WITHOUT ANY\n"
"WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS\n"
"FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more\n"
"details.\n"
"\n"
"You should have received a copy of the GNU General Public License along\n"
"with Surfer; if not, write to the Free Software Foundation, Inc., 59 Temple\n"
"Place, Suite 330, Boston, MA  02111-1307  USA\n";



#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libintl.h>
#include "surfer.h"
#include "ghtml.h"
#include "callbacks.h"
#include "common.h"
#include "../config.h"
#define _(string) gettext(string)


/* constants */
static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};


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

	surfer_close_tab(surfer);
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
static gboolean _about_on_closex(gpointer data);
# if !GTK_CHECK_VERSION(2, 6, 0)
static void _about_on_close(GtkWidget * widget, gpointer data);
static void _about_on_credits(GtkWidget * widget, gpointer data);
static void _about_on_license(GtkWidget * widget, gpointer data);
# endif

void on_help_about(gpointer data)
{
	Surfer * surfer = data;
	static GtkWidget * window = NULL;
# if GTK_CHECK_VERSION(2, 6, 0)
	gsize cnt = 65536;
	gchar * buf;

	if(window != NULL)
	{
		gtk_widget_show(window);
		return;
	}
	if((buf = malloc(sizeof(*buf) * cnt)) == NULL)
	{
		surfer_error(surfer, "Memory allocation failed", 0);
		return;
	}
	window = gtk_about_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(
				surfer->window));
	gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(window), PACKAGE);
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(window), VERSION);
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(window), _authors);
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(window), _copyright);
	gtk_about_dialog_set_logo_icon_name(GTK_ABOUT_DIALOG(window),
			"stock_internet");
	if(g_file_get_contents("/usr/share/common-licenses/GPL-2", &buf, &cnt,
				NULL) == TRUE)
		gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(window), buf);
	else
		gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(window),
				_license);
	free(buf);
	g_signal_connect_swapped(G_OBJECT(window), "delete-event", G_CALLBACK(
				_about_on_closex), window);
	g_signal_connect(G_OBJECT(window), "response", G_CALLBACK(
				gtk_widget_hide), NULL);
	gtk_widget_show(window);
}
# else
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * button;

	if(window != NULL)
	{
		gtk_widget_show(window);
		return;
	}
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect_swapped(G_OBJECT(window), "delete-event", G_CALLBACK(
				_about_on_closex), window);
	gtk_container_set_border_width(GTK_CONTAINER(window), 4);
	gtk_window_set_title(GTK_WINDOW(window), _("About " PACKAGE));
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(
				surfer->window));
	vbox = gtk_vbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(PACKAGE " " VERSION),
			FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(_copyright), FALSE,
			FALSE, 2);
	hbox = gtk_hbox_new(TRUE, 4);
	button = gtk_button_new_with_mnemonic(_("C_redits"));
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(
				_about_on_credits), window);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, TRUE, 4);
	button = gtk_button_new_with_mnemonic(_("_License"));
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
# endif

static gboolean _about_on_closex(gpointer data)
{
	GtkWidget * widget = data;

	gtk_widget_hide(widget);
	return TRUE;
}

# if !GTK_CHECK_VERSION(2, 6, 0)
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
	gtk_window_set_title(GTK_WINDOW(window), _("Credits"));
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(about));
	g_signal_connect_swapped(G_OBJECT(window), "delete-event", G_CALLBACK(
				_about_on_closex), window);
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
			GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(widget), textview);
	notebook = gtk_notebook_new();
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget,
			gtk_label_new(_("Written by")));
	gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 4);
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	g_signal_connect(G_OBJECT(widget), "clicked",
			G_CALLBACK(_about_on_close), window);
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
	gtk_window_set_title(GTK_WINDOW(window), _("License"));
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(about));
	g_signal_connect_swapped(G_OBJECT(window), "delete-event", G_CALLBACK(
				_about_on_closex), window);
	vbox = gtk_vbox_new(FALSE, 0);
	textview = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), FALSE);
	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	gtk_text_buffer_set_text(tbuf, _license, strlen(_license));
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(widget),
			GTK_SHADOW_ETCHED_IN);
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
# endif /* !GTK_CHECK_VERSION(2, 6, 0) */
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

	window = gtk_widget_get_window(surfer->window);
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


/* on_notebook_close_tab */
void on_notebook_close_tab(gpointer data)
{
	Surfer * surfer = data;

	surfer_close_tab(surfer);
}


/* on_notebook_switch_page */
void on_notebook_switch_page(GtkWidget * widget, GtkNotebookPage * page,
		guint n, gpointer data)
{
	Surfer * surfer = data;
	GtkWidget * ghtml;

	ghtml = gtk_notebook_get_nth_page(GTK_NOTEBOOK(surfer->notebook), n);
	/* FIXME implement:
	 * - change the title (tab)
	 * - update toolbar buttons */
	surfer_set_location(surfer, ghtml_get_location(ghtml));
	surfer_set_progress(surfer, ghtml_get_progress(ghtml));
	surfer_set_status(surfer, ghtml_get_status(ghtml));
	surfer_set_title(surfer, ghtml_get_title(ghtml));
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
static void _preferences_on_cancel(gpointer data);
static void _preferences_on_ok(gpointer data);

void on_preferences(gpointer data)
{
	Surfer * surfer = data;
	PangoFontDescription * desc;
	GtkWidget * vbox;
	GtkWidget * widget;
	GtkWidget * notebook;
	GtkWidget * page;
	GtkWidget * hbox;
	GtkSizeGroup * group;

	if(surfer->pr_window != NULL)
	{
		gtk_widget_show(surfer->pr_window);
		return;
	}
	/* FIXME consider using gtk_dialog_new() */
	desc = pango_font_description_new();
	pango_font_description_set_weight(desc, PANGO_WEIGHT_BOLD);
	surfer->pr_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable(GTK_WINDOW(surfer->pr_window), FALSE);
	gtk_window_set_title(GTK_WINDOW(surfer->pr_window),
			_("Web surfer preferences"));
	gtk_window_set_transient_for(GTK_WINDOW(surfer->pr_window), GTK_WINDOW(
				surfer->window));
	g_signal_connect_swapped(G_OBJECT(surfer->pr_window), "delete-event",
			G_CALLBACK(_preferences_on_closex), surfer);
	vbox = gtk_vbox_new(FALSE, 0);
	/* notebook */
	notebook = gtk_notebook_new();
	/* general */
	page = gtk_vbox_new(FALSE, 0);
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new(_("Homepage:"));
	gtk_widget_modify_font(widget, desc);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	surfer->pr_homepage = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), surfer->pr_homepage, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(page), hbox, TRUE, TRUE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page,
			gtk_label_new(_("General")));
	gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);
	/* dialog */
	hbox = gtk_hbox_new(FALSE, 0);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	widget = gtk_button_new_from_stock(GTK_STOCK_OK);
	gtk_size_group_add_widget(group, widget);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				_preferences_on_ok), surfer);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_size_group_add_widget(group, widget);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				_preferences_on_cancel), surfer);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, TRUE, 4);
	/* separator */
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 4);
	gtk_box_pack_end(GTK_BOX(vbox), hbox, TRUE, TRUE, 4);
	_preferences_set(surfer);
	gtk_container_add(GTK_CONTAINER(surfer->pr_window), vbox);
	gtk_widget_show_all(surfer->pr_window);
	pango_font_description_free(desc);
}

static void _preferences_set(Surfer * surfer)
{
	gtk_entry_set_text(GTK_ENTRY(surfer->pr_homepage), surfer->homepage
			!= NULL ? surfer->homepage : "");
}

static gboolean _preferences_on_closex(gpointer data)
{
	Surfer * surfer = data;

	_preferences_on_cancel(surfer);
	return TRUE;
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

	gtk_widget_hide(surfer->pr_window);
	free(surfer->homepage);
	surfer->homepage = strdup(gtk_entry_get_text(GTK_ENTRY(
					surfer->pr_homepage)));
	surfer_config_save(surfer);
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
