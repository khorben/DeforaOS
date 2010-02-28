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
#include <gdk/gdkkeysyms.h>
#include <Desktop.h>
#include "callbacks.h"
#include "ghtml.h"
#include "surfer.h"
#include "common.h"


/* Surfer */
/* private */
/* variables */
#ifdef EMBEDDED
static DesktopAccel _surfer_accel[] =
{
	/* FIXME implement the missing accelerators in embedded mode */
	{ G_CALLBACK(on_refresh), GDK_CONTROL_MASK, GDK_R },
	{ NULL, 0, 0 }
};
#endif

#ifndef EMBEDDED
static DesktopMenu _menu_file[] =
{
	{ "_New window",	G_CALLBACK(on_file_new_window), "window-new",
		GDK_N },
	{ "_Open...",		G_CALLBACK(on_file_open), GTK_STOCK_OPEN,
		GDK_O },
	{ "_Open URL...",	G_CALLBACK(on_file_open_url), NULL, GDK_L },
	{ "", NULL, NULL, 0 },
	{ "_Close",		G_CALLBACK(on_file_close), GTK_STOCK_CLOSE,
		GDK_W },
	{ NULL,			NULL, NULL, 0 }
};

static DesktopMenu _menu_edit[] =
{
	{ "_Cut",		NULL, GTK_STOCK_CUT, GDK_X },
	{ "Cop_y",		NULL, GTK_STOCK_COPY, GDK_C },
	{ "_Paste",		NULL, GTK_STOCK_PASTE, GDK_V },
	{ "",			NULL, NULL, 0 },
	{ "Select _all",	G_CALLBACK(on_edit_select_all),
#if GTK_CHECK_VERSION(2, 10, 0)
		GTK_STOCK_SELECT_ALL,
#else
		NULL,
#endif
		GDK_A },
	{ "Unselect all",	G_CALLBACK(on_edit_unselect_all), NULL, 0 },
	{ "",			NULL, NULL, 0 },
	{ "_Preferences",	G_CALLBACK(on_edit_preferences),
		GTK_STOCK_PREFERENCES, GDK_P },
	{ NULL,			NULL, NULL, 0 }
};

static DesktopMenu _menu_view[] =
{
	{ "Zoom in",		G_CALLBACK(on_view_zoom_in), "zoom-in",
		GDK_plus },
	{ "Zoom out",		G_CALLBACK(on_view_zoom_out), "zoom-out",
		GDK_minus },
	{ "Normal size",	G_CALLBACK(on_view_normal_size), "zoom-1",
		GDK_0 },
	{ "",			NULL, NULL, 0 },
	{ "_Refresh",		G_CALLBACK(on_view_refresh), GTK_STOCK_REFRESH,
		GDK_R },
	{ "_Force refresh",	G_CALLBACK(on_view_force_refresh), NULL, 0 },
	{ "_Stop",		G_CALLBACK(on_view_stop), GTK_STOCK_STOP, 0 },
	{ "",			NULL, NULL, 0 },
	{ "Page so_urce",	G_CALLBACK(on_view_page_source),
		"stock_view-html-source", GDK_U },
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
	{ "_File", _menu_file },
	{ "_Edit", _menu_edit },
	{ "_View", _menu_view },
	{ "_Help", _menu_help },
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
static gboolean _new_idle(gpointer data);

Surfer * surfer_new(char const * url)
{
	Surfer * surfer;
	GtkAccelGroup * group;
	GtkWidget * vbox;
	GtkWidget * toolbar;
	GtkToolItem * toolitem;
	GtkWidget * widget;

	if((surfer = malloc(sizeof(*surfer))) == NULL)
		return NULL;
	surfer->url = NULL;
	surfer->homepage = NULL;
	if((surfer->config = config_new()) == NULL
			|| surfer_config_load(surfer) != 0
			|| (url != NULL && (surfer->url = strdup(url)) == NULL))
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
	surfer_set_title(surfer, NULL);
	g_signal_connect(G_OBJECT(surfer->window), "delete_event", G_CALLBACK(
				on_closex), surfer);
	vbox = gtk_vbox_new(FALSE, 0);
#ifndef EMBEDDED
	/* menubar */
	surfer->menubar = desktop_menubar_create(_surfer_menubar, surfer,
			group);
	gtk_box_pack_start(GTK_BOX(vbox), surfer->menubar, FALSE, FALSE, 0);
#else
	desktop_accel_create(_surfer_accel, surfer, group);
#endif
	/* toolbar */
	toolbar = desktop_toolbar_create(_surfer_toolbar, surfer, group);
	surfer->toolbar = toolbar;
	surfer->tb_back = _surfer_toolbar[0].widget;
	surfer->tb_forward = _surfer_toolbar[1].widget;
	surfer->tb_stop = _surfer_toolbar[2].widget;
	surfer->tb_refresh = _surfer_toolbar[3].widget;
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_back), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_forward), FALSE);
#if GTK_CHECK_VERSION(2, 8, 0)
	toolitem = gtk_toggle_tool_button_new_from_stock(GTK_STOCK_FULLSCREEN);
#else
	toolitem = gtk_toggle_tool_button_new_from_stock(GTK_STOCK_ZOOM_FIT);
#endif
	g_signal_connect(G_OBJECT(toolitem), "toggled", G_CALLBACK(
				on_fullscreen), surfer);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, TRUE, 0);
	/* toolbar */
	toolbar = gtk_toolbar_new();
	gtk_toolbar_set_icon_size(GTK_TOOLBAR(toolbar),
			GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
#ifndef EMBEDDED
	toolitem = gtk_tool_item_new();
	widget = gtk_label_new(" Location: ");
	gtk_container_add(GTK_CONTAINER(toolitem), widget);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
#endif
	toolitem = gtk_tool_item_new();
	surfer->tb_path = gtk_combo_box_entry_new_text();
	widget = gtk_bin_get_child(GTK_BIN(surfer->tb_path));
	g_signal_connect_swapped(G_OBJECT(widget), "activate", G_CALLBACK(
				on_path_activate), surfer);
	if(url != NULL)
		gtk_entry_set_text(GTK_ENTRY(widget), url);
	gtk_tool_item_set_expand(toolitem, TRUE);
	gtk_container_add(GTK_CONTAINER(toolitem), surfer->tb_path);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_JUMP_TO);
	g_signal_connect_swapped(G_OBJECT(toolitem), "clicked", G_CALLBACK(
				on_path_activate), surfer);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
	/* view */
	if((surfer->view = ghtml_new(surfer)) == NULL)
	{
		surfer_error(NULL, "Could not initialize HTML renderer", 0);
		surfer_delete(surfer);
		return NULL;
	}
	gtk_box_pack_start(GTK_BOX(vbox), surfer->view, TRUE, TRUE, 0);
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
	gtk_widget_grab_focus(GTK_WIDGET(surfer->tb_path));
	gtk_widget_show_all(surfer->window);
	/* preferences window */
	surfer->pr_window = NULL;
	/* hack to display the statusbar only if necessary */
	gtk_box_pack_start(GTK_BOX(vbox), surfer->statusbox, FALSE, FALSE, 0);
	surfer_set_status(surfer, NULL);
	surfer_cnt++;
	/* load url as soon as we're idle */
	/* FIXME this apparently breaks surfer_new_copy() */
	if(url != NULL)
		g_idle_add(_new_idle, surfer);
	return surfer;
}

static gboolean _new_idle(gpointer data)
{
	Surfer * surfer = data;

	ghtml_load_url(surfer->view, surfer->url);
	return FALSE;
}


/* surfer_new_copy */
Surfer * surfer_new_copy(Surfer * surfer)
{
	Surfer * ret;

	if((ret = surfer_new(surfer->url)) == NULL)
		return NULL;
	/* FIXME also copy history */
	return ret;
}


/* surfer_delete */
void surfer_delete(Surfer * surfer)
{
	gtk_widget_destroy(surfer->window);
	config_delete(surfer->config);
	free(surfer->url);
	free(surfer->homepage);
	free(surfer);
	surfer_cnt--;
}


/* accessors */
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

	widget = gtk_bin_get_child(GTK_BIN(surfer->tb_path));
	gtk_entry_set_text(GTK_ENTRY(widget), url);
	/* FIXME also set surfer->url? what about history? */
	if(i == 8)
		gtk_combo_box_remove_text(GTK_COMBO_BOX(surfer->tb_path), 0);
	else
		i++;
	gtk_combo_box_append_text(GTK_COMBO_BOX(surfer->tb_path), url);
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_back),
			ghtml_can_go_back(surfer->view));
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_forward),
			ghtml_can_go_forward(surfer->view));
}


/* surfer_set_progress */
void surfer_set_progress(Surfer * surfer, gdouble fraction)
{
	char buf[10] = " ";

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
	GtkStatusbar * sb = GTK_STATUSBAR(surfer->statusbar);
	GtkProgressBar * pb = GTK_PROGRESS_BAR(surfer->progress);

	if(surfer->statusbar_id != 0)
		gtk_statusbar_remove(sb, gtk_statusbar_get_context_id(sb, ""),
				surfer->statusbar_id);
	surfer->statusbar_id = gtk_statusbar_push(sb,
			gtk_statusbar_get_context_id(sb, ""), (status != NULL)
			? status : "Ready");
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
	char buf[256];

	snprintf(buf, sizeof(buf), "%s%s%s", "Web surfer", title != NULL
			? " - " : "", title != NULL ? title : "");
	gtk_window_set_title(GTK_WINDOW(surfer->window), buf);
}


/* useful */
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
int surfer_confirm(Surfer * surfer, char const * message)
{
	int ret;
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new((surfer != NULL)
			? GTK_WINDOW(surfer->window) : NULL,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "%s",
			message);
	gtk_window_set_title(GTK_WINDOW(dialog), "Question");
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(
				gtk_widget_destroy), NULL);
	ret = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return (ret == GTK_RESPONSE_YES) ? 1 : 0;
}


/* surfer_download */
void surfer_download(Surfer * surfer, char const * url, char const * suggested)
{
	GtkWidget * dialog;
	char * filename = NULL;
	char * argv[] = { "download", "-O", NULL, NULL, NULL };
	GError * error = NULL;

	dialog = gtk_file_chooser_dialog_new("Save file as...",
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
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", message);
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(
				gtk_widget_destroy), NULL);
	gtk_widget_show(dialog);
	return ret;
}


/* surfer_go_back */
gboolean surfer_go_back(Surfer * surfer)
{
	gboolean ret;

	ret = ghtml_go_back(surfer->view);
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_back), ret);
	return ret;
}


/* surfer_go_forward */
gboolean surfer_go_forward(Surfer * surfer)
{
	gboolean ret;

	ret = ghtml_go_forward(surfer->view);
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
	if(url != NULL)
	{
		ghtml_stop(surfer->view);
		ghtml_load_url(surfer->view, url);
	}
	else
		gtk_widget_grab_focus(surfer->tb_path);
}


/* surfer_open_dialog */
void surfer_open_dialog(Surfer * surfer)
{
	GtkWidget * dialog;
	char * filename = NULL;

	dialog = gtk_file_chooser_dialog_new("Open file...",
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


/* surfer_refresh */
void surfer_refresh(Surfer * surfer)
{
	ghtml_refresh(surfer->view);
}


/* surfer_reload */
void surfer_reload(Surfer * surfer)
{
	ghtml_reload(surfer->view);
}


/* surfer_resize */
void surfer_resize(Surfer * surfer, gint width, gint height)
{
	gtk_window_resize(GTK_WINDOW(surfer->window), width, height);
}


/* surfer_select_all */
void surfer_select_all(Surfer * surfer)
{
	ghtml_select_all(surfer->view);
}


/* surfer_show_menubar */
void surfer_show_menubar(Surfer * surfer, gboolean show)
{
	if(show == TRUE)
		gtk_widget_show(surfer->menubar);
	else
		gtk_widget_hide(surfer->menubar);
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
		gtk_widget_show(surfer->toolbar);
	else
		gtk_widget_hide(surfer->toolbar);
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
	ghtml_stop(surfer->view);
}


/* surfer_unselect_all */
void surfer_unselect_all(Surfer * surfer)
{
	ghtml_unselect_all(surfer->view);
}


/* surfer_warning */
void surfer_warning(Surfer * surfer, char const * message)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new((surfer != NULL)
			? GTK_WINDOW(surfer->window) : NULL,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "%s", message);
	gtk_window_set_title(GTK_WINDOW(dialog), "Warning");
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(
				gtk_widget_destroy), NULL);
	gtk_widget_show(dialog);
}


/* surfer_zoom_in */
void surfer_zoom_in(Surfer * surfer)
{
	ghtml_zoom_in(surfer->view);
}


/* surfer_zoom_out */
void surfer_zoom_out(Surfer * surfer)
{
	ghtml_zoom_out(surfer->view);
}


/* surfer_zoom_reset */
void surfer_zoom_reset(Surfer * surfer)
{
	ghtml_zoom_reset(surfer->view);
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
