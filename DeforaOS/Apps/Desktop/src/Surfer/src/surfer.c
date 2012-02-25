/* $Id$ */
static char const _copyright[] =
"Copyright (c) 2006-2012 Pierre Pronchery <khorben@defora.org>";
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
/* TODO:
 * - implement the favicon
 * - support multiple profiles
 * - implement "always ask" for the default download directory
 * - provide access to SSL information in embedded mode
 * - rework preferences handling:
 *   * no longer cache settings outside of Config *
 *   * use helpers to draw widgets and load/save/set preferences
 * - support separate backends via plug-ins (with own preferences)
 * - consider putting the location bar below the tabs
 * - consider using GtkSourceView to display the page source
 * - consider moving the find dialog above the status bar */



#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libintl.h>
#include <gdk/gdkkeysyms.h>
#include <Desktop.h>
#include "callbacks.h"
#include "ghtml.h"
#include "surfer.h"
#include "common.h"
#include "../config.h"
#define _(string) gettext(string)
#define N_(string) (string)

/* embed the Download class */
static unsigned int _surfer_cnt = 0;
#if defined(WITH_GTKHTML) || defined(WITH_GTKTEXTVIEW) /* uses GNet */
# define WITH_DOWNLOAD
#endif
#if defined(WITH_WEBKIT) /* WebKit 1.1.0 and above */
# include <webkit/webkit.h>
# if WEBKIT_CHECK_VERSION(1, 1, 0)
#  define WITH_DOWNLOAD
# endif
#endif
#ifdef WITH_DOWNLOAD
# define PROGNAME "surfer"
# define _download_cnt _surfer_cnt
# include "download.c"
#endif


/* Surfer */
/* private */
/* types */
typedef enum _SurferConsoleMessage
{
	SCM_MESSAGE = 0, SCM_SOURCE, SCM_LINE, SCM_DISPLAY_LINE
} SurferConsoleMessage;
#define SCM_LAST	SCM_DISPLAY_LINE
#define SCM_COUNT	(SCM_LAST + 1)

typedef struct _SurferIdle
{
	Surfer * surfer;
	char * url;
} SurferIdle;


/* constants */
static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};


/* variables */
#ifndef GDK_Back
# define GDK_Back	0
#endif
#ifndef GDK_Forward
# define GDK_Forward	0
#endif
static const DesktopAccel _surfer_accel[] =
{
	{ G_CALLBACK(on_close), GDK_CONTROL_MASK, GDK_KEY_W },
#ifdef EMBEDDED
	{ G_CALLBACK(on_refresh), GDK_CONTROL_MASK, GDK_KEY_R },
	{ G_CALLBACK(on_normal_size), GDK_CONTROL_MASK, GDK_KEY_0 },
	{ G_CALLBACK(on_zoom_in), GDK_CONTROL_MASK, GDK_KEY_plus },
	{ G_CALLBACK(on_zoom_out), GDK_CONTROL_MASK, GDK_KEY_minus },
#endif
	{ G_CALLBACK(on_back), 0, GDK_KEY_Back },
	{ G_CALLBACK(on_forward), 0, GDK_KEY_Forward },
	{ G_CALLBACK(on_fullscreen), 0, GDK_KEY_F11 },
	{ NULL, 0, 0 }
};

#ifndef EMBEDDED
static const DesktopMenu _menu_file[] =
{
	{ N_("_New tab"),	G_CALLBACK(on_file_new_tab), "tab-new",
		GDK_CONTROL_MASK, GDK_KEY_T },
	{ N_("_New window"),	G_CALLBACK(on_file_new_window), "window-new",
		GDK_CONTROL_MASK, GDK_KEY_N },
	{ N_("_Open..."),	G_CALLBACK(on_file_open), GTK_STOCK_OPEN,
		GDK_CONTROL_MASK, GDK_KEY_O },
	{ N_("Open _URL..."),	G_CALLBACK(on_file_open_url), NULL,
		GDK_CONTROL_MASK, GDK_KEY_L },
	{ "", NULL, NULL, 0, 0 },
	{ N_("Save _as..."),	G_CALLBACK(on_file_save_as), GTK_STOCK_SAVE_AS,
		GDK_CONTROL_MASK, GDK_KEY_S },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Print..."),	G_CALLBACK(on_file_print), GTK_STOCK_PRINT, 0,
		0 },
	{ "", NULL, NULL, 0, 0 },
	{ N_("Close _tab"),	G_CALLBACK(on_file_close_tab), NULL,
		GDK_CONTROL_MASK, GDK_KEY_W },
	{ N_("_Close"),		G_CALLBACK(on_file_close), GTK_STOCK_CLOSE, 0,
		0 },
	{ NULL,			NULL, NULL, 0, 0 }
};

static const DesktopMenu _menu_edit[] =
{
	{ N_("_Undo"),		G_CALLBACK(on_edit_undo), GTK_STOCK_UNDO,
		GDK_CONTROL_MASK, GDK_KEY_Z },
	{ N_("_Redo"),		G_CALLBACK(on_edit_redo), GTK_STOCK_REDO,
		GDK_CONTROL_MASK, GDK_KEY_Y },
	{ "",			NULL, NULL, 0, 0 },
	{ N_("_Cut"),		G_CALLBACK(on_edit_cut), GTK_STOCK_CUT,
		GDK_CONTROL_MASK, GDK_KEY_X },
	{ N_("Cop_y"),		G_CALLBACK(on_edit_copy), GTK_STOCK_COPY,
		GDK_CONTROL_MASK, GDK_KEY_C },
	{ N_("_Paste"),		G_CALLBACK(on_edit_paste), GTK_STOCK_PASTE,
		GDK_CONTROL_MASK, GDK_KEY_V },
	{ "",			NULL, NULL, 0, 0 },
	{ N_("Select _all"),	G_CALLBACK(on_edit_select_all),
#if GTK_CHECK_VERSION(2, 10, 0)
		GTK_STOCK_SELECT_ALL,
#else
		NULL,
#endif
		GDK_CONTROL_MASK, GDK_KEY_A },
	{ N_("_Unselect all"),	G_CALLBACK(on_edit_unselect_all), NULL, 0, 0 },
	{ "",			NULL, NULL, 0, 0 },
	{ N_("_Find"),		G_CALLBACK(on_edit_find), GTK_STOCK_FIND,
		GDK_CONTROL_MASK, GDK_KEY_F },
	{ "",			NULL, NULL, 0, 0 },
	{ N_("_Preferences"),	G_CALLBACK(on_edit_preferences),
		GTK_STOCK_PREFERENCES, GDK_CONTROL_MASK, GDK_KEY_P },
	{ NULL,			NULL, NULL, 0, 0 }
};

static const DesktopMenu _menu_view[] =
{
	{ N_("Zoom in"),	G_CALLBACK(on_view_zoom_in), "zoom-in",
		GDK_CONTROL_MASK, GDK_KEY_plus },
	{ N_("Zoom out"),	G_CALLBACK(on_view_zoom_out), "zoom-out",
		GDK_CONTROL_MASK, GDK_KEY_minus },
	{ N_("Normal size"),	G_CALLBACK(on_view_normal_size),
		"zoom-original", GDK_CONTROL_MASK, GDK_KEY_0 },
	{ "",			NULL, NULL, 0, 0 },
	{ N_("_Fullscreen"),	G_CALLBACK(on_view_fullscreen),
		GTK_STOCK_FULLSCREEN, 0, GDK_KEY_F11 },
	{ "",			NULL, NULL, 0, 0 },
	{ N_("_Refresh"),	G_CALLBACK(on_view_refresh), GTK_STOCK_REFRESH,
		GDK_CONTROL_MASK, GDK_KEY_R },
	{ N_("_Force refresh"),	G_CALLBACK(on_view_force_refresh), NULL,
		GDK_CONTROL_MASK | GDK_SHIFT_MASK, GDK_KEY_R },
	{ N_("_Stop"),		G_CALLBACK(on_view_stop), GTK_STOCK_STOP, 0,
		GDK_KEY_Escape },
	{ "",			NULL, NULL, 0, 0 },
	{ N_("Javascript _console"), G_CALLBACK(on_view_javascript_console),
		NULL, 0, 0 },
	{ N_("Page so_urce"),	G_CALLBACK(on_view_page_source),
		"surfer-view-html-source", GDK_CONTROL_MASK, GDK_KEY_U },
	{ NULL,			NULL, NULL, 0, 0 }
};

static const DesktopMenu _menu_help[] =
{
	{ N_("_About"),		G_CALLBACK(on_help_about),
#if GTK_CHECK_VERSION(2, 6, 0)
		GTK_STOCK_ABOUT, 0, 0 },
#else
		NULL, 0, 0 },
#endif
	{ NULL,			NULL, NULL, 0, 0 }
};

static const DesktopMenubar _surfer_menubar[] =
{
	{ N_("_File"), _menu_file },
	{ N_("_Edit"), _menu_edit },
	{ N_("_View"), _menu_view },
	{ N_("_Help"), _menu_help },
	{ NULL, NULL }
};
#endif /* !EMBEDDED */

static DesktopToolbar _surfer_toolbar[] =
{
	{ N_("Back"), G_CALLBACK(on_back), GTK_STOCK_GO_BACK, GDK_MOD1_MASK,
		GDK_KEY_Left, NULL },
	{ N_("Forward"), G_CALLBACK(on_forward), GTK_STOCK_GO_FORWARD,
		GDK_MOD1_MASK, GDK_KEY_Right, NULL },
	{ N_("Stop"), G_CALLBACK(on_stop), GTK_STOCK_STOP, 0, 0, NULL },
	{ N_("Refresh"), G_CALLBACK(on_refresh), GTK_STOCK_REFRESH, 0, 0,
		NULL },
	{ "", NULL, NULL, 0, 0, NULL },
	{ N_("Home"), G_CALLBACK(on_home), GTK_STOCK_HOME, GDK_MOD1_MASK,
		GDK_KEY_Home, NULL },
	{ "", NULL, NULL, 0, 0, NULL },
#ifdef EMBEDDED
	{ N_("Zoom in"), G_CALLBACK(on_zoom_in), "zoom-in", 0, 0, NULL },
	{ N_("Zoom out"), G_CALLBACK(on_zoom_out), "zoom-out", 0, 0, NULL },
	{ N_("Normal size"), G_CALLBACK(on_normal_size), "zoom-original", 0, 0,
		NULL },
	{ "", NULL, NULL, 0, 0, NULL },
	{ N_("New tab"), G_CALLBACK(on_new_tab), "tab-new", GDK_CONTROL_MASK,
		GDK_KEY_T, NULL },
	{ N_("New window"), G_CALLBACK(on_new_window), "window-new",
		GDK_CONTROL_MASK, GDK_KEY_N, NULL },
#endif
	{ NULL, NULL, NULL, 0, 0, NULL }
};


/* prototypes */
static gboolean _surfer_filename_confirm(Surfer * surfer,
		char const * filename);

static char * _config_get_filename(char const * filename);
static int _config_load_integer(Config * config, char const * section,
		char const * variable, unsigned int * value);
static int _config_load_string(Config * config, char const * section,
		char const * variable, char ** value);
static int _config_save_boolean(Config * config, char const * section,
		char const * variable, gboolean value);
static int _config_save_integer(Config * config, char const * section,
		char const * variable, unsigned int value);
static int _config_save_string(Config * config, char const * section,
		char const * variable, char const * value);


/* public */
/* functions */
/* surfer_new */
static Surfer * _new_do(char const * url);
static gboolean _new_idle(gpointer data);

Surfer * surfer_new(char const * url)
{
	Surfer * surfer;
	SurferIdle * si;

	if((surfer = _new_do(url)) == NULL)
		return NULL;
	gtk_widget_show(surfer->window);
	/* load url as soon as we're idle */
	if((si = malloc(sizeof(*si))) == NULL
			|| (url != NULL && (si->url = strdup(url)) == NULL))
	{
		free(si);
		surfer_delete(surfer);
		return NULL;
	}
	si->surfer = surfer;
	if(url == NULL)
		si->url = NULL;
	g_idle_add(_new_idle, si);
	return surfer;
}

Surfer * _new_do(char const * url)
{
	Surfer * surfer;
	GtkAccelGroup * group;
	GtkWidget * vbox;
	GtkWidget * toolbar;
	GtkToolItem * toolitem;
	GtkWidget * widget;

	if((surfer = object_new(sizeof(*surfer))) == NULL)
		return NULL;
	surfer->homepage = NULL;
	surfer->download_dir = NULL;
	surfer->download_close = 0;
	surfer->proxy_type = SPT_NONE;
	surfer->proxy_http = NULL;
	surfer->proxy_http_port = 0;
	surfer->user_agent = NULL;
	surfer->javascript = TRUE;
	if((surfer->config = config_new()) == NULL
			|| surfer_config_load(surfer) != 0)
	{
		surfer_delete(surfer);
		return NULL;
	}
	/* widgets */
	/* window */
	group = gtk_accel_group_new();
	surfer->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_add_accel_group(GTK_WINDOW(surfer->window), group);
	gtk_window_set_default_size(GTK_WINDOW(surfer->window), 1024, 768);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(surfer->window), "web-browser");
#endif
	gtk_window_set_title(GTK_WINDOW(surfer->window), _("Web surfer"));
	g_signal_connect_swapped(surfer->window, "delete-event", G_CALLBACK(
				on_closex), surfer);
	vbox = gtk_vbox_new(FALSE, 0);
#ifndef EMBEDDED
	/* menubar */
	surfer->menubar = desktop_menubar_create(_surfer_menubar, surfer,
			group);
	gtk_box_pack_start(GTK_BOX(vbox), surfer->menubar, FALSE, FALSE, 0);
#endif
	desktop_accel_create(_surfer_accel, surfer, group);
	/* toolbar */
	toolbar = desktop_toolbar_create(_surfer_toolbar, surfer, group);
	surfer->toolbar = toolbar;
	surfer->tb_back = _surfer_toolbar[0].widget;
	surfer->tb_forward = _surfer_toolbar[1].widget;
	surfer->tb_stop = _surfer_toolbar[2].widget;
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_stop), FALSE);
	surfer->tb_refresh = _surfer_toolbar[3].widget;
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_refresh), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_back), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_forward), FALSE);
#ifndef EMBEDDED
	/* toolbar: zoom value */
	toolitem = gtk_tool_item_new();
	surfer->tb_zoom = toolitem;
	widget = gtk_spin_button_new_with_range(0.0, 400.0, 1.0);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(widget), 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), 100.0);
	g_signal_connect_swapped(widget, "value-changed", G_CALLBACK(
				on_zoom_changed), surfer);
	gtk_container_add(GTK_CONTAINER(toolitem), widget);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
#endif
	/* toolbar: fullscreen button */
#if GTK_CHECK_VERSION(2, 8, 0)
	toolitem = gtk_toggle_tool_button_new_from_stock(GTK_STOCK_FULLSCREEN);
#else
	toolitem = gtk_toggle_tool_button_new_from_stock(GTK_STOCK_ZOOM_FIT);
#endif
	surfer->tb_fullscreen = toolitem;
	g_signal_connect_swapped(toolitem, "toggled", G_CALLBACK(
				on_fullscreen), surfer);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
#ifdef EMBEDDED
	toolitem = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_PREFERENCES);
	g_signal_connect_swapped(toolitem, "clicked", G_CALLBACK(
				on_preferences), surfer);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
#endif /* EMBEDDED */
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, TRUE, 0);
	/* toolbar */
	toolbar = gtk_toolbar_new();
	surfer->locationbar = toolbar;
	gtk_toolbar_set_icon_size(GTK_TOOLBAR(toolbar),
			GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
#ifndef EMBEDDED
	toolitem = gtk_tool_item_new();
	widget = gtk_label_new(_(" Location: "));
	gtk_container_add(GTK_CONTAINER(toolitem), widget);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
#endif
	toolitem = gtk_tool_item_new();
#if GTK_CHECK_VERSION(3, 0, 0)
	surfer->lb_path = gtk_combo_box_text_new_with_entry();
#else
	surfer->lb_path = gtk_combo_box_entry_new_text();
#endif
	widget = gtk_bin_get_child(GTK_BIN(surfer->lb_path));
	g_signal_connect_swapped(widget, "activate", G_CALLBACK(
				on_path_activate), surfer);
	if(url != NULL)
		gtk_entry_set_text(GTK_ENTRY(widget), url);
	gtk_tool_item_set_expand(toolitem, TRUE);
	gtk_container_add(GTK_CONTAINER(toolitem), surfer->lb_path);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_JUMP_TO);
	g_signal_connect_swapped(toolitem, "clicked", G_CALLBACK(
				on_path_activate), surfer);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
	/* notebook */
	surfer->notebook = gtk_notebook_new();
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(surfer->notebook), TRUE);
	gtk_notebook_set_show_border(GTK_NOTEBOOK(surfer->notebook), FALSE);
	surfer_open_tab(surfer, NULL);
	g_signal_connect_swapped(surfer->notebook, "switch-page", G_CALLBACK(
				on_notebook_switch_page), surfer);
	gtk_box_pack_start(GTK_BOX(vbox), surfer->notebook, TRUE, TRUE, 0);
	/* statusbar */
	surfer->statusbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(widget), GTK_RELIEF_NONE);
	surfer->security = gtk_image_new();
#if GTK_CHECK_VERSION(2, 12, 0)
	gtk_widget_set_tooltip_text(widget, _("Security information"));
#endif
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(on_security),
			surfer);
	gtk_container_add(GTK_CONTAINER(widget), surfer->security);
	gtk_box_pack_start(GTK_BOX(surfer->statusbox), widget, FALSE, TRUE, 0);
#if !GTK_CHECK_VERSION(2, 16, 0)
	surfer->progress = gtk_progress_bar_new();
	gtk_box_pack_start(GTK_BOX(surfer->statusbox), surfer->progress, FALSE,
			FALSE, 0);
#endif
	surfer->statusbar = gtk_statusbar_new();
	surfer->statusbar_id = 0;
	gtk_box_pack_start(GTK_BOX(surfer->statusbox), surfer->statusbar, TRUE,
			TRUE, 0);
	gtk_widget_show_all(surfer->statusbox);
	gtk_container_add(GTK_CONTAINER(surfer->window), vbox);
	gtk_widget_grab_focus(GTK_WIDGET(surfer->lb_path));
	gtk_widget_show_all(vbox);
	/* preferences window */
	surfer->pr_window = NULL;
	/* find dialog */
	surfer->fi_dialog = NULL;
	/* console window */
	surfer->co_window = NULL;
	surfer->co_store = gtk_list_store_new(SCM_COUNT, G_TYPE_STRING,
			G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING);
	/* about */
	surfer->ab_dialog = NULL;
	/* hack to display the statusbar only if necessary */
	gtk_box_pack_start(GTK_BOX(vbox), surfer->statusbox, FALSE, FALSE, 0);
	/* FIXME should be automatic and per tab */
	surfer_set_security(surfer, SS_NONE);
	surfer_set_status(surfer, NULL);
	_surfer_cnt++;
	return surfer;
}

static gboolean _new_idle(gpointer data)
{
	SurferIdle * si = data;

	surfer_open(si->surfer, si->url);
	free(si->url);
	free(si);
	return FALSE;
}


/* surfer_new_copy */
Surfer * surfer_new_copy(Surfer * surfer)
{
	Surfer * ret;
	GtkWidget * view;
	char const * url = NULL;
	
	if((view = surfer_get_view(surfer)) != NULL)
		url = ghtml_get_location(view);
	if((ret = surfer_new(url)) == NULL)
		return NULL;
	/* FIXME also copy history */
	return ret;
}


/* surfer_delete */
void surfer_delete(Surfer * surfer)
{
	gtk_widget_destroy(surfer->window);
	config_delete(surfer->config);
	free(surfer->user_agent);
	free(surfer->proxy_http);
	free(surfer->download_dir);
	free(surfer->homepage);
	object_delete(surfer);
	if(--_surfer_cnt == 0)
		gtk_main_quit();
}


/* accessors */
/* surfer_get_view */
GtkWidget * surfer_get_view(Surfer * surfer)
{
	int cur;

	if((cur = gtk_notebook_get_current_page(GTK_NOTEBOOK(surfer->notebook)))
			< 0)
		return NULL;
	return gtk_notebook_get_nth_page(GTK_NOTEBOOK(surfer->notebook), cur);
}


/* surfer_set_enable_javascript */
void surfer_set_enable_javascript(Surfer * surfer, gboolean enable)
{
	GtkWidget * view;
	gint n;
	gint i;

	n = gtk_notebook_get_n_pages(GTK_NOTEBOOK(surfer->notebook));
	for(i = 0; i < n; i++)
	{
		view = gtk_notebook_get_nth_page(GTK_NOTEBOOK(surfer->notebook),
				i);
		ghtml_set_enable_javascript(view, enable);
	}
}


/* surfer_set_fullscreen */
void surfer_set_fullscreen(Surfer * surfer, gboolean fullscreen)
{
	if(fullscreen == TRUE)
		gtk_window_fullscreen(GTK_WINDOW(surfer->window));
	else
		gtk_window_unfullscreen(GTK_WINDOW(surfer->window));
}


/* surfer_set_homepage */
void surfer_set_homepage(Surfer * surfer, char const * homepage)
{
	free(surfer->homepage);
	surfer->homepage = strdup(homepage);
}


/* surfer_set_location */
void surfer_set_location(Surfer * surfer, char const * url)
{
	static int i = 0; /* XXX should be set per-window */
	GtkWidget * widget;
	GtkWidget * view;

	if((view = surfer_get_view(surfer)) == NULL)
		return; /* consider the current tab only */
	url = ghtml_get_location(view);
	if(url == NULL)
		url = "";
	widget = gtk_bin_get_child(GTK_BIN(surfer->lb_path));
	gtk_entry_set_text(GTK_ENTRY(widget), url);
	if(i == 8)
#if GTK_CHECK_VERSION(3, 0, 0)
		gtk_combo_box_text_remove(GTK_COMBO_BOX_TEXT(surfer->lb_path),
				0);
#else
		gtk_combo_box_remove_text(GTK_COMBO_BOX(surfer->lb_path), 0);
#endif
	else
		i++;
#if GTK_CHECK_VERSION(3, 0, 0)
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(surfer->lb_path), NULL,
			url);
#else
	gtk_combo_box_append_text(GTK_COMBO_BOX(surfer->lb_path), url);
#endif
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_back), ghtml_can_go_back(
				view));
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_forward), 
			ghtml_can_go_forward(view));
}


/* surfer_set_progress */
void surfer_set_progress(Surfer * surfer, gdouble fraction)
{
	GtkWidget * view;
	char buf[10] = " ";

	if((view = surfer_get_view(surfer)) == NULL)
		return; /* consider the current tab only */
	fraction = ghtml_get_progress(view);
	if(fraction >= 0.0 && fraction < 1.0)
		snprintf(buf, sizeof(buf), "%.1f%%", fraction * 100);
	else
		fraction = 0.0;
#if GTK_CHECK_VERSION(2, 16, 0)
	view = gtk_bin_get_child(GTK_BIN(surfer->lb_path));
	gtk_entry_set_progress_fraction(GTK_ENTRY(view), fraction);
#else
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(surfer->progress), buf);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(surfer->progress),
			fraction);
#endif
}


/* surfer_set_proxy */
void surfer_set_proxy(Surfer * surfer, SurferProxyType type, char const * http,
		unsigned int http_port)
{
	GtkWidget * view;
	gint n;
	gint i;

	surfer->proxy_type = type;
	free(surfer->proxy_http);
	if(http == NULL)
		surfer->proxy_http = NULL;
	else if((surfer->proxy_http = strdup(http)) == NULL)
		return; /* XXX report error */
	surfer->proxy_http_port = http_port;
	n = gtk_notebook_get_n_pages(GTK_NOTEBOOK(surfer->notebook));
	for(i = 0; i < n; i++)
	{
		view = gtk_notebook_get_nth_page(GTK_NOTEBOOK(surfer->notebook),
				i);
		ghtml_set_proxy(view, type, http, http_port);
	}
}


/* surfer_set_security */
void surfer_set_security(Surfer * surfer, SurferSecurity security)
{
	GtkWidget * view;
	char const * level;
	GdkColor yellow = { 0, 65535, 65535, 16383 };
	GdkColor red = { 0, 65535, 16383, 16383 };
	GdkColor * color = NULL;

	if((view = surfer_get_view(surfer)) == NULL)
		return; /* consider the current tab only */
	security = ghtml_get_security(view);
	switch(security)
	{
		case SS_TRUSTED:
			color = &yellow;
			level = "security-high";
			break;
		case SS_UNTRUSTED:
			color = &red;
			level = "security-medium";
			break;
		case SS_NONE:
		default:
			level = "security-low";
			break;
	}
	gtk_widget_modify_base(gtk_bin_get_child(GTK_BIN(surfer->lb_path)),
			GTK_STATE_NORMAL, color);
	gtk_image_set_from_icon_name(GTK_IMAGE(surfer->security), level,
			GTK_ICON_SIZE_MENU);
}


/* surfer_set_status */
void surfer_set_status(Surfer * surfer, char const * status)
{
	GtkWidget * view;
	GtkStatusbar * sb = GTK_STATUSBAR(surfer->statusbar);

	if((view = surfer_get_view(surfer)) == NULL)
		return; /* consider the current tab only */
	status = ghtml_get_status(view);
	if(surfer->statusbar_id != 0)
		gtk_statusbar_remove(sb, gtk_statusbar_get_context_id(sb, ""),
				surfer->statusbar_id);
	surfer->statusbar_id = gtk_statusbar_push(sb,
			gtk_statusbar_get_context_id(sb, ""), (status != NULL)
			? status : _("Ready"));
	if(status == NULL)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_stop), FALSE);
		surfer_set_progress(surfer, -1.0);
#ifdef EMBEDDED
		gtk_widget_hide(surfer->statusbox);
	}
	else
	{
		gtk_widget_show(surfer->statusbox);
#endif
	}
}


/* surfer_set_title */
void surfer_set_title(Surfer * surfer, char const * title)
{
	GtkWidget * view;
	char buf[256];
	gint n;
	gint i;
	GtkWidget * label;

	if((view = surfer_get_view(surfer)) == NULL)
		return; /* consider the current tab only */
	title = ghtml_get_title(view);
	snprintf(buf, sizeof(buf), "%s%s%s", _("Web surfer"), (title != NULL)
			? " - " : "", (title != NULL) ? title : "");
	gtk_window_set_title(GTK_WINDOW(surfer->window), buf);
	/* XXX this could all be much more efficient */
	n = gtk_notebook_get_n_pages(GTK_NOTEBOOK(surfer->notebook));
	for(i = 0; i < n; i++)
	{
		view = gtk_notebook_get_nth_page(GTK_NOTEBOOK(surfer->notebook),
				i);
		if((label = g_object_get_data(G_OBJECT(view), "label")) == NULL)
			continue;
		title = ghtml_get_title(view);
		gtk_label_set_text(GTK_LABEL(label), (title != NULL) ? title
				: _("Untitled"));
	}
}


/* surfer_set_user_agent */
void surfer_set_user_agent(Surfer * surfer, char const * user_agent)
{
	GtkWidget * view;
	char * p = NULL;
	gint n;
	gint i;

	if(user_agent != NULL && (p = strdup(user_agent)) == NULL)
		return; /* XXX report error */
	free(surfer->user_agent);
	surfer->user_agent = p;
	n = gtk_notebook_get_n_pages(GTK_NOTEBOOK(surfer->notebook));
	for(i = 0; i < n; i++)
	{
		view = gtk_notebook_get_nth_page(GTK_NOTEBOOK(surfer->notebook),
				i);
		ghtml_set_user_agent(view, user_agent);
	}
}


/* surfer_set_zoom */
void surfer_set_zoom(Surfer * surfer, gdouble zoom)
{
	GtkWidget * view;
	GtkWidget * widget;

	if((view = surfer_get_view(surfer)) == NULL)
		return; /* consider the current tab only */
	if(zoom < 0.0)
	{
		zoom = ghtml_get_zoom(view) * 100.0;
		widget = gtk_bin_get_child(GTK_BIN(surfer->tb_zoom));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), zoom);
	}
	else
		ghtml_set_zoom(view, zoom);
}


/* useful */
/* surfer_about */
static gboolean _about_on_closex(gpointer data);

void surfer_about(Surfer * surfer)
{
	if(surfer->ab_dialog != NULL)
	{
		gtk_widget_show(surfer->ab_dialog);
		return;
	}
	surfer->ab_dialog = desktop_about_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(surfer->ab_dialog),
			GTK_WINDOW(surfer->window));
	desktop_about_dialog_set_authors(surfer->ab_dialog, _authors);
	desktop_about_dialog_set_copyright(surfer->ab_dialog, _copyright);
	desktop_about_dialog_set_logo_icon_name(surfer->ab_dialog,
			"web-browser");
	desktop_about_dialog_set_license(surfer->ab_dialog, _license);
	desktop_about_dialog_set_name(surfer->ab_dialog, PACKAGE);
	desktop_about_dialog_set_translator_credits(surfer->ab_dialog,
			_("translator-credits"));
	desktop_about_dialog_set_version(surfer->ab_dialog, VERSION);
	desktop_about_dialog_set_website(surfer->ab_dialog,
			"http://www.defora.org/");
	g_signal_connect_swapped(surfer->ab_dialog, "delete-event", G_CALLBACK(
				_about_on_closex), surfer);
	gtk_widget_show(surfer->ab_dialog);
}

static gboolean _about_on_closex(gpointer data)
{
	Surfer * surfer = data;

	gtk_widget_hide(surfer->ab_dialog);
	return TRUE;
}


/* surfer_close_tab */
void surfer_close_tab(Surfer * surfer, GtkWidget * view)
{
	gint n;

	if(gtk_notebook_get_n_pages(GTK_NOTEBOOK(surfer->notebook)) == 1)
	{
		surfer_delete(surfer);
		return;
	}
	if(view == NULL)
	{
		n = gtk_notebook_get_current_page(GTK_NOTEBOOK(
					surfer->notebook));
		view = gtk_notebook_get_nth_page(GTK_NOTEBOOK(surfer->notebook),
				n);
	}
	else if((n = gtk_notebook_page_num(GTK_NOTEBOOK(surfer->notebook),
					view)) < 0)
		return; /* XXX return error */
	ghtml_delete(view);
	gtk_notebook_remove_page(GTK_NOTEBOOK(surfer->notebook), n);
	if(gtk_notebook_get_n_pages(GTK_NOTEBOOK(surfer->notebook)) == 1)
		gtk_notebook_set_show_tabs(GTK_NOTEBOOK(surfer->notebook),
				FALSE);
}


/* surfer_config_load */
int surfer_config_load(Surfer * surfer)
{
	char * filename;
	char const * p;
	char buf[256];
	unsigned int u;

	if((filename = _config_get_filename(SURFER_CONFIG_FILE)) == NULL)
		return 1;
	config_load(surfer->config, filename); /* XXX ignore errors */
	free(filename);
	_config_load_string(surfer->config, NULL, "homepage",
			&surfer->homepage);
	_config_load_string(surfer->config, NULL, "download_directory",
			&surfer->download_dir);
	_config_load_integer(surfer->config, NULL, "download_close",
			&surfer->download_close);
	if((p = getenv("http_proxy")) != NULL && sscanf(p, "http://%255[^:]:%u",
				buf, &u) == 2)
	{
		surfer->proxy_type = SPT_HTTP;
		buf[sizeof(buf) - 1] = '\0';
		surfer->proxy_http = strdup(buf);
		surfer->proxy_http_port = u;
	}
	else
	{
		_config_load_integer(surfer->config, "proxy", "type",
				&surfer->proxy_type);
		_config_load_string(surfer->config, "proxy", "http",
				&surfer->proxy_http);
		_config_load_integer(surfer->config, "proxy", "http_port",
				&surfer->proxy_http_port);
	}
	_config_load_string(surfer->config, NULL, "user_agent",
			&surfer->user_agent);
	_config_load_integer(surfer->config, NULL, "javascript", &u);
	surfer->javascript = u;
	return 0;
}


/* surfer_config_save */
int surfer_config_save(Surfer * surfer)
{
	int ret = 0;
	char * filename;

	if((filename = _config_get_filename(SURFER_CONFIG_FILE)) == NULL)
		return 1;
	ret |= _config_save_string(surfer->config, NULL, "homepage",
			surfer->homepage);
	ret |= _config_save_string(surfer->config, NULL, "download_directory",
			surfer->download_dir);
	ret |= _config_save_boolean(surfer->config, NULL, "download_close",
			surfer->download_close);
	ret |= _config_save_boolean(surfer->config, NULL, "focus_new_tabs",
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
					surfer->pr_focus_tabs)));
	ret |= _config_save_integer(surfer->config, "proxy", "type",
			surfer->proxy_type);
	ret |= _config_save_string(surfer->config, "proxy", "http",
			surfer->proxy_http);
	ret |= _config_save_integer(surfer->config, "proxy", "http_port",
			surfer->proxy_http_port);
	ret |= _config_save_string(surfer->config, NULL, "user_agent",
			surfer->user_agent);
	ret |= _config_save_boolean(surfer->config, NULL, "javascript",
			surfer->javascript);
	if(ret == 0)
		ret |= config_save(surfer->config, filename);
	free(filename);
	return ret;
}


/* surfer_confirm */
int surfer_confirm(Surfer * surfer, char const * message, gboolean * confirmed)
{
	int ret = 0;
	GtkWidget * dialog;
	int res;

	dialog = gtk_message_dialog_new((surfer != NULL)
			? GTK_WINDOW(surfer->window) : NULL,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "%s",
			_("Question"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
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


/* surfer_console_clear */
void surfer_console_clear(Surfer * surfer)
{
	gtk_list_store_clear(surfer->co_store);
}


/* surfer_console_execute */
void surfer_console_execute(Surfer * surfer)
{
	GtkWidget * view;
	char const * text;

	if((view = surfer_get_view(surfer)) == NULL)
		return;
	if((text = gtk_entry_get_text(GTK_ENTRY(surfer->co_entry))) == NULL
			|| strlen(text) == 0)
		return;
	ghtml_execute(view, text);
}


/* surfer_console_message */
void surfer_console_message(Surfer * surfer, char const * message,
		char const * source, long line)
{
	GtkTreeIter iter;
	char buf[32] = "";

	if(line < 0)
		line = -1;
	else
		snprintf(buf, sizeof(buf), "%ld", line);
	gtk_list_store_append(surfer->co_store, &iter);
	gtk_list_store_set(surfer->co_store, &iter, SCM_MESSAGE, message,
			SCM_SOURCE, source,
			(line >= 0) ? SCM_LINE : -1, line,
			SCM_DISPLAY_LINE, buf,
			-1);
}


/* surfer_copy */
void surfer_copy(Surfer * surfer)
{
	GtkWidget * entry;
	GtkWidget * view;

	entry = gtk_bin_get_child(GTK_BIN(surfer->lb_path));
	if(gtk_window_get_focus(GTK_WINDOW(surfer->window)) == entry)
		gtk_editable_copy_clipboard(GTK_EDITABLE(entry));
	else if((view = surfer_get_view(surfer)) != NULL)
		ghtml_copy(view);
}


/* surfer_cut */
void surfer_cut(Surfer * surfer)
{
	GtkWidget * entry;
	GtkWidget * view;

	entry = gtk_bin_get_child(GTK_BIN(surfer->lb_path));
	if(gtk_window_get_focus(GTK_WINDOW(surfer->window)) == entry)
		gtk_editable_cut_clipboard(GTK_EDITABLE(entry));
	else if((view = surfer_get_view(surfer)) != NULL)
		ghtml_cut(view);
}


/* surfer_download */
int surfer_download(Surfer * surfer, char const * url, char const * suggested)
{
	int ret = 0;
	GtkWidget * dialog;
	char * filename = NULL;
#ifdef WITH_DOWNLOAD
	DownloadPrefs prefs;
	Download * download;
#else
	char * argv[] = { "download", "-O", NULL, NULL, NULL };
	GError * error = NULL;
#endif

	if(url == NULL)
		return -surfer_error(surfer, strerror(EINVAL), 1);
	dialog = gtk_file_chooser_dialog_new(_("Save file as..."),
			GTK_WINDOW(surfer->window),
			GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE,
			GTK_RESPONSE_ACCEPT, NULL);
	if(surfer->download_dir != NULL)
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),
				surfer->download_dir);
	if(suggested != NULL) /* XXX also suggest a name otherwise */
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog),
				suggested);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
					dialog));
	gtk_widget_destroy(dialog);
	if(filename == NULL)
		return 0;
	if(_surfer_filename_confirm(surfer, filename) != TRUE)
	{
		g_free(filename);
		return 0;
	}
#ifdef WITH_DOWNLOAD
	prefs.output = filename;
	prefs.user_agent = NULL;
	if((download = download_new(&prefs, url)) == NULL)
		ret = -surfer_error(surfer, error_get(), 1);
	else
		download_set_close(download, surfer->download_close);
#else
	argv[2] = filename;
	if((argv[3] = strdup(url)) == NULL)
		ret = -surfer_error(surfer, strerror(errno), 1);
	else
	{
		g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL,
				NULL, &error);
		free(argv[3]);
	}
#endif
	g_free(filename);
	return ret;
}


/* surfer_error */
int surfer_error(Surfer * surfer, char const * message, int ret)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new((surfer != NULL)
			? GTK_WINDOW(surfer->window) : NULL,
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


/* surfer_find */
static void _find_dialog(Surfer * surfer);
static void _on_find_activate(GtkWidget * widget, gpointer data);
static void _on_find_response(GtkWidget * widget, gint response, gpointer data);

void surfer_find(Surfer * surfer, char const * text)
{
	if(surfer->fi_dialog == NULL)
		_find_dialog(surfer);
	gtk_widget_grab_focus(surfer->fi_text);
	if(text != NULL)
		gtk_entry_set_text(GTK_ENTRY(surfer->fi_text), text);
	gtk_window_present(GTK_WINDOW(surfer->fi_dialog));
}

static void _find_dialog(Surfer * surfer)
{
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;

	surfer->fi_dialog = gtk_dialog_new_with_buttons(_("Find text"),
			GTK_WINDOW(surfer->window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
			GTK_STOCK_FIND, GTK_RESPONSE_ACCEPT, NULL);
#if GTK_CHECK_VERSION(2, 14, 0)
	vbox = gtk_dialog_get_content_area(GTK_DIALOG(surfer->fi_dialog));
#else
	vbox = GTK_DIALOG(surfer->fi_dialog)->vbox;
#endif
	/* text */
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new(_("Text:"));
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	surfer->fi_text = gtk_entry_new();
	g_signal_connect(G_OBJECT(surfer->fi_text), "activate", G_CALLBACK(
				_on_find_activate), surfer);
	gtk_box_pack_start(GTK_BOX(hbox), surfer->fi_text, TRUE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 4);
	/* case-sensitive */
	surfer->fi_case = gtk_check_button_new_with_label(_("Case-sensitive"));
	gtk_box_pack_start(GTK_BOX(vbox), surfer->fi_case, TRUE, TRUE, 4);
	/* search backwards */
	surfer->fi_back = gtk_check_button_new_with_label(
			_("Search backwards"));
	gtk_box_pack_start(GTK_BOX(vbox), surfer->fi_back, TRUE, TRUE, 4);
	/* wrap */
	surfer->fi_wrap = gtk_check_button_new_with_label(_("Wrap"));
	gtk_box_pack_start(GTK_BOX(vbox), surfer->fi_wrap, TRUE, TRUE, 4);
	gtk_widget_show_all(vbox);
	g_signal_connect(G_OBJECT(surfer->fi_dialog), "response", G_CALLBACK(
				_on_find_response), surfer);
}

static void _on_find_activate(GtkWidget * widget, gpointer data)
{
	Surfer * surfer = data;
	GtkWidget * view;
	char const * text;
	gboolean sensitive;
	gboolean backwards;
	gboolean wrap;

	if((view = surfer_get_view(surfer)) == NULL)
		return;
	if((text = gtk_entry_get_text(GTK_ENTRY(widget))) == NULL
			|| strlen(text) == 0)
		return;
	sensitive = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
				surfer->fi_case));
	backwards = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
				surfer->fi_back));
	wrap = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
				surfer->fi_wrap));
	if(ghtml_find(view, text, sensitive, backwards, wrap) == TRUE)
		return;
	/* FIXME display this error on top of the text search box instead */
	surfer_error(surfer, _("Text not found"), 0);
}

static void _on_find_response(GtkWidget * widget, gint response, gpointer data)
{
	Surfer * surfer = data;

	if(response != GTK_RESPONSE_ACCEPT)
	{
		gtk_widget_hide(widget);
		if(response == GTK_RESPONSE_DELETE_EVENT)
			surfer->fi_dialog = NULL;
		return;
	}
	_on_find_activate(surfer->fi_text, surfer);
}


/* surfer_go_back */
gboolean surfer_go_back(Surfer * surfer)
{
	gboolean ret;
	GtkWidget * view;

	if((view = surfer_get_view(surfer)) == NULL)
		return FALSE;
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_stop), TRUE);
	ret = ghtml_go_back(view);
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_back), ret);
	return ret;
}


/* surfer_go_forward */
gboolean surfer_go_forward(Surfer * surfer)
{
	gboolean ret;
	GtkWidget * view;

	if((view = surfer_get_view(surfer)) == NULL)
		return FALSE;
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_stop), TRUE);
	ret = ghtml_go_forward(view);
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_forward), ret);
	return ret;
}


/* surfer_go_home */
void surfer_go_home(Surfer * surfer)
{
	char const * homepage;

	if((homepage = config_get(surfer->config, NULL, "homepage")) == NULL)
		return;
	surfer_open(surfer, homepage);
}


/* surfer_open */
void surfer_open(Surfer * surfer, char const * url)
{
	GtkWidget * view;

	if((view = surfer_get_view(surfer)) == NULL)
	{
		surfer_open_tab(surfer, url);
		return;
	}
	if(url != NULL && url[0] != '\0')
	{
		view = surfer_get_view(surfer);
		gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_refresh), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_stop), TRUE);
		ghtml_stop(view);
		ghtml_load_url(view, url);
	}
	else
		gtk_widget_grab_focus(surfer->lb_path);
}


/* surfer_open_dialog */
void surfer_open_dialog(Surfer * surfer)
{
	GtkWidget * dialog;
	GtkFileFilter * filter;
	char * filename = NULL;

	dialog = gtk_file_chooser_dialog_new(_("Open file..."),
			GTK_WINDOW(surfer->window),
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, _("HTML files"));
	gtk_file_filter_add_mime_type(filter, "application/xhtml+xml");
	gtk_file_filter_add_mime_type(filter, "text/html");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, _("All files"));
	gtk_file_filter_add_pattern(filter, "*");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
					dialog));
	gtk_widget_destroy(dialog);
	if(filename == NULL)
		return;
	surfer_open(surfer, filename);
	g_free(filename);
}


/* surfer_open_tab */
static GtkWidget * _tab_button(Surfer * surfer, GtkWidget * widget,
		char const * text);

void surfer_open_tab(Surfer * surfer, char const * url)
{
	GtkWidget * widget;
	GtkNotebook * notebook = GTK_NOTEBOOK(surfer->notebook);
	int i;
	char const * p;

	if((widget = ghtml_new(surfer)) == NULL)
	{
		surfer_error(NULL, _("Could not initialize HTML renderer"), 0);
		return;
	}
	if(surfer->proxy_http != NULL)
		ghtml_set_proxy(widget, surfer->proxy_type, surfer->proxy_http,
				surfer->proxy_http_port);
	if(surfer->user_agent == NULL || surfer->user_agent[0] == '\0')
		ghtml_set_user_agent(widget, NULL);
	else
		ghtml_set_user_agent(widget, surfer->user_agent);
	ghtml_set_enable_javascript(widget, surfer->javascript);
	gtk_widget_show_all(widget); /* must be before set_current_page() */
	if(url != NULL && url[0] != '\0')
		ghtml_load_url(widget, url);
	if((i = gtk_notebook_append_page(notebook, widget, _tab_button(surfer,
						widget, _("Untitled")))) > 0)
	{
		gtk_notebook_set_show_tabs(notebook, TRUE);
#if GTK_CHECK_VERSION(2, 10, 0)
		gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(surfer->notebook),
				widget, TRUE);
#endif
		if((p = config_get(surfer->config, "", "focus_new_tabs"))
				!= NULL && strcmp(p, "1") == 0)
		{
			gtk_notebook_set_current_page(notebook, i);
			gtk_widget_grab_focus(surfer->lb_path);
		}
	}
	else
		gtk_notebook_set_show_tabs(notebook, FALSE);
}

static GtkWidget * _tab_button(Surfer * surfer, GtkWidget * widget,
		char const * text)
{
	GtkWidget * hbox;
	GtkWidget * label;
	GtkWidget * button;

	hbox = gtk_hbox_new(FALSE, 0);
	label = gtk_label_new(text);
	gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
	gtk_label_set_width_chars(GTK_LABEL(label), 10);
	g_object_set_data(G_OBJECT(widget), "label", label);
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 4);
	button = gtk_button_new();
	g_object_set_data(G_OBJECT(button), "widget", widget);
	gtk_button_set_image(GTK_BUTTON(button), gtk_image_new_from_stock(
				GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU));
	gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(
				on_notebook_close_tab), surfer);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, TRUE, 0);
	gtk_widget_show_all(hbox);
	return hbox;
}


/* surfer_paste */
void surfer_paste(Surfer * surfer)
{
	GtkWidget * entry;
	GtkWidget * view;

	entry = gtk_bin_get_child(GTK_BIN(surfer->lb_path));
	if(gtk_window_get_focus(GTK_WINDOW(surfer->window)) == entry)
		gtk_editable_paste_clipboard(GTK_EDITABLE(entry));
	else if((view = surfer_get_view(surfer)) != NULL)
		ghtml_paste(view);
}


/* surfer_print */
void surfer_print(Surfer * surfer)
{
	GtkWidget * view;

	if((view = surfer_get_view(surfer)) == NULL)
		return;
	ghtml_print(view);
}


/* surfer_prompt */
int surfer_prompt(Surfer * surfer, char const * message,
		char const * default_value, char ** value)
{
	int ret = 0;
	GtkWidget * dialog;
	GtkWidget * vbox;
	GtkWidget * entry;
	int res;

	dialog = gtk_message_dialog_new((surfer != NULL)
			? GTK_WINDOW(surfer->window) : NULL,
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
	else
		ret = 1;
	gtk_widget_destroy(dialog);
	return ret;
}


/* surfer_refresh */
void surfer_refresh(Surfer * surfer)
{
	GtkWidget * view;

	if((view = surfer_get_view(surfer)) == NULL)
		return;
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_stop), TRUE);
	ghtml_refresh(view);
}


/* surfer_reload */
void surfer_reload(Surfer * surfer)
{
	GtkWidget * view;

	if((view = surfer_get_view(surfer)) == NULL)
		return;
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_stop), TRUE);
	ghtml_reload(view);
}


/* surfer_redo */
void surfer_redo(Surfer * surfer)
{
	GtkWidget * view;

	if((view = surfer_get_view(surfer)) == NULL)
		return;
	ghtml_redo(view);
}


/* surfer_resize */
void surfer_resize(Surfer * surfer, gint width, gint height)
{
	gtk_window_resize(GTK_WINDOW(surfer->window), width, height);
}


/* surfer_save */
void surfer_save(Surfer * surfer, char const * filename)
{
	GtkWidget * view;
	char const * source;
	GtkWidget * dialog;
	size_t len;
	FILE * fp;
	char buf[256];

	if((view = surfer_get_view(surfer)) == NULL)
		return;
	if((source = ghtml_get_source(view)) == NULL)
		return; /* XXX report error */
	if(filename == NULL)
	{
		dialog = gtk_file_chooser_dialog_new(_("Save file as..."),
				GTK_WINDOW(surfer->window),
				GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL,
				GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE,
				GTK_RESPONSE_ACCEPT, NULL);
		if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
			filename = gtk_file_chooser_get_filename(
					GTK_FILE_CHOOSER(dialog));
		gtk_widget_destroy(dialog);
		if(filename == NULL)
			return;
	}
	if(_surfer_filename_confirm(surfer, filename) != TRUE)
		return;
	if((fp = fopen(filename, "w")) == NULL) /* XXX use GIOChannel instead */
	{
		snprintf(buf, sizeof(buf), "%s: %s", filename, strerror(errno));
		surfer_error(surfer, buf, 0);
		return;
	}
	len = strlen(source);
	if(fwrite(source, sizeof(*source), len, fp) != len)
	{
		snprintf(buf, sizeof(buf), "%s: %s", filename, strerror(errno));
		surfer_error(surfer, buf, 0);
	}
	fclose(fp);
}


/* surfer_select_all */
void surfer_select_all(Surfer * surfer)
{
	GtkWidget * widget;

	/* XXX this assumes the keyboard shortcut was used */
	widget = gtk_bin_get_child(GTK_BIN(surfer->lb_path));
	if(gtk_window_get_focus(GTK_WINDOW(surfer->window)) == widget)
	{
		gtk_editable_set_position(GTK_EDITABLE(widget), 0);
		return;
	}
	if((widget = surfer_get_view(surfer)) == NULL)
		return;
	ghtml_select_all(widget);
}


/* surfer_show_console */
void surfer_show_console(Surfer * surfer, gboolean show)
{
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;

	if(surfer->co_window != NULL)
	{
		if(show == TRUE)
			gtk_widget_show(surfer->co_window);
		else
			gtk_widget_hide(surfer->co_window);
		return;
	}
	surfer->co_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(surfer->co_window), 400, 300);
	gtk_window_set_title(GTK_WINDOW(surfer->co_window),
			_("Javascript console"));
	g_signal_connect_swapped(surfer->co_window, "delete-event", G_CALLBACK(
				on_console_closex), surfer);
	vbox = gtk_vbox_new(FALSE, 0);
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new(_("Command:"));
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	surfer->co_entry = gtk_entry_new();
	g_signal_connect_swapped(surfer->co_entry, "activate", G_CALLBACK(
				on_console_execute), surfer);
	gtk_box_pack_start(GTK_BOX(hbox), surfer->co_entry, TRUE, TRUE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_EXECUTE);
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(
				on_console_execute), surfer);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 4);
	/* view */
	hbox = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(hbox),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	widget = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				surfer->co_store));
	/* message */
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Message"),
			renderer, "text", SCM_MESSAGE, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	/* source */
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Source"),
			renderer, "text", SCM_SOURCE, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	/* line */
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Line"),
			renderer, "text", SCM_DISPLAY_LINE, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	gtk_container_add(GTK_CONTAINER(hbox), widget);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
	/* dialog */
	hbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbox), GTK_BUTTONBOX_END);
	gtk_box_set_spacing(GTK_BOX(hbox), 4);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_CLEAR);
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(
				on_console_clear), surfer);
	gtk_container_add(GTK_CONTAINER(hbox), widget);
	widget = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(
				on_console_close), surfer);
	gtk_container_add(GTK_CONTAINER(hbox), widget);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	gtk_widget_show_all(vbox);
	gtk_container_add(GTK_CONTAINER(surfer->co_window), vbox);
	surfer_show_console(surfer, show);
}


/* surfer_show_menubar */
void surfer_show_menubar(Surfer * surfer, gboolean show)
{
#ifndef EMBEDDED
	if(show == TRUE)
		gtk_widget_show(surfer->menubar);
	else
		gtk_widget_hide(surfer->menubar);
#endif
}


/* surfer_show_statusbar */
void surfer_show_statusbar(Surfer * surfer, gboolean show)
{
	if(show == TRUE)
		gtk_widget_show(surfer->statusbar);
	else
		gtk_widget_hide(surfer->statusbar);
}


/* surfer_show_toolbar */
void surfer_show_toolbar(Surfer * surfer, gboolean show)
{
	if(show == TRUE)
	{
		gtk_widget_show(surfer->toolbar);
		gtk_widget_show(surfer->locationbar);
	}
	else
	{
		gtk_widget_hide(surfer->toolbar);
		gtk_widget_hide(surfer->locationbar);
	}
}


/* surfer_show_window */
void surfer_show_window(Surfer * surfer, gboolean show)
{
	if(show == TRUE)
		gtk_widget_show(surfer->window);
	else
		gtk_widget_hide(surfer->window);
}


/* surfer_stop */
void surfer_stop(Surfer * surfer)
{
	GtkWidget * view;

	if((view = surfer_get_view(surfer)) == NULL)
		return;
	ghtml_stop(view);
}


/* surfer_undo */
void surfer_undo(Surfer * surfer)
{
	GtkWidget * view;

	if((view = surfer_get_view(surfer)) == NULL)
		return;
	ghtml_undo(view);
}


/* surfer_unselect_all */
void surfer_unselect_all(Surfer * surfer)
{
	GtkWidget * view;

	if((view = surfer_get_view(surfer)) == NULL)
		return;
	ghtml_unselect_all(view);
}


/* surfer_view_preferences */
static GtkWidget * _preferences_general(Surfer * surfer);
static GtkWidget * _preferences_network(Surfer * surfer);
static GtkWidget * _preferences_advanced(Surfer * surfer);
static void _preferences_set(Surfer * surfer);
/* callbacks */
static gboolean _preferences_on_closex(gpointer data);
static void _preferences_on_response(GtkWidget * widget, gint response,
		gpointer data);
static void _preferences_on_cancel(gpointer data);
static void _preferences_on_ok(gpointer data);
static void _preferences_on_proxy_http_toggled(gpointer data);

void surfer_view_preferences(Surfer * surfer)
{
	GtkWidget * vbox;
	GtkWidget * notebook;
	GtkWidget * page;

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
	g_signal_connect_swapped(surfer->pr_window, "delete-event", G_CALLBACK(
				_preferences_on_closex), surfer);
	g_signal_connect(surfer->pr_window, "response", G_CALLBACK(
				_preferences_on_response), surfer);
#if GTK_CHECK_VERSION(2, 14, 0)
	vbox = gtk_dialog_get_content_area(GTK_DIALOG(surfer->pr_window));
#else
	vbox = GTK_DIALOG(surfer->pr_window)->vbox;
#endif
	/* notebook */
	notebook = gtk_notebook_new();
	/* general tab */
	page = _preferences_general(surfer);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page,
			gtk_label_new(_("General")));
	/* network tab */
	page = _preferences_network(surfer);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page,
			gtk_label_new(_("Network")));
	/* advanced tab */
	page = _preferences_advanced(surfer);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page,
			gtk_label_new(_("Advanced")));
	gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);
	_preferences_set(surfer);
	gtk_widget_show_all(surfer->pr_window);
}

static GtkWidget * _preferences_general(Surfer * surfer)
{
	GtkWidget * page;
	GtkWidget * hbox;
	GtkWidget * frame;
	GtkWidget * widget;

	page = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(page), 4);
	/* homepage */
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new(_("Homepage:"));
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	surfer->pr_homepage = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), surfer->pr_homepage, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(page), hbox, FALSE, TRUE, 0);
	/* tabs */
	frame = gtk_frame_new(_("Tab handling"));
	/* focus new tabs */
	hbox = gtk_hbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);
	surfer->pr_focus_tabs = gtk_check_button_new_with_label(
			_("Focus new tabs"));
	gtk_box_pack_start(GTK_BOX(hbox), surfer->pr_focus_tabs, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(frame), hbox);
	gtk_box_pack_start(GTK_BOX(page), frame, FALSE, TRUE, 0);
	/* downloads */
	frame = gtk_frame_new(_("Downloads"));
	hbox = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);
	widget = gtk_label_new(_("Default download directory:"));
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	surfer->pr_download_dir = gtk_file_chooser_button_new(
			_("Choose the default download directory"),
			GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_box_pack_start(GTK_BOX(hbox), surfer->pr_download_dir, TRUE, TRUE,
			0);
	surfer->pr_download_close = gtk_check_button_new_with_mnemonic(
			_("_Close download windows when complete"));
	gtk_box_pack_start(GTK_BOX(hbox), surfer->pr_download_close, FALSE,
			TRUE, 0);
	gtk_container_add(GTK_CONTAINER(frame), hbox);
	gtk_box_pack_start(GTK_BOX(page), frame, FALSE, TRUE, 0);
	return page;
}

static GtkWidget * _preferences_network(Surfer * surfer)
{
	GtkWidget * page;
	GtkWidget * frame;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;

	page = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(page), 4);
	frame = gtk_frame_new(_("Connectivity:"));
	vbox = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	widget = gtk_radio_button_new_with_label(NULL, _("Direct connection"));
	surfer->pr_proxy_radio_direct = widget;
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	widget = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(
				widget), _("HTTP proxy:"));
	surfer->pr_proxy_radio_http = widget;
	g_signal_connect_swapped(widget, "toggled", G_CALLBACK(
				_preferences_on_proxy_http_toggled), surfer);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
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
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_box_pack_start(GTK_BOX(page), frame, FALSE, TRUE, 0);
	return page;
}

static GtkWidget * _preferences_advanced(Surfer * surfer)
{
	GtkWidget * page;
	GtkWidget * hbox;
	GtkWidget * widget;

	page = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(page), 4);
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new(_("User agent:"));
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	surfer->pr_user_agent = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), surfer->pr_user_agent, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(page), hbox, FALSE, TRUE, 0);
	surfer->pr_javascript = gtk_check_button_new_with_mnemonic(
			_("Enable _Javascript"));
	gtk_box_pack_start(GTK_BOX(page), surfer->pr_javascript, FALSE, TRUE,
			0);
	return page;
}

static void _preferences_set(Surfer * surfer)
{
	char const * p;

	gtk_entry_set_text(GTK_ENTRY(surfer->pr_homepage),
			(surfer->homepage != NULL) ? surfer->homepage : "");
	if((p = config_get(surfer->config, NULL, "focus_new_tabs")) != NULL
			&& strcmp(p, "1") == 0)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					surfer->pr_focus_tabs), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					surfer->pr_focus_tabs), FALSE);
	if(surfer->download_dir == NULL)
		surfer->download_dir = _config_get_filename("Downloads");
	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(surfer->pr_download_dir),
			(surfer->download_dir != NULL) ? surfer->download_dir
			: g_get_home_dir());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
				surfer->pr_download_close),
			surfer->download_close);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					surfer->pr_proxy_radio_http),
			surfer->proxy_type == SPT_HTTP);
	_preferences_on_proxy_http_toggled(surfer);
	if(surfer->proxy_http != NULL)
		gtk_entry_set_text(GTK_ENTRY(surfer->pr_proxy_http),
				surfer->proxy_http);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(surfer->pr_proxy_http_port),
			surfer->proxy_http_port);
	p = config_get(surfer->config, NULL, "user_agent");
	gtk_entry_set_text(GTK_ENTRY(surfer->pr_user_agent), (p != NULL) ? p
			: "");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
				surfer->pr_javascript), surfer->javascript);
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
	surfer->download_close = gtk_toggle_button_get_active(
			GTK_TOGGLE_BUTTON(surfer->pr_download_close));
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
					surfer->pr_proxy_radio_http)))
		type = SPT_HTTP;
	surfer_set_proxy(surfer, type, gtk_entry_get_text(GTK_ENTRY(
					surfer->pr_proxy_http)),
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(
					surfer->pr_proxy_http_port)));
	surfer_set_user_agent(surfer, gtk_entry_get_text(GTK_ENTRY(
					surfer->pr_user_agent)));
	surfer->javascript = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
				surfer->pr_javascript));
	surfer_set_enable_javascript(surfer, surfer->javascript);
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


/* surfer_view_security */
void surfer_view_security(Surfer * surfer)
{
	GtkWidget * view;
	SurferSecurity security;
	char const * text;
	GtkWidget * dialog;

	if((view = surfer_get_view(surfer)) == NULL)
		return; /* consider the current tab only */
	security = ghtml_get_security(view);
	switch(security)
	{
		case SS_TRUSTED:
			text = _("This connection is encrypted and trusted.");
			break;
		case SS_UNTRUSTED:
			text = _("This connection is encrypted but not trusted.");
			break;
		case SS_NONE:
		default:
			text = _("This connection is not encrypted.");
			break;
	}
	dialog = gtk_message_dialog_new(GTK_WINDOW(surfer->window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE,
#if GTK_CHECK_VERSION(2, 8, 0)
			"%s", _("Security information"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
#endif
			"%s", text);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Security information"));
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}


/* surfer_view_source */
static void _on_source_close(GtkWidget * widget);
static gboolean _on_source_closex(void);

void surfer_view_source(Surfer * surfer)
{
	GtkAccelGroup * group;
	GClosure * cc;
	GtkWidget * window;
	GtkWidget * scrolled;
	GtkWidget * widget;
	GtkTextBuffer * tbuf;
	PangoFontDescription * desc;
	char buf[256];
	char const * url;
	char const * source;
	size_t len;

	/* XXX this assumes the keyboard shortcut was used */
	widget = gtk_bin_get_child(GTK_BIN(surfer->lb_path));
	if(gtk_window_get_focus(GTK_WINDOW(surfer->window)) == widget)
	{
		gtk_entry_set_text(GTK_ENTRY(widget), "");
		return;
	}
	if((widget = surfer_get_view(surfer)) == NULL)
		return;
	if((url = ghtml_get_location(widget)) == NULL)
		return;
	if((source = ghtml_get_source(widget)) == NULL)
		return; /* FIXME download to a temporary file and open */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	group = gtk_accel_group_new();
	cc = g_cclosure_new_swap(G_CALLBACK(_on_source_close), window, NULL);
	gtk_accel_group_connect(group, GDK_KEY_W, GDK_CONTROL_MASK,
			GTK_ACCEL_VISIBLE, cc);
	gtk_window_add_accel_group(GTK_WINDOW(window), group);
	gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);
	snprintf(buf, sizeof(buf), "%s%s", _("Web surfer - Source of "), url);
	gtk_window_set_title(GTK_WINDOW(window), buf);
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(
				_on_source_closex), NULL);
	scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	widget = gtk_text_view_new();
	desc = pango_font_description_new();
	pango_font_description_set_family(desc, "monospace");
	gtk_widget_modify_font(widget, desc);
	pango_font_description_free(desc);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(widget), FALSE);
	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	if((len = strlen(source)) > 0)
		/* FIXME probably should convert to UTF-8 when not already */
		gtk_text_buffer_set_text(tbuf, source, len);
	gtk_container_add(GTK_CONTAINER(scrolled), widget);
	gtk_container_add(GTK_CONTAINER(window), scrolled);
	gtk_widget_show_all(window);
}

static void _on_source_close(GtkWidget * widget)
{
	gtk_widget_destroy(widget);
}

static gboolean _on_source_closex(void)
{
	return FALSE;
}


/* surfer_warning */
void surfer_warning(Surfer * surfer, char const * message)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new((surfer != NULL)
			? GTK_WINDOW(surfer->window) : NULL,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE, "%s",
			_("Warning"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s", message);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Warning"));
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}


/* surfer_zoom_in */
void surfer_zoom_in(Surfer * surfer)
{
	GtkWidget * view;

	if((view = surfer_get_view(surfer)) == NULL)
		return;
	ghtml_zoom_in(view);
	surfer_set_zoom(surfer, -1.0);
}


/* surfer_zoom_out */
void surfer_zoom_out(Surfer * surfer)
{
	GtkWidget * view;

	if((view = surfer_get_view(surfer)) == NULL)
		return;
	ghtml_zoom_out(view);
	surfer_set_zoom(surfer, -1.0);
}


/* surfer_zoom_reset */
void surfer_zoom_reset(Surfer * surfer)
{
	GtkWidget * view;

	if((view = surfer_get_view(surfer)) == NULL)
		return;
	ghtml_zoom_reset(view);
	surfer_set_zoom(surfer, -1.0);
}


/* private */
/* functions */
/* surfer_filename_confirm */
static gboolean _surfer_filename_confirm(Surfer * surfer,
		char const * filename)
{
	struct stat st;
	GtkWidget * dialog;
	int res;

	if(stat(filename, &st) != 0)
		return TRUE;
	dialog = gtk_message_dialog_new(GTK_WINDOW(surfer->window),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO, "%s",
#if GTK_CHECK_VERSION(2, 6, 0)
			_("Warning"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(
				dialog), "%s",
#endif
			_("This file already exists. Overwrite?"));
	gtk_window_set_title(GTK_WINDOW(dialog), _("Warning"));
	res = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	if(res == GTK_RESPONSE_NO)
		return FALSE;
	return TRUE;
}


/* config_get_filename */
static char * _config_get_filename(char const * filename)
{
	char * ret;
	char const * homedir;
	size_t len;

	if((homedir = getenv("HOME")) == NULL)
		homedir = g_get_home_dir();
	len = strlen(homedir) + 1 + strlen(filename) + 1;
	if((ret = malloc(len)) == NULL)
		return NULL;
	snprintf(ret, len, "%s/%s", homedir, filename);
	return ret;
}


/* config_load_integer */
static int _config_load_integer(Config * config, char const * section,
		char const * variable, unsigned int * value)
{
	unsigned int u;
	char const * p;
	char * q;

	if((p = config_get(config, section, variable)) == NULL || p[0] == '\0')
		return -1;
	u = strtoul(p, &q, 10);
	if(*q != '\0')
		return -1;
	*value = u;
	return 0;
}


/* config_load_string */
static int _config_load_string(Config * config, char const * section,
		char const * variable, char ** value)
{
	char const * str;
	char * p;

	if((str = config_get(config, section, variable)) == NULL)
		return -1;
	if((p = strdup(str)) == NULL)
		return -1;
	free(*value);
	*value = p;
	return 0;
}


/* config_save_boolean */
static int _config_save_boolean(Config * config, char const * section,
		char const * variable, gboolean value)
{
	return config_set(config, section, variable, value ? "1" : "0");
}


/* config_save_integer */
static int _config_save_integer(Config * config, char const * section,
		char const * variable, unsigned int value)
{
	char buf[32];

	snprintf(buf, sizeof(buf), "%u", value);
	return config_set(config, section, variable, buf);
}


/* config_save_string */
static int _config_save_string(Config * config, char const * section,
		char const * variable, char const * value)
{
	return config_set(config, section, variable, value);
}
