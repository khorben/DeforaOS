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
#include "callbacks.h"
#include "gedi.h"
#include "../config.h"


/* GEDI */
/* constants */
#define ICON_NAME	"applications-development"

#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif


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
	{ "_New file...", G_CALLBACK(on_file_new), GTK_STOCK_NEW },
	{ "_Open file...", G_CALLBACK(on_file_open), GTK_STOCK_OPEN },
	{ "", NULL , NULL },
	{ "_Preferences...", G_CALLBACK(on_file_preferences),
		GTK_STOCK_PREFERENCES },
	{ "", NULL, NULL },
	{ "_Exit", G_CALLBACK(on_exit), GTK_STOCK_QUIT },
	{ NULL, NULL, NULL }
};
struct _menu _menu_projects[] = { /* FIXME will certainly be dynamic */
	{ "_New project...", G_CALLBACK(on_project_new), GTK_STOCK_NEW },
	{ "_Open project...", G_CALLBACK(on_project_open), GTK_STOCK_OPEN },
	{ "_Save project", G_CALLBACK(on_project_save), GTK_STOCK_SAVE },
	{ "Save project _as...", G_CALLBACK(on_project_save_as),
		GTK_STOCK_SAVE_AS },
	{ "", NULL, NULL },
	{ "_Properties...", G_CALLBACK(on_project_properties),
		GTK_STOCK_PROPERTIES },
	{ NULL, NULL, NULL }
};
struct _menu _menu_help[] = {
	{ "_About", G_CALLBACK(on_help_about), GTK_STOCK_ABOUT },
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
	g_signal_connect(G_OBJECT(g->tb_window), "delete-event",
			G_CALLBACK(on_closex), g);
	g->tb_vbox = gtk_vbox_new(FALSE, 0);
	_new_toolbar_menu(g);
	g->tb_toolbar = gtk_toolbar_new();
	gtk_toolbar_insert_stock(GTK_TOOLBAR(g->tb_toolbar), GTK_STOCK_QUIT,
			"Exit", NULL, G_CALLBACK(on_exit), g, 0);
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
