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



#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <gtk/gtk.h>
#include <Panel.h>
#include "../config.h"


/* constants */
#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif


/* private */
/* functions */
static char const * _helper_config_get(Panel * panel, char const * section,
		char const * variable);

static int _test(char * applets[])
{
	char const path[] = PREFIX "/lib/Panel/applets/";
	char const so[] = ".so";
	GtkWidget * window;
	GtkWidget * box;
	GtkWidget * widget;
	size_t i;
	size_t len;
	char * p = NULL;
	char * q;
	void * dl;
	PanelAppletHelper helper;
	PanelApplet * pa;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(
				gtk_main_quit), NULL);
	gtk_window_set_title(GTK_WINDOW(window), "Applet tester");
	box = gtk_hbox_new(FALSE, 4);
	memset(&helper, 0, sizeof(helper));
	helper.icon_size = GTK_ICON_SIZE_SMALL_TOOLBAR;
	helper.config_get = _helper_config_get;
	for(i = 0; applets[i] != NULL; i++)
	{
		len = sizeof(path) + strlen(applets[i]) + sizeof(so);
		if((q = realloc(p, len)) == NULL)
			break;
		p = q;
		snprintf(p, len, "%s%s%s", path, applets[i], so);
		if((dl = dlopen(p, RTLD_LAZY)) == NULL)
			continue;
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
	gtk_container_add(GTK_CONTAINER(window), box);
	gtk_widget_show_all(window);
	return 0;
}

static char const * _helper_config_get(Panel * panel, char const * section,
		char const * variable)
{
	return NULL;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: panel_test applet...\n", stderr);
	return 1;
}


/* public */
/* functions */
/* main */
int main(int argc, char * argv[])
{
	int o;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind == argc)
		return _usage();
	_test(&argv[optind]);
	gtk_main();
	return 0;
}
