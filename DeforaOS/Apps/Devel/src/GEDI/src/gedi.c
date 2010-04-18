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
	{ "_Exit", G_CALLBACK(on_file_exit), GTK_STOCK_QUIT, GDK_Q },
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
	{ "Exit", G_CALLBACK(on_file_exit), GTK_STOCK_QUIT, 0, NULL },
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
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;

	if((gedi = malloc(sizeof(*gedi))) == NULL)
		return NULL;
	gedi->projects = NULL;
	gedi->projects_cnt = 0;
	gedi->cur = NULL;
	_new_config(gedi);
	/* main window */
	group = gtk_accel_group_new();
	gedi->tb_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_add_accel_group(GTK_WINDOW(gedi->tb_window), group);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(gedi->tb_window), ICON_NAME);
#endif
	gtk_window_set_title(GTK_WINDOW(gedi->tb_window), "GEDI");
	gtk_window_set_resizable(GTK_WINDOW(gedi->tb_window), FALSE);
	g_signal_connect_swapped(G_OBJECT(gedi->tb_window), "delete-event",
			G_CALLBACK(on_closex), gedi);
	vbox = gtk_vbox_new(FALSE, 0);
	/* menubar */
	widget = desktop_menubar_create(_gedi_menubar, gedi, group);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	/* toolbar */
	widget = desktop_toolbar_create(_gedi_toolbar, gedi, group);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(gedi->tb_window), vbox);
	/* files */
	gedi->fi_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(gedi->fi_window), 150, 200);
	gtk_window_set_title(GTK_WINDOW(gedi->fi_window), "Files");
	hbox = gtk_hbox_new(FALSE, 0);
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 2);
	gedi->fi_combo = gtk_combo_box_new_text();
	gtk_box_pack_start(GTK_BOX(vbox), gedi->fi_combo, FALSE, TRUE, 2);
	gedi->fi_view = gtk_tree_view_new();
	gtk_box_pack_start(GTK_BOX(vbox), gedi->fi_view, TRUE, TRUE, 2);
	gtk_container_add(GTK_CONTAINER(gedi->fi_window), hbox);
	gtk_widget_show_all(gedi->tb_window);
	gtk_widget_show_all(gedi->fi_window);
	return gedi;
}

static char * _config_file(void);
static void _new_config(GEDI * gedi)
{
	char * filename;

	if((gedi->config = config_new()) == NULL)
	{
		gedi_error(gedi, strerror(errno), 0);
		return;
	}
	config_load(gedi->config, PREFIX "/etc/" PACKAGE ".conf");
	if((filename = _config_file()) == NULL)
		return;
	config_load(gedi->config, filename);
	free(filename);
}

static char * _config_file(void)
{
	char const conffile[] = ".gedirc";
	char const * homedir;
	size_t len;
	char * filename;

	if((homedir = getenv("HOME")) == NULL)
		return NULL;
	len = strlen(homedir) + 1 + strlen(conffile) + 1;
	if((filename = malloc(len)) == NULL)
		return NULL;
	snprintf(filename, len, "%s/%s", homedir, conffile);
	return filename;
}


/* gedi_delete */
void gedi_delete(GEDI * gedi)
{
	char * filename;
	size_t i;

	if((filename = _config_file()) != NULL)
	{
		config_save(gedi->config, filename);
		free(filename);
	}
	config_delete(gedi->config);
	for(i = 0; i < gedi->projects_cnt; i++)
		project_delete(gedi->projects[i]);
	free(gedi->projects);
	free(gedi);
}


/* useful */
/* gedi_error */
int gedi_error(GEDI * gedi, char const * message, int ret)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL
			| GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE, "%s",
#if GTK_CHECK_VERSION(2, 6, 0)
			"Error");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s",
#endif
			message);
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return ret;
}


/* gedi_file_open */
void gedi_file_open(GEDI * gedi, char const * file)
{
	/* FIXME */
}


/* gedi_project_open */
int gedi_project_open(GEDI * gedi, char const * file)
{
	Project ** p;

	if((p = realloc(gedi->projects, sizeof(*p) * (gedi->projects_cnt + 1)))
			== NULL)
		return gedi_error(gedi, strerror(errno), 1);
	gedi->projects = p;
	p = &gedi->projects[gedi->projects_cnt];
	if((*p = project_new()) == NULL)
		return gedi_error(gedi, error_get(), 1);
	if(project_load(*p, file) != 0)
	{
		project_delete(*p);
		return gedi_error(gedi, error_get(), 1);
	}
	gedi->projects_cnt++;
	gedi->cur = *p;
	gtk_combo_box_append_text(GTK_COMBO_BOX(gedi->fi_combo),
			project_get_package(*p));
	/* FIXME doesn't always select the last project opened */
	gtk_combo_box_set_active(GTK_COMBO_BOX(gedi->fi_combo),
			gtk_combo_box_get_active(gedi->fi_combo) + 1);
	return 0;
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
