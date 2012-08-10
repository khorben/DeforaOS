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
};


/* prototypes */
static int _test(char const * demo);
static int _usage(void);

/* helpers */
static char const * _test_helper_config_get(Locker * locker,
		char const * section, char const * variable);
static int _test_helper_config_set(Locker * locker, char const * section,
		char const * variable, char const * value);
static int _test_helper_error(Locker * locker, char const * message, int ret);

/* callbacks */
static gboolean _test_on_closex(void);


/* functions */
/* test */
static Config * _test_config(void);

static int _test(char const * demo)
{
	int ret = 0;
	Locker * locker;
	LockerDemoHelper helper;
	Plugin * plugin;
	LockerDemoDefinition * dplugin;
	LockerDemo * d;
	GtkWidget * window;

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
	if((dplugin = plugin_lookup(plugin, "plugin")) == NULL
			|| dplugin->init == NULL
			|| (d = dplugin->init(&helper)) == NULL)
	{
		plugin_delete(plugin);
		if(locker->config != NULL)
			config_delete(locker->config);
		object_delete(locker);
		return error_set_print(PROGNAME, 1, "%s: %s", demo,
				"Could not initialize demo plug-in");
	}
	/* widgets */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);
	g_signal_connect(window, "delete-event", G_CALLBACK(_test_on_closex),
			NULL);
	gtk_widget_show_all(window);
	if(dplugin->add(d, window) != 0)
		ret = error_set_print(PROGNAME, 1, "%s: %s", demo,
				"Could not add window");
	else
	{
		dplugin->start(d);
		gtk_main();
	}
	dplugin->destroy(d);
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
	fputs("Usage: locker-test demo\n", stderr);
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
	return FALSE;
}


/* public */
/* functions */
/* main */
int main(int argc, char * argv[])
{
	int o;
	char const * demo = NULL;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind + 1 != argc)
		return _usage();
	demo = argv[optind];
	return (_test(demo) == 0) ? 2 : 0;
}
