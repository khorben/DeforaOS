/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Surfer */
/* Surfer is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * Surfer is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Surfer; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA */



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
#define _(string) gettext(string)
#define N_(string) (string)


/* Surfer */
/* private */
/* types */
typedef struct _SurferIdle
{
	Surfer * surfer;
	char * url;
} SurferIdle;

typedef enum _SurferConsoleMessage
{
	SCM_MESSAGE = 0, SCM_SOURCE, SCM_LINE, SCM_DISPLAY_LINE
} SurferConsoleMessage;
#define SCM_LAST	SCM_DISPLAY_LINE
#define SCM_COUNT	(SCM_LAST + 1)


/* variables */
static DesktopAccel _surfer_accel[] =
{
#ifdef EMBEDDED
	/* FIXME implement the missing accelerators in embedded mode */
	{ G_CALLBACK(on_refresh), GDK_CONTROL_MASK, GDK_R },
	{ G_CALLBACK(on_close), GDK_CONTROL_MASK, GDK_W },
#endif
	{ G_CALLBACK(on_fullscreen), 0, GDK_F11 },
	{ NULL, 0, 0 }
};

#ifndef EMBEDDED
static DesktopMenu _menu_file[] =
{
	{ N_("_New tab"),	G_CALLBACK(on_file_new_tab), "tab-new",
		GDK_T },
	{ N_("_New window"),	G_CALLBACK(on_file_new_window), "window-new",
		GDK_N },
	{ N_("_Open..."),	G_CALLBACK(on_file_open), GTK_STOCK_OPEN,
		GDK_O },
	{ N_("Open _URL..."),	G_CALLBACK(on_file_open_url), NULL, GDK_L },
	{ "", NULL, NULL, 0 },
	{ N_("Save _as..."),	G_CALLBACK(on_file_save_as), GTK_STOCK_SAVE_AS,
		GDK_S },
	{ "", NULL, NULL, 0 },
	{ N_("_Print..."),	G_CALLBACK(on_file_print), GTK_STOCK_PRINT, 0 },
	{ "", NULL, NULL, 0 },
	{ N_("Close _tab"),	G_CALLBACK(on_file_close_tab), NULL, GDK_W },
	{ N_("_Close"),		G_CALLBACK(on_file_close), GTK_STOCK_CLOSE,
		0 },
	{ NULL,			NULL, NULL, 0 }
};

static DesktopMenu _menu_edit[] =
{
	{ N_("_Cut"),		NULL, GTK_STOCK_CUT, GDK_X },
	{ N_("Cop_y"),		NULL, GTK_STOCK_COPY, GDK_C },
	{ N_("_Paste"),		NULL, GTK_STOCK_PASTE, GDK_V },
	{ "",			NULL, NULL, 0 },
	{ N_("Select _all"),	G_CALLBACK(on_edit_select_all),
#if GTK_CHECK_VERSION(2, 10, 0)
		GTK_STOCK_SELECT_ALL,
#else
		NULL,
#endif
		GDK_A },
	{ N_("Unselect all"),	G_CALLBACK(on_edit_unselect_all), NULL, 0 },
	{ "",			NULL, NULL, 0 },
	{ N_("_Find"),		G_CALLBACK(on_edit_find), GTK_STOCK_FIND,
		GDK_F },
	{ "",			NULL, NULL, 0 },
	{ N_("_Preferences"),	G_CALLBACK(on_edit_preferences),
		GTK_STOCK_PREFERENCES, GDK_P },
	{ NULL,			NULL, NULL, 0 }
};

static DesktopMenu _menu_view[] =
{
	{ N_("Zoom in"),	G_CALLBACK(on_view_zoom_in), "zoom-in",
		GDK_plus },
	{ N_("Zoom out"),	G_CALLBACK(on_view_zoom_out), "zoom-out",
		GDK_minus },
	{ N_("Normal size"),	G_CALLBACK(on_view_normal_size), "zoom-1",
		GDK_0 },
	{ "",			NULL, NULL, 0 },
	{ N_("_Refresh"),	G_CALLBACK(on_view_refresh), GTK_STOCK_REFRESH,
		GDK_R },
	{ N_("_Force refresh"),	G_CALLBACK(on_view_force_refresh), NULL, 0 },
	{ N_("_Stop"),		G_CALLBACK(on_view_stop), GTK_STOCK_STOP, 0 },
	{ "",			NULL, NULL, 0 },
	{ N_("Page so_urce"),	G_CALLBACK(on_view_page_source),
		"stock_view-html-source", GDK_U },
	{ N_("Javascript _console"), G_CALLBACK(on_view_javascript_console),
		NULL, 0 },
	{ NULL,			NULL, NULL, 0 }
};

static DesktopMenu _menu_help[] =
{
	{ "_About",		G_CALLBACK(on_help_about),
#if GTK_CHECK_VERSION(2, 6, 0)
		GTK_STOCK_ABOUT, 0 },
#else
		NULL, 0 },
#endif
	{ NULL,			NULL, NULL, 0 }
};

static DesktopMenubar _surfer_menubar[] =
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
	{ "Back", G_CALLBACK(on_back), GTK_STOCK_GO_BACK, 0, NULL },
	{ "Forward", G_CALLBACK(on_forward), GTK_STOCK_GO_FORWARD, 0, NULL },
	{ "Stop", G_CALLBACK(on_stop), GTK_STOCK_STOP, 0, NULL },
	{ "Refresh", G_CALLBACK(on_refresh), GTK_STOCK_REFRESH, 0, NULL },
	{ "", NULL, NULL, 0, NULL },
	{ "Home", G_CALLBACK(on_home), GTK_STOCK_HOME, 0, NULL },
	{ NULL, NULL, NULL, 0, NULL }
};

unsigned int surfer_cnt = 0;


/* prototypes */
static char * _config_get_filename(void);
static int _config_load_string(Config * config, char const * variable,
		char ** value);
static int _config_save_string(Config * config, char const * variable,
		char const * value);


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

	if((surfer = malloc(sizeof(*surfer))) == NULL)
		return NULL;
	surfer->homepage = NULL;
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
	gtk_window_set_default_size(GTK_WINDOW(surfer->window), 800, 600);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(surfer->window), "stock_internet");
#endif
	gtk_window_set_title(GTK_WINDOW(surfer->window), _("Web surfer"));
	g_signal_connect_swapped(G_OBJECT(surfer->window), "delete-event",
			G_CALLBACK(on_closex), surfer);
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
#if GTK_CHECK_VERSION(2, 8, 0)
	toolitem = gtk_toggle_tool_button_new_from_stock(GTK_STOCK_FULLSCREEN);
#else
	toolitem = gtk_toggle_tool_button_new_from_stock(GTK_STOCK_ZOOM_FIT);
#endif
	surfer->tb_fullscreen = toolitem;
	g_signal_connect_swapped(G_OBJECT(toolitem), "toggled", G_CALLBACK(
				on_fullscreen), surfer);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
#ifdef EMBEDDED
	toolitem = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_PREFERENCES);
	g_signal_connect_swapped(G_OBJECT(toolitem), "clicked", G_CALLBACK(
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
	surfer->lb_path = gtk_combo_box_entry_new_text();
	widget = gtk_bin_get_child(GTK_BIN(surfer->lb_path));
	g_signal_connect_swapped(G_OBJECT(widget), "activate", G_CALLBACK(
				on_path_activate), surfer);
	if(url != NULL)
		gtk_entry_set_text(GTK_ENTRY(widget), url);
	gtk_tool_item_set_expand(toolitem, TRUE);
	gtk_container_add(GTK_CONTAINER(toolitem), surfer->lb_path);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_JUMP_TO);
	g_signal_connect_swapped(G_OBJECT(toolitem), "clicked", G_CALLBACK(
				on_path_activate), surfer);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
	/* notebook */
	surfer->notebook = gtk_notebook_new();
	gtk_notebook_set_show_border(GTK_NOTEBOOK(surfer->notebook), FALSE);
	surfer_open_tab(surfer, NULL);
	g_signal_connect_swapped(G_OBJECT(surfer->notebook), "switch-page",
			G_CALLBACK(on_notebook_switch_page), surfer);
	gtk_box_pack_start(GTK_BOX(vbox), surfer->notebook, TRUE, TRUE, 0);
	/* statusbar */
	surfer->statusbox = gtk_hbox_new(FALSE, 0);
	surfer->progress = gtk_progress_bar_new();
	gtk_box_pack_start(GTK_BOX(surfer->statusbox), surfer->progress, FALSE,
			FALSE, 0);
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
	/* hack to display the statusbar only if necessary */
	gtk_box_pack_start(GTK_BOX(vbox), surfer->statusbox, FALSE, FALSE, 0);
	surfer_set_status(surfer, NULL);
	surfer_cnt++;
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
	free(surfer->homepage);
	free(surfer);
	if(--surfer_cnt == 0)
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


/* surfer_set_fullscreen */
void surfer_set_fullscreen(Surfer * surfer, gboolean fullscreen)
{
	if(fullscreen == TRUE)
		gtk_window_fullscreen(GTK_WINDOW(surfer->window));
	else
		gtk_window_unfullscreen(GTK_WINDOW(surfer->window));
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
		gtk_combo_box_remove_text(GTK_COMBO_BOX(surfer->lb_path), 0);
	else
		i++;
	gtk_combo_box_append_text(GTK_COMBO_BOX(surfer->lb_path), url);
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
	if(fraction >= 0.0 && fraction <= 1.0)
		snprintf(buf, sizeof(buf), "%.1f%%", fraction * 100);
	else
		fraction = 0.0;
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(surfer->progress), buf);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(surfer->progress),
			fraction);
}


/* surfer_set_status */
void surfer_set_status(Surfer * surfer, char const * status)
{
	GtkWidget * view;
	GtkStatusbar * sb = GTK_STATUSBAR(surfer->statusbar);
	GtkProgressBar * pb = GTK_PROGRESS_BAR(surfer->progress);

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
		gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_stop), FALSE);
	if(status == NULL)
	{
		gtk_progress_bar_set_text(pb, " ");
		gtk_progress_bar_set_fraction(pb, 0.0);
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


/* useful */
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

	if((filename = _config_get_filename()) == NULL)
		return 1;
	config_load(surfer->config, filename); /* XXX ignore errors */
	free(filename);
	_config_load_string(surfer->config, "homepage", &surfer->homepage);
	return 0;
}


/* surfer_config_save */
int surfer_config_save(Surfer * surfer)
{
	int ret = 0;
	char * filename;

	if((filename = _config_get_filename()) == NULL)
		return 1;
	ret |= _config_save_string(surfer->config, "homepage",
			surfer->homepage);
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


/* surfer_download */
void surfer_download(Surfer * surfer, char const * url, char const * suggested)
{
	GtkWidget * dialog;
	char * filename = NULL;
	char * argv[] = { "download", "-O", NULL, NULL, NULL };
	GError * error = NULL;

	dialog = gtk_file_chooser_dialog_new(_("Save file as..."),
			GTK_WINDOW(surfer->window),
			GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE,
			GTK_RESPONSE_ACCEPT, NULL);
	if(suggested != NULL) /* XXX also suggest a name otherwise */
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog),
				suggested);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
					dialog));
	gtk_widget_destroy(dialog);
	if(filename == NULL)
		return;
	argv[2] = filename;
	argv[3] = strdup(url); /* XXX may fail */
	g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL,
			&error);
	free(argv[3]);
	g_free(filename);
}


/* surfer_error */
int surfer_error(Surfer * surfer, char const * message, int ret)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new((surfer != NULL)
			? GTK_WINDOW(surfer->window) : NULL,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", _("Error"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
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
	gtk_entry_set_text(GTK_ENTRY(surfer->fi_text), (text != NULL) ? text
			: "");
	gtk_widget_show(surfer->fi_dialog);
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
	vbox = GTK_DIALOG(surfer->fi_dialog)->vbox;
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new(_("Text:"));
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	surfer->fi_text = gtk_entry_new();
	g_signal_connect(G_OBJECT(surfer->fi_text), "activate", G_CALLBACK(
				_on_find_activate), surfer);
	gtk_box_pack_start(GTK_BOX(hbox), surfer->fi_text, TRUE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 4);
	surfer->fi_case = gtk_check_button_new_with_label(_("Case-sensitive"));
	gtk_box_pack_start(GTK_BOX(vbox), surfer->fi_case, TRUE, TRUE, 4);
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
	gboolean wrap;

	if((view = surfer_get_view(surfer)) == NULL)
		return;
	if((text = gtk_entry_get_text(GTK_ENTRY(widget))) == NULL
			|| strlen(text) == 0)
		return;
	sensitive = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
				surfer->fi_case));
	wrap = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
				surfer->fi_wrap));
	if(ghtml_find(view, text, sensitive, wrap) == TRUE)
		return;
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

	if((homepage = config_get(surfer->config, "", "homepage")) == NULL)
		homepage = SURFER_DEFAULT_HOME;
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
	if(url != NULL)
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
	char * filename = NULL;

	dialog = gtk_file_chooser_dialog_new(_("Open file..."),
			GTK_WINDOW(surfer->window),
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
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

	if((widget = ghtml_new(surfer)) == NULL)
	{
		surfer_error(NULL, _("Could not initialize HTML renderer"), 0);
		return;
	}
	if(url != NULL)
		ghtml_load_url(widget, url);
	gtk_notebook_append_page(GTK_NOTEBOOK(surfer->notebook), widget,
			_tab_button(surfer, widget, _("Untitled")));
	if(gtk_notebook_get_n_pages(GTK_NOTEBOOK(surfer->notebook)) > 1)
		gtk_notebook_set_show_tabs(GTK_NOTEBOOK(surfer->notebook),
				TRUE);
	else
		gtk_notebook_set_show_tabs(GTK_NOTEBOOK(surfer->notebook),
				FALSE);
	gtk_widget_show_all(widget);
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
			GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL, "%s",
			_("Question"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s", message);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Question"));
	vbox = GTK_DIALOG(dialog)->vbox;
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
	GtkWidget * view;

	if((view = surfer_get_view(surfer)) == NULL)
		return;
	ghtml_select_all(view);
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
	g_signal_connect_swapped(G_OBJECT(surfer->co_window), "delete-event",
			G_CALLBACK(on_console_closex), surfer);
	vbox = gtk_vbox_new(FALSE, 0);
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new(_("Command:"));
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	surfer->co_entry = gtk_entry_new();
	g_signal_connect_swapped(G_OBJECT(surfer->co_entry), "activate",
			G_CALLBACK(on_console_execute), surfer);
	gtk_box_pack_start(GTK_BOX(hbox), surfer->co_entry, TRUE, TRUE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_EXECUTE);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				on_console_execute), surfer);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 4);
	/* view */
	hbox = gtk_scrolled_window_new(NULL, NULL);
	widget = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				surfer->co_store));
	/* message */
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Message",
			renderer, "text", SCM_MESSAGE, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	/* source */
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Source",
			renderer, "text", SCM_SOURCE, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	/* line */
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Line",
			renderer, "text", SCM_DISPLAY_LINE, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	gtk_container_add(GTK_CONTAINER(hbox), widget);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
	/* dialog */
	hbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbox), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing(GTK_BUTTON_BOX(hbox), 4);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_CLEAR);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				on_console_clear), surfer);
	gtk_container_add(GTK_CONTAINER(hbox), widget);
	widget = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
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


/* surfer_unselect_all */
void surfer_unselect_all(Surfer * surfer)
{
	GtkWidget * view;

	if((view = surfer_get_view(surfer)) == NULL)
		return;
	ghtml_unselect_all(view);
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

	if((widget = surfer_get_view(surfer)) == NULL)
		return;
	if((url = ghtml_get_location(widget)) == NULL)
		return;
	if((source = ghtml_get_source(widget)) == NULL)
		return; /* FIXME download to a temporary file and open */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	group = gtk_accel_group_new();
	cc = g_cclosure_new_swap(G_CALLBACK(_on_source_close), window, NULL);
	gtk_accel_group_connect(group, GDK_W, GDK_CONTROL_MASK,
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
	/* FIXME probably should convert to UTF-8 when not already */
	gtk_text_buffer_set_text(tbuf, source, strlen(source));
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
			GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "%s",
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
}


/* surfer_zoom_out */
void surfer_zoom_out(Surfer * surfer)
{
	GtkWidget * view;

	if((view = surfer_get_view(surfer)) == NULL)
		return;
	ghtml_zoom_out(view);
}


/* surfer_zoom_reset */
void surfer_zoom_reset(Surfer * surfer)
{
	GtkWidget * view;

	if((view = surfer_get_view(surfer)) == NULL)
		return;
	ghtml_zoom_reset(view);
}


/* private */
/* functions */
static char * _config_get_filename(void)
{
	char const * homedir;
	size_t len;
	char * filename;

	if((homedir = getenv("HOME")) == NULL)
		homedir = g_get_home_dir();
	len = strlen(homedir) + 1 + sizeof(SURFER_CONFIG_FILE);
	if((filename = malloc(len)) == NULL)
		return NULL;
	snprintf(filename, len, "%s/%s", homedir, SURFER_CONFIG_FILE);
	return filename;
}


static int _config_load_string(Config * config, char const * variable,
		char ** value)
{
	char const * str;
	char * p;

	if((str = config_get(config, "", variable)) == NULL)
		return 0;
	if((p = strdup(str)) == NULL)
		return 1;
	free(*value);
	*value = p;
	return 0;
}


static int _config_save_string(Config * config, char const * variable,
		char const * value)
{
	return config_set(config, "", variable, value);
}
