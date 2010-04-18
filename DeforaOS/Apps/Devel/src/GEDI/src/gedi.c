/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel GEDI */
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



#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <gdk/gdkkeysyms.h>
#include <Desktop.h>
#include "callbacks.h"
#include "gedi.h"
#include "../config.h"


/* GEDI */
/* private */
/* constants */
#define ICON_NAME	"applications-development"

#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif


/* variables */
/* menubar */
static DesktopMenu _gedi_menu_file[] =
{
	{ "_New file...", G_CALLBACK(on_file_new), GTK_STOCK_NEW, GDK_N },
	{ "_Open file...", G_CALLBACK(on_file_open), GTK_STOCK_OPEN, GDK_O },
	{ "", NULL , NULL, 0 },
	{ "_Preferences...", G_CALLBACK(on_file_preferences),
		GTK_STOCK_PREFERENCES, GDK_P },
	{ "", NULL, NULL, 0 },
	{ "_Exit", G_CALLBACK(on_exit), GTK_STOCK_QUIT, GDK_Q },
	{ NULL, NULL, NULL, 0 }
};

static DesktopMenu _gedi_menu_projects[] = /* FIXME will certainly be dynamic */
{
	{ "_New project...", G_CALLBACK(on_project_new), GTK_STOCK_NEW, 0 },
	{ "_Open project...", G_CALLBACK(on_project_open), GTK_STOCK_OPEN, 0 },
	{ "_Save project", G_CALLBACK(on_project_save), GTK_STOCK_SAVE, GDK_S },
	{ "Save project _as...", G_CALLBACK(on_project_save_as),
		GTK_STOCK_SAVE_AS, 0 },
	{ "", NULL, NULL, 0 },
	{ "_Properties...", G_CALLBACK(on_project_properties),
		GTK_STOCK_PROPERTIES, 0 },
	{ NULL, NULL, NULL, 0 }
};

static DesktopMenu _gedi_menu_help[] =
{
	{ "_About", G_CALLBACK(on_help_about), GTK_STOCK_ABOUT, 0 },
	{ NULL, NULL, NULL, 0 }
};

static DesktopMenubar _gedi_menubar[] =
{
	{ "_File",	_gedi_menu_file },
	{ "_Projects",	_gedi_menu_projects },
	{ "_Help",	_gedi_menu_help },
	{ NULL,		NULL }
};

/* toolbar */
static DesktopToolbar _gedi_toolbar[] =
{
	{ "Exit", G_CALLBACK(on_exit), GTK_STOCK_QUIT, 0, NULL },
	{ NULL, NULL, NULL, 0, NULL }
};


/* public */
/* functions */
/* gedi_new */
static void _new_config(GEDI * g);

GEDI * gedi_new(void)
{
	GEDI * gedi;
	GtkAccelGroup * group;
	GtkWidget * widget;

	if((gedi = malloc(sizeof(*gedi))) == NULL)
		return NULL;
	gedi->projects = NULL;
	gedi->project = NULL;
	_new_config(gedi);
	/* window */
	group = gtk_accel_group_new();
	gedi->tb_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_add_accel_group(GTK_WINDOW(gedi->tb_window), group);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(gedi->tb_window), ICON_NAME);
#endif
	gtk_window_set_title(GTK_WINDOW(gedi->tb_window), "GEDI");
	gtk_window_set_resizable(GTK_WINDOW(gedi->tb_window), FALSE);
	g_signal_connect(G_OBJECT(gedi->tb_window), "delete-event", G_CALLBACK(
				on_closex), gedi);
	gedi->tb_vbox = gtk_vbox_new(FALSE, 0);
	/* menubar */
	widget = desktop_menubar_create(_gedi_menubar, gedi, group);
	gtk_box_pack_start(GTK_BOX(gedi->tb_vbox), widget, FALSE, TRUE, 0);
	/* toolbar */
	widget = desktop_toolbar_create(_gedi_toolbar, gedi, group);
	gtk_box_pack_start(GTK_BOX(gedi->tb_vbox), widget, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(gedi->tb_window), gedi->tb_vbox);
	gtk_widget_show_all(gedi->tb_window);
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
	char const conffile[] = ".gedirc";
	char const * homedir;
	char * filename;

	if((homedir = getenv("HOME")) == NULL)
		return NULL;
	if((filename = malloc(strlen(homedir) + 1 + strlen(conffile) + 1))
			== NULL)
		return NULL;
	sprintf(filename, "%s/%s", homedir, conffile);
	return filename;
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
