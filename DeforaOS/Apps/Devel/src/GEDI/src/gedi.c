/* $Id$ */
static char const _copyright[] =
"Copyright (c) 2010 Pierre Pronchery <khorben@defora.org>";
/* This file is part of DeforaOS Devel GEDI */
/* GEDI is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * GEDI is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * GEDI; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA */



#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "gedi.h"
#include "../config.h"


/* GEDI */
/* constants */
#define ICON_NAME	"applications-development"

#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif


/* callbacks */
static void _on_exit(GtkWidget * widget, gpointer data);
static void _on_help_about(GtkWidget * widget, gpointer data);
static void _on_file_new(GtkWidget * widget, gpointer data);
static void _on_file_open(GtkWidget * widget, gpointer data);
static void _on_file_preferences(GtkWidget * widget, gpointer data);
static void _on_preferences_apply(GtkWidget * widget, gpointer data);
static void _on_preferences_cancel(GtkWidget * widget, gpointer data);
static void _on_preferences_ok(GtkWidget * widget, gpointer data);
static gboolean _on_preferences_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static void _on_project_new(GtkWidget * widget, gpointer data);
static void _on_project_open(GtkWidget * widget, gpointer data);
static void _on_project_properties(GtkWidget * widget, gpointer data);
static void _on_project_save(GtkWidget * widget, gpointer data);
static void _on_project_save_as(GtkWidget * widget, gpointer data);
static gboolean _on_closex(GtkWidget * widget, GdkEvent * event, gpointer data);
/* menus */
struct _menu {
	char * name;
	GtkSignalFunc callback;
	char * stock;
};
struct _menubar {
	char * name;
	struct _menu * menu;
};
struct _menu _menu_file[] = {
	{ "_New file...", G_CALLBACK(_on_file_new), GTK_STOCK_NEW },
	{ "_Open file...", G_CALLBACK(_on_file_open), GTK_STOCK_OPEN },
	{ "", NULL , NULL },
	{ "_Preferences...", G_CALLBACK(_on_file_preferences),
		GTK_STOCK_PREFERENCES },
	{ "", NULL, NULL },
	{ "_Exit", G_CALLBACK(_on_exit), GTK_STOCK_QUIT },
	{ NULL, NULL, NULL }
};
struct _menu _menu_projects[] = { /* FIXME will certainly be dynamic */
	{ "_New project...", G_CALLBACK(_on_project_new), GTK_STOCK_NEW },
	{ "_Open project...", G_CALLBACK(_on_project_open), GTK_STOCK_OPEN },
	{ "_Save project", G_CALLBACK(_on_project_save), GTK_STOCK_SAVE },
	{ "Save project _as...", G_CALLBACK(_on_project_save_as),
		GTK_STOCK_SAVE_AS },
	{ "", NULL, NULL },
	{ "_Properties...", G_CALLBACK(_on_project_properties),
		GTK_STOCK_PROPERTIES },
	{ NULL, NULL, NULL }
};
struct _menu _menu_help[] = {
	{ "_About", G_CALLBACK(_on_help_about), GTK_STOCK_ABOUT },
	{ NULL, NULL, NULL }
};
struct _menubar _menubar[] = {
	{ "_File", _menu_file },
	{ "_Projects", _menu_projects },
	{ "_Help", _menu_help },
	{ NULL, NULL }
};


/* gedi_new */
static void _new_config(GEDI * g);
static void _new_toolbar(GEDI * g);

GEDI * gedi_new(void)
{
	GEDI * gedi;

	if((gedi = malloc(sizeof(*gedi))) == NULL)
		return NULL;
	gedi->projects = NULL;
	gedi->project = NULL;
	_new_config(gedi);
	_new_toolbar(gedi);
	return gedi;
}

static char * _config_file(void);
static void _new_config(GEDI * g)
{
	char * filename;

	if((g->config = config_new()) == NULL)
	{
		gedi_error(g, "Could not read configuration", strerror(errno));
		return;
	}
	config_load(g->config, PREFIX "/etc/" PACKAGE ".conf");
	if((filename = _config_file()) == NULL)
		return;
	config_load(g->config, filename);
	free(filename);
}

static char * _config_file(void)
{
	char conffile[] = ".gedirc";
	char * homedir;
	char * filename;

	if((homedir = getenv("HOME")) == NULL)
		return NULL;
	if((filename = malloc(strlen(homedir) + 1 + strlen(conffile) + 1))
			== NULL)
		return NULL;
	sprintf(filename, "%s/%s", homedir, conffile);
	return filename;
}

static void _new_toolbar_menu(GEDI * g);
static void _new_toolbar(GEDI * g)
{
	g->tb_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(g->tb_window), ICON_NAME);
#endif
	gtk_window_set_title(GTK_WINDOW(g->tb_window), "GEDI");
	gtk_window_set_resizable(GTK_WINDOW(g->tb_window), FALSE);
	g_signal_connect(G_OBJECT(g->tb_window), "delete_event",
			G_CALLBACK(_on_closex), g);
	g->tb_vbox = gtk_vbox_new(FALSE, 0);
	_new_toolbar_menu(g);
	g->tb_toolbar = gtk_toolbar_new();
	gtk_toolbar_insert_stock(GTK_TOOLBAR(g->tb_toolbar), GTK_STOCK_QUIT,
			"Exit", NULL, G_CALLBACK(_on_exit), g, 0);
	gtk_box_pack_start(GTK_BOX(g->tb_vbox), g->tb_toolbar, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(g->tb_window), g->tb_vbox);
	gtk_widget_show_all(g->tb_window);
}

static void _new_toolbar_menu(GEDI * g)
{
	GtkWidget * menu;
	GtkWidget * menubar;
	GtkWidget * menuitem;
	unsigned int i;
	unsigned int j;

	g->tb_menubar = gtk_menu_bar_new();
	for(i = 0; _menubar[i].name != NULL; i++)
	{
		menubar = gtk_menu_item_new_with_mnemonic(_menubar[i].name);
		menu = gtk_menu_new();
		for(j = 0; _menubar[i].menu[j].name != NULL; j++)
		{
			if(_menubar[i].menu[j].name[0] == '\0')
				menuitem = gtk_separator_menu_item_new();
			else if(_menubar[i].menu[j].stock == 0)
				menuitem = gtk_menu_item_new_with_mnemonic(
						_menubar[i].menu[j].name);
			else
				menuitem = gtk_image_menu_item_new_from_stock(
						_menubar[i].menu[j].stock,
						NULL);
			if(_menubar[i].menu[j].callback != NULL)
				g_signal_connect(G_OBJECT(menuitem), "activate",
						G_CALLBACK(_menubar[i].menu[j].callback), g);
			else
				gtk_widget_set_sensitive(menuitem, FALSE);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		}
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(menubar), menu);
		gtk_menu_bar_append(GTK_MENU_BAR(g->tb_menubar), menubar);
	}
	gtk_box_pack_start(GTK_BOX(g->tb_vbox), g->tb_menubar, TRUE, TRUE, 0);
}


/* gedi_delete */
void gedi_delete(GEDI * gedi)
{
	char * filename;

	if((filename = _config_file()) != NULL)
	{
		config_save(gedi->config, filename);
		free(filename);
	}
	config_delete(gedi->config);
	free(gedi);
}


/* useful */
/* gedi_error */
int gedi_error(GEDI * gedi, char const * title, char const * message)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL
			| GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE, "%s", title);
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s", message);
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return 1;
}


/* gedi_file_open */
void gedi_file_open(GEDI * gedi, char const * file)
{
	/* FIXME */
}


/* gedi_project_open */
void gedi_project_open(GEDI * gedi, char const * file)
{
	/* FIXME
	 * - open project.conf file
	 * - verify it has a package name
	 * - */
}


/* gedi_project_save */
void gedi_project_save(GEDI * gedi)
{
	/* FIXME */
}


/* gedi_project_save_as */
void gedi_project_save_as(GEDI * gedi, char const * file)
{
	/* FIXME */
}


/* callbacks */
/* _on_exit */
static void _on_exit(GtkWidget * widget, gpointer data)
{
	/* FIXME check that everything is properly saved */
	gtk_main_quit();
}


/* _on_help_about */
/* callbacks */
static gboolean _on_about_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static void _on_help_about(GtkWidget * widget, gpointer data)
{
	GEDI * gedi = data;
	static GtkWidget * window = NULL; /* FIXME do not make it static */
	const char * authors[] = { "Pierre Pronchery <khorben@defora.org>",
		NULL };
	const char license[] = "GNU GPL version 2";

	if(window != NULL)
	{
		gtk_widget_show(window);
		return;
	}
	window = gtk_about_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(
				gedi->tb_window));
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(window), authors);
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(window), _copyright);
	gtk_about_dialog_set_logo_icon_name(GTK_ABOUT_DIALOG(window),
			ICON_NAME);
	gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(window), license);
	gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(window), PACKAGE);
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(window), VERSION);
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(
				_on_about_closex), NULL);
	g_signal_connect(G_OBJECT(window), "response", G_CALLBACK(
				gtk_widget_hide), NULL);
	gtk_widget_show(window);
}

static gboolean _on_about_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	gtk_widget_hide(widget);
	return TRUE;
}


/* _on_file_new */
static void _on_file_new(GtkWidget * widget, gpointer data)
{
	/* FIXME */
}


/* _on_file_open */
static void _on_file_open(GtkWidget * widget, gpointer data)
{
	GEDI * gedi = data;
	GtkWidget * dialog;
	char * file;

	dialog = gtk_file_chooser_dialog_new ("Open file...", NULL,
			GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
			GTK_RESPONSE_ACCEPT, NULL);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		gedi_file_open(gedi, file);
		g_free(file);
	}
	gtk_widget_destroy(dialog);
}


/* _on_file_preferences */
static void _file_preferences_new(GEDI * g);
static void _on_file_preferences(GtkWidget * widget, gpointer data)
{
	GEDI * g = data;

	if(g->pr_window == NULL)
		_file_preferences_new(g);
	gtk_widget_show_all(g->pr_window);
}

static void _file_preferences_new(GEDI * g)
{
	GtkWidget * vbox;
	GtkWidget * nb;
	GtkWidget * nb_vbox;
	GtkWidget * hbox;
	GtkWidget * b_ok;
	GtkWidget * b_apply;
	GtkWidget * b_cancel;

	g->pr_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(g->pr_window), 4);
	gtk_window_set_title(GTK_WINDOW(g->pr_window), "Preferences");
	g_signal_connect(G_OBJECT(g->pr_window), "delete_event",
			G_CALLBACK(_on_preferences_closex), g);
	vbox = gtk_vbox_new(FALSE, 0);
	nb = gtk_notebook_new();
	/* notebook page editor */
	nb_vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(nb_vbox), gtk_label_new("Choose editor"),
			FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(nb_vbox), gtk_label_new(
				"Program executable"), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(nb_vbox), gtk_label_new(
				"Runs in a terminal"), FALSE, FALSE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(nb), nb_vbox, gtk_label_new(
				"Editor"));
	/* notebook page plug-ins */
	nb_vbox = gtk_vbox_new(FALSE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(nb), nb_vbox, gtk_label_new(
				"Plug-ins"));
	gtk_box_pack_start(GTK_BOX(vbox), nb, TRUE, TRUE, 4);
	/* buttons */
	hbox = gtk_hbox_new(TRUE, 0);
	b_ok = gtk_button_new_from_stock(GTK_STOCK_OK);
	g_signal_connect(G_OBJECT(b_ok), "clicked", G_CALLBACK(
				_on_preferences_ok), g);
	gtk_box_pack_end(GTK_BOX(hbox), b_ok, FALSE, TRUE, 0);
	b_apply = gtk_button_new_from_stock(GTK_STOCK_APPLY);
	g_signal_connect(G_OBJECT(b_apply), "clicked", G_CALLBACK(
				_on_preferences_apply), g);
	gtk_box_pack_end(GTK_BOX(hbox), b_apply, FALSE, TRUE, 4);
	b_cancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	g_signal_connect(G_OBJECT(b_cancel), "clicked", G_CALLBACK(
				_on_preferences_cancel), g);
	gtk_box_pack_end(GTK_BOX(hbox), b_cancel, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(g->pr_window), vbox);
}


/* _on_preferences_apply */
static void _on_preferences_apply(GtkWidget * widget, gpointer data)
{
	/* FIXME */
}


/* _on_preferences_cancel */
static void _on_preferences_cancel(GtkWidget * widget, gpointer data)
{
	GEDI * gedi = data;

	/* FIXME */
	gtk_widget_hide(gedi->pr_window);
}


/* _on_preferences_ok */
static void _on_preferences_ok(GtkWidget * widget, gpointer data)
{
	GEDI * g = data;

	_on_preferences_apply(widget, data);
	gtk_widget_hide(g->pr_window);
}


/* _on_preferences_closex */
static gboolean _on_preferences_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	gtk_widget_hide(widget);
	return TRUE;
}


/* _on_project_new */
static void _on_project_new(GtkWidget * widget, gpointer data)
{
	/* FIXME */
}


/* _on_project_open */
static void _on_project_open(GtkWidget * widget, gpointer data)
{
	GEDI * g = data;
	GtkWidget * dialog;
	char * file;

	dialog = gtk_file_chooser_dialog_new("Open project...", NULL,
			GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
			GTK_RESPONSE_ACCEPT, NULL);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		gedi_project_open(g, file);
		g_free(file);
	}
	gtk_widget_destroy(dialog);
}


/* _on_project_properties */
static void _on_project_properties(GtkWidget * widget, gpointer data)
{
	GEDI * g = data;

	if(g->project == NULL)
	{
		/* FIXME should not happen (disable callback action) */
		gedi_error(g, "Error", "No project opened");
		return;
	}
	project_properties(g->project);
}


/* _on_project_save */
static void _on_project_save(GtkWidget * widget, gpointer data)
{
	GEDI * g = data;

	/* FIXME */
	gedi_project_save(g);
}


/* _on_project_save_as */
static void _on_project_save_as(GtkWidget * widget, gpointer data)
{
	GEDI * g = data;
	GtkWidget * dialog;
	char * file;

	dialog = gtk_file_chooser_dialog_new("Save project as...", NULL,
			GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
			GTK_RESPONSE_ACCEPT, NULL);
	/* FIXME add options? (recursive save) */
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		gedi_project_save_as(g, file);
		g_free(file);
	}
	gtk_widget_destroy(dialog);
}


/* _on_closex */
static gboolean _on_closex(GtkWidget * widget, GdkEvent * event, gpointer data)
{
	_on_exit(widget, data);
	return FALSE;
}
