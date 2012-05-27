/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
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
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <Desktop.h>
#include "ghtml.h"
#include <System.h>
#include "../config.h"

/* constants */
#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif
#ifndef DATADIR
# define DATADIR	PREFIX "/share"
#endif


/* helper */
/* private */
/* types */
typedef struct _Surfer Helper;

struct _Surfer
{
	/* widgets */
	GtkWidget * window;
	GtkWidget * menubar;
	GtkWidget * view;
};


/* prototypes */
static int _helper(int section, char const * name);
/* callbacks */
static gboolean _helper_on_closex(void);
static void _helper_on_file_close(void);
static void _helper_on_fullscreen(gpointer data);
static void _helper_on_help_about(gpointer data);
static void _helper_on_view_fullscreen(gpointer data);


/* constants */
static const DesktopMenu _menu_file[] =
{
	{ "Close", G_CALLBACK(_helper_on_file_close), GTK_STOCK_CLOSE,
		GDK_CONTROL_MASK, GDK_KEY_W },
	{ NULL, NULL, NULL, 0, 0 }
};

static const DesktopMenu _menu_view[] =
{
	{ "Fullscreen", G_CALLBACK(_helper_on_view_fullscreen),
		GTK_STOCK_FULLSCREEN, 0, GDK_KEY_F11 },
	{ NULL, NULL, NULL, 0, 0 }
};

static const DesktopMenu _menu_help[] =
{
	{ "About", G_CALLBACK(_helper_on_help_about),
#if GTK_CHECK_VERSION(2, 6, 0)
		GTK_STOCK_ABOUT, 0, 0 },
#else
		NULL, 0, 0 },
#endif
	{ NULL, NULL, NULL, 0, 0 }
};

static const DesktopMenubar _helper_menubar[] =
{
	{ "File", _menu_file },
	{ "View", _menu_view },
	{ "Help", _menu_help },
	{ NULL, NULL }
};


/* functions */
/* helper */
static int _helper(int section, char const * name)
{
	Helper * helper;
	GtkAccelGroup * group;
	GtkWidget * vbox;
	GtkToolItem * toolitem;
	GtkWidget * widget;
	char buf[256];

	if((helper = object_new(sizeof(*helper))) == NULL)
		return -1;
	/* window */
	group = gtk_accel_group_new();
	helper->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_add_accel_group(GTK_WINDOW(helper->window), group);
	gtk_window_set_default_size(GTK_WINDOW(helper->window), 800, 600);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(helper->window), "help-browser");
#endif
	snprintf(buf, sizeof(buf), "%s - %s", "Helper", name);
	gtk_window_set_title(GTK_WINDOW(helper->window), buf);
	g_signal_connect_swapped(helper->window, "delete-event", G_CALLBACK(
				_helper_on_closex), NULL);
	vbox = gtk_vbox_new(FALSE, 0);
	/* menubar */
	helper->menubar = desktop_menubar_create(_helper_menubar, helper,
			group);
	gtk_box_pack_start(GTK_BOX(vbox), helper->menubar, FALSE, TRUE, 0);
	/* toolbar */
	widget = gtk_toolbar_new();
#if GTK_CHECK_VERSION(2, 8, 0)
	toolitem = gtk_toggle_tool_button_new_from_stock(GTK_STOCK_FULLSCREEN);
#else
	toolitem = gtk_toggle_tool_button_new_from_stock(GTK_STOCK_ZOOM_FIT);
#endif
	g_signal_connect_swapped(toolitem, "toggled", G_CALLBACK(
				_helper_on_fullscreen), helper);
	gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	/* view */
	helper->view = ghtml_new(helper);
	ghtml_set_enable_javascript(helper->view, FALSE);
	if(section > 0 && section < 10)
		/* read a manual page */
		snprintf(buf, sizeof(buf), "%s%d%s%s%s",
				"file://" DATADIR "/man/html", section, "/",
				name, ".html");
	else
		/* read a package document */
		snprintf(buf, sizeof(buf), "%s%s%s%s%s",
				"file://" DATADIR "/doc/html/", name, "/", name,
				".html");
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__, buf);
#endif
	ghtml_load_url(helper->view, buf);
	gtk_box_pack_start(GTK_BOX(vbox), helper->view, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(helper->window), vbox);
	gtk_widget_grab_focus(helper->view);
	gtk_widget_show_all(helper->window);
	gtk_main();
	/* delete everything */
	object_delete(helper);
	return 0;
}


/* callbacks */
/* helper_on_closex */
static gboolean _helper_on_closex(void)
{
	gtk_main_quit();
	return FALSE;
}


/* helper_on_file_close */
static void _helper_on_file_close(void)
{
	gtk_main_quit();
}


/* helper_on_fullscreen */
static void _helper_on_fullscreen(gpointer data)
{
	Helper * helper = data;
	GdkWindow * window;
	gboolean fullscreen;

#if GTK_CHECK_VERSION(2, 14, 0)
	window = gtk_widget_get_window(helper->window);
#else
	window = helper->window->window;
#endif
	fullscreen = (gdk_window_get_state(window)
			& GDK_WINDOW_STATE_FULLSCREEN)
		== GDK_WINDOW_STATE_FULLSCREEN;
	surfer_set_fullscreen(helper, !fullscreen);
}


/* helper_on_help_about */
static void _helper_on_help_about(gpointer data)
{
	/* FIXME implement */
}


/* helper_on_view_fullscreen */
static void _helper_on_view_fullscreen(gpointer data)
{
	Helper * helper = data;

	_helper_on_fullscreen(helper);
}


/* usage */
static int _usage(void)
{
	fputs("Usage: helper package\n"
"       helper -s section page\n"
"  -s	Section of the manual page\n", stderr);
	return 1;
}


/* public */
/* surfer */
/* essential */
/* surfer_new */
Surfer * surfer_new(char const * url)
{
	return NULL;
}


/* surfer_delete */
void surfer_delete(Surfer * surfer)
{
}


/* accessors */
/* surfer_get_view */
GtkWidget * surfer_get_view(Surfer * surfer)
{
	/* FIXME remove from the API? */
	return NULL;
}


/* surfer_set_fullscreen */
void surfer_set_fullscreen(Surfer * surfer, gboolean fullscreen)
{
	Helper * helper = surfer;

	if(fullscreen)
	{
		gtk_widget_hide(helper->menubar);
		gtk_window_fullscreen(GTK_WINDOW(helper->window));
	}
	else
	{
		gtk_widget_show(helper->menubar);
		gtk_window_unfullscreen(GTK_WINDOW(helper->window));
	}
}


/* surfer_set_location */
void surfer_set_location(Surfer * surfer, char const * location)
{
	/* FIXME implement? */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, location);
#endif
}


/* surfer_set_progress */
void surfer_set_progress(Surfer * surfer, gdouble fraction)
{
	/* FIXME implement? */
}


/* surfer_set_security */
void surfer_set_security(Surfer * surfer, SurferSecurity security)
{
	/* FIXME implement? */
}


/* surfer_set_status */
void surfer_set_status(Surfer * surfer, char const * status)
{
	/* FIXME really implement? */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, status);
#endif
}


/* surfer_set_title */
void surfer_set_title(Surfer * surfer, char const * title)
{
	Helper * helper = surfer;
	char buf[256];

	snprintf(buf, sizeof(buf), "%s%s%s", "Helper", (title != NULL)
			? " - " : "", (title != NULL) ? title : "");
	gtk_window_set_title(GTK_WINDOW(helper->window), buf);
}


/* useful */
/* surfer_confirm */
int surfer_confirm(Surfer * surfer, char const * message, gboolean * confirmed)
{
	Helper * helper = surfer;
	int ret = 0;
	GtkWidget * dialog;
	int res;

	dialog = gtk_message_dialog_new((helper != NULL)
			? GTK_WINDOW(helper->window) : NULL,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
#if GTK_CHECK_VERSION(2, 6, 0)
			"%s", "Question");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
#endif
			"%s", message);
	gtk_window_set_title(GTK_WINDOW(dialog), "Question");
	res = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	if(res == GTK_RESPONSE_YES)
		*confirmed = TRUE;
	else if(res == GTK_RESPONSE_NO)
		*confirmed = FALSE;
	else
		ret = 1;
	return ret;
}


/* surfer_console_message */
void surfer_console_message(Surfer * surfer, char const * message,
		char const * source, long line)
{
	/* FIXME really implement */
	fprintf(stderr, "%s: %s:%ld: %s\n", "helper", source, line, message);
}


/* surfer_download */
int surfer_download(Surfer * surfer, char const * url, char const * suggested)
{
	/* FIXME really implement */
	return 0;
}


/* surfer_error */
int surfer_error(Surfer * surfer, char const * message, int ret)
{
	Helper * helper = surfer;
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new((helper != NULL)
			? GTK_WINDOW(helper->window) : NULL,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
#if GTK_CHECK_VERSION(2, 6, 0)
			"%s", "Error");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
#endif
			"%s", (message != NULL) ? message : "Unknown error");
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return ret;
}


/* surfer_prompt */
int surfer_prompt(Surfer * surfer, char const * message,
		char const * default_value, char ** value)
{
	Helper * helper = surfer;
	int ret = 0;
	GtkWidget * dialog;
	GtkWidget * vbox;
	GtkWidget * entry;
	int res;

	dialog = gtk_message_dialog_new((helper != NULL)
			? GTK_WINDOW(helper->window) : NULL,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL,
#if GTK_CHECK_VERSION(2, 6, 0)
			"%s", "Question");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
#endif
			"%s", message);
	gtk_window_set_title(GTK_WINDOW(dialog), "Question");
#if GTK_CHECK_VERSION(2, 14, 0)
	vbox = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
#else
	vbox = GTK_DIALOG(dialog)->vbox;
#endif
	entry = gtk_entry_new();
	if(default_value != NULL)
		gtk_entry_set_text(GTK_ENTRY(entry), default_value);
	gtk_widget_show(entry);
	gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, TRUE, 0);
	if((res = gtk_dialog_run(GTK_DIALOG(dialog))) == GTK_RESPONSE_OK)
		*value = strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
	if(res != GTK_RESPONSE_OK || value == NULL)
		ret = 1;
	gtk_widget_destroy(dialog);
	return ret;
}


/* surfer_resize */
void surfer_resize(Surfer * surfer, gint width, gint height)
{
	Helper * helper = surfer;

	gtk_window_resize(GTK_WINDOW(helper->window), width, height);
}


/* surfer_show_console */
void surfer_show_console(Surfer * surfer, gboolean show)
{
}


/* surfer_show_location */
void surfer_show_location(Surfer * surfer, gboolean show)
{
}


/* surfer_show_menubar */
void surfer_show_menubar(Surfer * surfer, gboolean show)
{
}


/* surfer_show_statusbar */
void surfer_show_statusbar(Surfer * surfer, gboolean show)
{
}


/* surfer_show_toolbar */
void surfer_show_toolbar(Surfer * surfer, gboolean show)
{
}


/* surfer_show_window */
void surfer_show_window(Surfer * surfer, gboolean show)
{
}


/* surfer_warning */
void surfer_warning(Surfer * surfer, char const * message)
{
	fprintf(stderr, "%s: %s\n", "helper", message);
}


/* helper */
/* main */
int main(int argc, char * argv[])
{
	int o;
	int section = -1;
	char * p;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "s:")) != -1)
		switch(o)
		{
			case 's':
				section = strtol(optarg, &p, 10);
				if(optarg[0] == '\0' || *p != '\0'
						|| section < 0 || section > 9)
					return _usage();
				break;
			default:
				return _usage();
		}
	if(optind + 1 != argc)
		return _usage();
	return (_helper(section, argv[optind]) == 0) ? 0 : 2;
}
