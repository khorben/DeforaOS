/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Panel */
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
/* TODO:
 * - implement all remaining helpers */



#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <gtk/gtk.h>
#include <System.h>
#include "Panel.h"
#include "panel.h"
#include "../config.h"


/* constants */
#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif


/* private */
/* types */
struct _Panel
{
	Config * config;
	GtkWidget * window;
};


/* prototypes */
static int _test(char * applets[]);
static int _test_list(void);
static char * _config_get_filename(void);
static int _error(char const * message, int ret);
static int _usage(void);


/* functions */
static char const * _helper_config_get(Panel * panel, char const * section,
		char const * variable);
static int _helper_error(Panel * panel, char const * message, int ret);
static void _helper_position_menu(Panel * panel, GtkMenu * menu, gint * x,
		gint * y, gboolean * push_in);

static int _test(char * applets[])
{
	Panel panel;
	char * filename;
	char const path[] = PREFIX "/lib/Panel/applets/";
	char const so[] = ".so";
	GtkWidget * box;
	GtkWidget * widget;
	size_t i;
	size_t len;
	char * p = NULL;
	char * q;
	void * dl;
	PanelAppletHelper helper;
	PanelApplet * pa;

	if((panel.config = config_new()) == NULL)
		return error_print("panel_test");
	if((filename = _config_get_filename()) != NULL
			&& config_load(panel.config, filename) != 0)
		error_print("panel_test");
	free(filename);
	panel.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(panel.window), "delete-event", G_CALLBACK(
				gtk_main_quit), NULL);
	gtk_window_set_title(GTK_WINDOW(panel.window), "Applet tester");
	box = gtk_hbox_new(FALSE, 4);
	memset(&helper, 0, sizeof(helper));
	helper.panel = &panel;
	helper.icon_size = GTK_ICON_SIZE_SMALL_TOOLBAR;
	helper.config_get = _helper_config_get;
	helper.error = _helper_error;
	helper.position_menu = _helper_position_menu;
	for(i = 0; applets[i] != NULL; i++)
	{
		len = sizeof(path) + strlen(applets[i]) + sizeof(so);
		if((q = realloc(p, len)) == NULL)
			break;
		p = q;
		snprintf(p, len, "%s%s%s", path, applets[i], so);
		if((dl = dlopen(p, RTLD_LAZY)) == NULL)
		{
			fprintf(stderr, "%s: %s: %s\n", "panel_test",
					applets[i], dlerror());
			continue;
		}
		if((pa = dlsym(dl, "applet")) == NULL)
		{
			dlclose(dl);
			continue;
		}
		pa->helper = &helper;
		if((widget = pa->init(pa)) != NULL)
			gtk_box_pack_start(GTK_BOX(box), widget, pa->expand,
					pa->fill, 0);
	}
	free(p);
	gtk_container_add(GTK_CONTAINER(panel.window), box);
	gtk_widget_show_all(panel.window);
	gtk_main();
	return 0;
}

static char const * _helper_config_get(Panel * panel, char const * section,
		char const * variable)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", \"%s\")\n", __func__, section,
			variable);
#endif
	return config_get(panel->config, section, variable);
}

static int _helper_error(Panel * panel, char const * message, int ret)
{
	fputs("panel_test: ", stderr);
	perror(message);
	return ret;
}

static void _helper_position_menu(Panel * panel, GtkMenu * menu, gint * x,
		gint * y, gboolean * push_in)
{
	GtkRequisition req;
	gint sx = 0;
	gint sy = 0;

	gtk_widget_size_request(GTK_WIDGET(menu), &req);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() width=%d, height=%d\n", __func__,
			req.width, req.height);
#endif
	if(req.height <= 0)
		return;
	gtk_window_get_position(GTK_WINDOW(panel->window), x, y);
	gtk_window_get_size(GTK_WINDOW(panel->window), &sx, &sy);
	*y += sy;
	*push_in = TRUE;
}


/* test_list */
static int _test_list(void)
{
	char const path[] = PREFIX "/lib/Panel/applets";
	DIR * dir;
	struct dirent * de;
	size_t len;
	char const * sep = "";

	puts("Applets available:");
	if((dir = opendir(path)) == NULL)
		return _error(path, 1);
	while((de = readdir(dir)) != NULL)
	{
		len = strlen(de->d_name);
		if(len < 4 || strcmp(&de->d_name[len - 3], ".so") != 0)
			continue;
		de->d_name[len - 3] = '\0';
		printf("%s%s", sep, de->d_name);
		sep = ", ";
	}
	putchar('\n');
	closedir(dir);
	return 0;
}


/* config_get_filename */
static char * _config_get_filename(void)
{
	char const * homedir;
	size_t len;
	char * filename;

	if((homedir = getenv("HOME")) == NULL)
		homedir = g_get_home_dir();
	len = strlen(homedir) + 1 + sizeof(PANEL_CONFIG_FILE);
	if((filename = malloc(len)) == NULL)
		return NULL;
	snprintf(filename, len, "%s/%s", homedir, PANEL_CONFIG_FILE);
	return filename;
}


/* error */
static int _error(char const * message, int ret)
{
	fputs("panel_test: ", stderr);
	perror(message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: panel_test applet...\n"
"       panel_test -l\n"
"  -l	Lists the plug-ins available\n", stderr);
	return 1;
}


/* public */
/* functions */
/* main */
int main(int argc, char * argv[])
{
	int o;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "l")) != -1)
		switch(o)
		{
			case 'l':
				return _test_list();
			default:
				return _usage();
		}
	if(optind == argc)
		return _usage();
	_test(&argv[optind]);
	return 0;
}
