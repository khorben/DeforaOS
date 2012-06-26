/* $Id$ */
static char const _copyright[] =
"Copyright (c) 2012 Pierre Pronchery <khorben@defora.org>";
/* This file is part of DeforaOS Desktop Surfer */
static char const _license[] =
"This program is free software: you can redistribute it and/or modify\n"
"it under the terms of the GNU General Public License as published by\n"
"the Free Software Foundation, version 3 of the License.\n"
"\n"
"This program is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"GNU General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU General Public License\n"
"along with this program.  If not, see <http://www.gnu.org/licenses/>.";



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <locale.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <Desktop.h>
#include "ghtml.h"
#include <System.h>
#include "../config.h"
#define _(string) gettext(string)
#define N_(string) string

/* constants */
#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif
#ifndef DATADIR
# define DATADIR	PREFIX "/share"
#endif
#ifndef LOCALEDIR
# define LOCALEDIR	DATADIR "/locale"
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
	GtkWidget * ab_window;
};


/* prototypes */
static Helper * _helper_new(void);
void _helper_delete(Helper * helper);

static int _helper_open(Helper * helper, char const * url);
static int _helper_open_contents(Helper * helper, char const * package,
		char const * command);
static int _helper_open_dialog(Helper * helper);
static int _helper_open_man(Helper * helper, int section, char const * page);
static int _helper_open_devel(Helper * helper, char const * package);

static int _usage(void);

/* callbacks */
static gboolean _helper_on_close(gpointer data);
static gboolean _helper_on_closex(gpointer data);
#ifndef EMBEDDED
static void _helper_on_file_close(gpointer data);
static void _helper_on_file_open(gpointer data);
#endif
static void _helper_on_fullscreen(gpointer data);
#ifndef EMBEDDED
static void _helper_on_help_about(gpointer data);
#endif
#ifdef EMBEDDED
static void _helper_on_open(gpointer data);
#endif
#ifndef EMBEDDED
static void _helper_on_view_fullscreen(gpointer data);
#endif


/* constants */
#ifndef EMBEDDED
static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};
#endif

#ifdef EMBEDDED
static const DesktopAccel _helper_accel[] =
{
	{ G_CALLBACK(_helper_on_close), GDK_CONTROL_MASK, GDK_KEY_W },
	{ G_CALLBACK(_helper_on_open), GDK_CONTROL_MASK, GDK_KEY_O },
	{ NULL, 0, 0 }
};
#endif

#ifndef EMBEDDED
static const DesktopMenu _menu_file[] =
{
	{ N_("_Open..."), G_CALLBACK(_helper_on_file_open), GTK_STOCK_OPEN,
		GDK_CONTROL_MASK, GDK_KEY_O },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Close"), G_CALLBACK(_helper_on_file_close), GTK_STOCK_CLOSE,
		GDK_CONTROL_MASK, GDK_KEY_W },
	{ NULL, NULL, NULL, 0, 0 }
};

static const DesktopMenu _menu_view[] =
{
	{ N_("_Fullscreen"), G_CALLBACK(_helper_on_view_fullscreen),
# if GTK_CHECK_VERSION(2, 8, 0)
		GTK_STOCK_FULLSCREEN, 0, GDK_KEY_F11 },
# else
		NULL, 0, GDK_KEY_F11 },
# endif
	{ NULL, NULL, NULL, 0, 0 }
};

static const DesktopMenu _menu_help[] =
{
	{ N_("_About"), G_CALLBACK(_helper_on_help_about),
# if GTK_CHECK_VERSION(2, 6, 0)
		GTK_STOCK_ABOUT, 0, 0 },
# else
		NULL, 0, 0 },
# endif
	{ NULL, NULL, NULL, 0, 0 }
};

static const DesktopMenubar _helper_menubar[] =
{
	{ N_("_File"), _menu_file },
	{ N_("_View"), _menu_view },
	{ N_("_Help"), _menu_help },
	{ NULL, NULL }
};
#endif


/* functions */
/* helper */
static Helper * _helper_new(void)
{
	Helper * helper;
	GtkAccelGroup * group;
	GtkWidget * vbox;
	GtkToolItem * toolitem;
	GtkWidget * widget;

	if((helper = object_new(sizeof(*helper))) == NULL)
		return NULL;
	/* window */
	group = gtk_accel_group_new();
	helper->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_add_accel_group(GTK_WINDOW(helper->window), group);
	gtk_window_set_default_size(GTK_WINDOW(helper->window), 800, 600);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(helper->window), "help-browser");
#endif
	gtk_window_set_title(GTK_WINDOW(helper->window), _("Help browser"));
	g_signal_connect_swapped(helper->window, "delete-event", G_CALLBACK(
				_helper_on_closex), helper);
	vbox = gtk_vbox_new(FALSE, 0);
#ifndef EMBEDDED
	/* menubar */
	helper->menubar = desktop_menubar_create(_helper_menubar, helper,
			group);
	gtk_box_pack_start(GTK_BOX(vbox), helper->menubar, FALSE, TRUE, 0);
#else
	desktop_accel_create(_helper_accel, helper, group);
#endif
	/* toolbar */
	widget = gtk_toolbar_new();
#ifdef EMBEDDED
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_OPEN);
	g_signal_connect_swapped(toolitem, "clicked", G_CALLBACK(
				_helper_on_open), helper);
	gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
#endif
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
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__, buf);
#endif
	gtk_box_pack_start(GTK_BOX(vbox), helper->view, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(helper->window), vbox);
	gtk_widget_grab_focus(helper->view);
	gtk_widget_show_all(helper->window);
	helper->ab_window = NULL;
	return helper;
}


/* helper_delete */
void _helper_delete(Helper * helper)
{
	gtk_widget_destroy(helper->window);
	object_delete(helper);
}


/* useful */
/* helper_open */
static int _helper_open(Helper * helper, char const * url)
{
	if(url == NULL)
		return _helper_open_dialog(helper);
	ghtml_load_url(helper->view, url);
	return 0;
}


/* helper_open_contents */
static int _helper_open_contents(Helper * helper, char const * package,
		char const * command)
{
	char buf[256];

	if(package == NULL)
		return -1;
	if(command == NULL)
		command = "index";
	/* read a package documentation */
	snprintf(buf, sizeof(buf), "%s%s%s%s%s", "file://" DATADIR "/doc/html/",
			package, "/", command, ".html");
	return _helper_open(helper, buf);
}


/* helper_open_devel */
static int _helper_open_devel(Helper * helper, char const * package)
{
	char buf[256];

	/* read a package API documentation */
	snprintf(buf, sizeof(buf), "%s%s%s", "file://" DATADIR "/gtk-doc/html/",
			package, "/index.html");
	return _helper_open(helper, buf);
}


/* helper_open_dialog */
static void _open_dialog_activated(gpointer data);

static int _helper_open_dialog(Helper * helper)
{
	int ret;
	GtkWidget * dialog;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * label;
	GtkWidget * entry;
	char * page = NULL;

	dialog = gtk_dialog_new_with_buttons(_("Open page..."),
			GTK_WINDOW(helper->window),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL);
#if GTK_CHECK_VERSION(2, 14, 0)
	vbox = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
#else
	vbox = GTK_DIALOG(dialog)->vbox;
#endif
	hbox = gtk_hbox_new(FALSE, 4);
	label = gtk_label_new("Package: ");
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 0);
	entry = gtk_entry_new();
	g_signal_connect_swapped(entry, "activate", G_CALLBACK(
				_open_dialog_activated), dialog);
	gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
	gtk_widget_show_all(vbox);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK)
		page = strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
	gtk_widget_destroy(dialog);
	if(page == NULL || strlen(page) == 0)
		ret = -1;
	else
		ret = _helper_open_contents(helper, page, NULL);
	free(page);
	return ret;
}

static void _open_dialog_activated(gpointer data)
{
	GtkWidget * dialog = data;

	gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
}


/* helper_open_man */
static int _helper_open_man(Helper * helper, int section, char const * page)
{
	char buf[256];

	if(section <= 0 || section >= 10)
		return -1;
	/* read a manual page */
	snprintf(buf, sizeof(buf), "%s%d%s%s%s", "file://" DATADIR "/man/html",
			section, "/", page, ".html");
	return _helper_open(helper, buf);
}


/* callbacks */
/* helper_on_close */
static gboolean _helper_on_close(gpointer data)
{
	Helper * helper = data;

	gtk_widget_hide(helper->window);
	gtk_main_quit();
}


/* helper_on_closex */
static gboolean _helper_on_closex(gpointer data)
{
	Helper * helper = data;

	_helper_on_close(helper);
	return TRUE;
}


#ifndef EMBEDDED
/* helper_on_file_close */
static void _helper_on_file_close(gpointer data)
{
	Helper * helper = data;

	gtk_widget_hide(helper->window);
	gtk_main_quit();
}


/* helper_on_file_open */
static void _helper_on_file_open(gpointer data)
{
	Helper * helper = data;

	_helper_open_dialog(helper);
}
#endif


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


#ifndef EMBEDDED
/* helper_on_help_about */
static gboolean _about_on_closex(gpointer data);

static void _helper_on_help_about(gpointer data)
{
	Helper * helper = data;

	if(helper->ab_window != NULL)
	{
		gtk_window_present(GTK_WINDOW(helper->ab_window));
		return;
	}
	helper->ab_window = desktop_about_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(helper->ab_window), GTK_WINDOW(
				helper->window));
	desktop_about_dialog_set_authors(helper->ab_window, _authors);
	desktop_about_dialog_set_comments(helper->ab_window,
			_("Online help for the DeforaOS desktop"));
	desktop_about_dialog_set_copyright(helper->ab_window, _copyright);
	desktop_about_dialog_set_logo_icon_name(helper->ab_window,
			"help-browser");
	desktop_about_dialog_set_license(helper->ab_window, _license);
	desktop_about_dialog_set_name(helper->ab_window, PACKAGE);
	desktop_about_dialog_set_version(helper->ab_window, VERSION);
	g_signal_connect_swapped(G_OBJECT(helper->ab_window), "delete-event",
			G_CALLBACK(_about_on_closex), helper);
	gtk_widget_show(helper->ab_window);
}

static gboolean _about_on_closex(gpointer data)
{
	Helper * helper = data;

	gtk_widget_hide(helper->ab_window);
	return TRUE;
}
#endif


#ifdef EMBEDDED
/* helper_on_open */
static void _helper_on_open(gpointer data)
{
	Helper * helper = data;

	_helper_open_dialog(helper);
}
#endif


#ifndef EMBEDDED
/* helper_on_view_fullscreen */
static void _helper_on_view_fullscreen(gpointer data)
{
	Helper * helper = data;

	_helper_on_fullscreen(helper);
}
#endif


/* usage */
static int _usage(void)
{
	fputs(_("Usage: helper [-c][-p package] command\n"
"       helper -d package\n"
"       helper -s section page\n"
"  -s	Section of the manual page to read from\n"), stderr);
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

	snprintf(buf, sizeof(buf), "%s%s%s", _("Help browser"),
			(title != NULL) ? " - " : "",
			(title != NULL) ? title : "");
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
			"%s", _("Question"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
#endif
			"%s", message);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Question"));
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
			"%s", _("Error"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
#endif
			"%s", (message != NULL) ? message : _("Unknown error"));
	gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
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
			"%s", _("Question"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
#endif
			"%s", message);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Question"));
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
	int devel = 0;
	char const * package = NULL;
	int section = -1;
	char * p;
	Helper * helper;

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "cdp:s:")) != -1)
		switch(o)
		{
			case 'c':
				section = -1;
				devel = 0;
				break;
			case 'd':
				section = -1;
				devel = 1;
				break;
			case 'p':
				package = optarg;
				break;
			case 's':
				section = strtol(optarg, &p, 10);
				if(optarg[0] == '\0' || *p != '\0'
						|| section < 0 || section > 9)
					return _usage();
				break;
			default:
				return _usage();
		}
	if(optind != argc && (optind + 1) != argc)
		return _usage();
	if((helper = _helper_new()) == NULL)
		return 2;
	if(section > 0)
		_helper_open_man(helper, section, argv[optind]);
	else if(argv[optind] != NULL && devel != 0)
		_helper_open_devel(helper, argv[optind]);
	else if(argv[optind] != NULL)
		_helper_open_contents(helper, (package != NULL) ? package
				: argv[optind], argv[optind]);
	else
		_helper_open_dialog(helper);
	gtk_main();
	_helper_delete(helper);
	return 0;
}
