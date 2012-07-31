/* $Id$ */
/* Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org> */
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
#include <Desktop.h>
#include "../src/panel.h"
#include "../config.h"
#ifdef PACKAGE
# undef PACKAGE
#endif
#define PACKAGE "panel-test"
#include "helper.c"


/* private */
/* prototypes */
static int _test(GtkIconSize iconsize, char * applets[]);
static int _usage(void);


/* functions */
/* test */
static int _test(GtkIconSize iconsize, char * applets[])
{
	Panel panel;
	char * filename;
	char const path[] = PREFIX "/lib/Panel/applets/";
#ifdef __APPLE__
	char const ext[] = ".dylib";
#else
	char const ext[] = ".so";
#endif
	GtkWidget * box;
	GtkWidget * widget;
	size_t i;
	size_t len;
	char * p = NULL;
	char * q;
	void * dl;
	PanelAppletHelper helper;
	PanelAppletDefinition * pad;
	PanelApplet * pa;

	if((panel.config = config_new()) == NULL)
		return error_print(PACKAGE);
	if((filename = _config_get_filename()) != NULL
			&& config_load(panel.config, filename) != 0)
		error_print(PACKAGE);
	free(filename);
	panel.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(panel.window), "delete-event", G_CALLBACK(
				gtk_main_quit), NULL);
	gtk_window_set_title(GTK_WINDOW(panel.window), "Applet tester");
	box = gtk_hbox_new(FALSE, 4);
	_helper_init(&helper, &panel, iconsize);
	for(i = 0; applets[i] != NULL; i++)
	{
		len = sizeof(path) + strlen(applets[i]) + sizeof(ext);
		if((q = realloc(p, len)) == NULL)
			break;
		p = q;
		snprintf(p, len, "%s%s%s", path, applets[i], ext);
		if((dl = dlopen(p, RTLD_LAZY)) == NULL)
		{
			fprintf(stderr, "%s: %s: %s\n", "panel-test",
					applets[i], dlerror());
			continue;
		}
		if((pad = dlsym(dl, "applet")) == NULL)
		{
			dlclose(dl);
			continue;
		}
		widget = NULL;
		if((pa = pad->init(&helper, &widget)) != NULL
				&& widget != NULL)
			gtk_box_pack_start(GTK_BOX(box), widget, pad->expand,
					pad->fill, 0);
	}
	free(p);
	gtk_container_add(GTK_CONTAINER(panel.window), box);
	gtk_widget_show_all(panel.window);
	panel.timeout = 0;
	gtk_main();
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: panel-test [-sx] applet...\n"
"       panel-test -l\n"
"  -l	Lists the plug-ins available\n", stderr);
	return 1;
}


/* public */
/* functions */
/* main */
int main(int argc, char * argv[])
{
	GtkIconSize iconsize = GTK_ICON_SIZE_LARGE_TOOLBAR;
	int o;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "lsx")) != -1)
		switch(o)
		{
			case 'l':
				return _applet_list();
			case 's':
				iconsize = GTK_ICON_SIZE_SMALL_TOOLBAR;
				break;
			case 'x':
				iconsize = GTK_ICON_SIZE_MENU;
				break;
			default:
				return _usage();
		}
	if(optind == argc)
		return _usage();
	_test(iconsize, &argv[optind]);
	return 0;
}
