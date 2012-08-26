/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Locker */
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
#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <System.h>
#include "../include/Locker/demo.h"
#include "../src/locker.h"
#include "../config.h"

#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif
#ifndef LIBDIR
# define LIBDIR		PREFIX "/lib"
#endif
#ifndef PROGNAME
# define PROGNAME	"locker-test"
#endif


/* private */
/* types */
struct _Locker
{
	Config * config;
	LockerDemoDefinition * dplugin;
	LockerDemo * demo;
};


/* prototypes */
static int _test(int root, int width, int height, char const * demo);
static int _usage(void);

/* helpers */
static char const * _test_helper_config_get(Locker * locker,
		char const * section, char const * variable);
static int _test_helper_config_set(Locker * locker, char const * section,
		char const * variable, char const * value);
static int _test_helper_error(Locker * locker, char const * message, int ret);

/* callbacks */
static gboolean _test_on_closex(void);
static void _test_on_start(gpointer data);
static void _test_on_stop(gpointer data);


/* functions */
/* test */
static Config * _test_config(void);

static int _test(int root, int width, int height, char const * demo)
{
	int ret = 0;
	Locker * locker;
	LockerDemoHelper helper;
	Plugin * plugin;
	GtkWidget * window;
	GtkWidget * dwindow = NULL;
	GdkWindow * wwindow;
	GtkWidget * widget;
	GtkWidget * button;
	GdkScreen * screen;

	if((locker = object_new(sizeof(*locker))) == NULL)
		return error_print(PROGNAME);
	locker->config = _test_config();
	/* helper */
	helper.locker = locker;
	helper.error = _test_helper_error;
	helper.config_get = _test_helper_config_get;
	helper.config_set = _test_helper_config_set;
	if((plugin = plugin_new(LIBDIR, PACKAGE, "demos", demo)) == NULL)
	{
		if(locker->config != NULL)
			config_delete(locker->config);
		object_delete(locker);
		return error_set_print(PROGNAME, 1, "%s: %s", demo,
				"Could not load demo plug-in");
	}
	if((locker->dplugin = plugin_lookup(plugin, "plugin")) == NULL
			|| locker->dplugin->init == NULL
			|| (locker->demo = locker->dplugin->init(&helper))
			== NULL)
	{
		plugin_delete(plugin);
		if(locker->config != NULL)
			config_delete(locker->config);
		object_delete(locker);
		return error_set_print(PROGNAME, 1, "%s: %s", demo,
				"Could not initialize demo plug-in");
	}
	/* widgets */
	/* toolbar */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	g_signal_connect(window, "delete-event", G_CALLBACK(_test_on_closex),
			NULL);
	widget = gtk_hbox_new(TRUE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(widget), 4);
	button = gtk_button_new_with_label("Start");
	g_signal_connect_swapped(button, "clicked", G_CALLBACK(_test_on_start),
			locker);
	gtk_box_pack_start(GTK_BOX(widget), button, FALSE, TRUE, 0);
	button = gtk_button_new_with_label("Stop");
	g_signal_connect_swapped(button, "clicked", G_CALLBACK(_test_on_stop),
			locker);
	gtk_box_pack_start(GTK_BOX(widget), button, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(window), widget);
	gtk_widget_show_all(window);
	/* demo window */
	if(root)
	{
		screen = gdk_screen_get_default();
		wwindow = gdk_screen_get_root_window(screen);
	}
	else
	{
		dwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_default_size(GTK_WINDOW(dwindow), width, height);
		g_signal_connect(dwindow, "delete-event", G_CALLBACK(
					_test_on_closex), NULL);
		gtk_widget_show_all(dwindow);
#if GTK_CHECK_VERSION(2, 14, 0)
		wwindow = gtk_widget_get_window(dwindow);
#else
		wwindow = dwindow->window;
#endif
	}
	if(locker->dplugin->add(locker->demo, wwindow) != 0)
		ret = error_set_print(PROGNAME, 1, "%s: %s", demo,
				"Could not add window");
	else
	{
		locker->dplugin->start(locker->demo);
		gtk_main();
		if(dwindow != NULL)
			gtk_widget_destroy(dwindow);
		gtk_widget_destroy(window);
	}
	locker->dplugin->destroy(locker->demo);
	plugin_delete(plugin);
	if(locker->config != NULL)
		config_delete(locker->config);
	object_delete(locker);
	return ret;
}

static Config * _test_config(void)
{
	Config * config;
	char const * homedir;
	String * filename;

	if((config = config_new()) == NULL)
		return NULL;
	if((homedir = getenv("HOME")) == NULL)
		homedir = g_get_home_dir();
	if((filename = string_new_append(homedir, "/", LOCKER_CONFIG_FILE,
					NULL)) == NULL)
	{
		error_print(PROGNAME);
		return config;
	}
	if(config_load(config, filename) != 0)
		/* we can ignore errors */
		error_print(PROGNAME);
	string_delete(filename);
	return config;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: locker-test [-r ][-w width][-h height] demo\n"
"  -r	Display the demo on the root window\n"
"  -w	Set the width of the test window\n"
"  -h	Set the height of the test window\n", stderr);
	return 1;
}


/* helpers */
/* test_helper_config_get */
static char const * _test_helper_config_get(Locker * locker,
		char const * section, char const * variable)
{
	char const * ret;
	String * s = NULL;

	if(locker->config == NULL)
	{
		error_set_code(1, "%s", "Configuration not available");
		return NULL;
	}
	if(section != NULL
			&& (s = string_new_append("demo::", section, NULL))
			== NULL)
		return NULL;
	ret = config_get(locker->config, s, variable);
	string_delete(s);
	return ret;
}


/* test_helper_config_set */
static int _test_helper_config_set(Locker * locker, char const * section,
		char const * variable, char const * value)
{
	if(locker->config == NULL)
		return -error_set_code(1, "%s", "Configuration not available");
	return config_set(locker->config, section, variable, value);
}


static int _test_helper_error(Locker * locker, char const * message, int ret)
{
	return error_set_print(PROGNAME, ret, "%s", message);
}


/* callbacks */
/* test_on_closex */
static gboolean _test_on_closex(void)
{
	gtk_main_quit();
	return TRUE;
}


/* test_on_start */
static void _test_on_start(gpointer data)
{
	Locker * locker = data;

	locker->dplugin->start(locker->demo);
}



/* test_on_stop */
static void _test_on_stop(gpointer data)
{
	Locker * locker = data;

	locker->dplugin->stop(locker->demo);
}


/* public */
/* functions */
/* main */
int main(int argc, char * argv[])
{
	int o;
	int root = 0;
	int width = 640;
	int height = 480;
	char const * demo = NULL;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "rw:h:")) != -1)
		switch(o)
		{
			case 'r':
				root = 1;
				break;
			case 'w':
				width = strtoul(optarg, NULL, 0);
				break;
			case 'h':
				height = strtoul(optarg, NULL, 0);
				break;
			default:
				return _usage();
		}
	if(width == 0 || height == 0 || optind + 1 != argc)
		return _usage();
	demo = argv[optind];
	return (_test(root, width, height, demo) == 0) ? 2 : 0;
}
