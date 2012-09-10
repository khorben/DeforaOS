/* $Id$ */
/* Copyright (c) 2007-2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Browser */
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
 * - add a file count and disk usage tab for directories */



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <locale.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include <System.h>
#include "../include/Browser.h"
#define _Browser _BrowserHidden /* XXX */
#include "browser.h"
#undef _Browser
#include "../config.h"
#define _(string) gettext(string)
#define N_(string) (string)

#define COMMON_CONFIG_FILENAME
#include "common.c"

/* constants */
#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif
#ifndef DATADIR
# define DATADIR	PREFIX "/share"
#endif
#ifndef LOCALEDIR
# define LOCALEDIR	DATADIR "/locale"
#endif


/* properties */
/* private */
/* types */
#define _Properties _Browser
typedef struct _Properties
{
	/* internal */
	Mime * mime;
	char * filename;

	/* plugins */
	BrowserPluginHelper helper;

	/* widgets */
	GtkIconTheme * theme;
	GtkWidget * window;
	GtkWidget * notebook;
} Properties;


/* variables */
static unsigned int _properties_cnt = 0; /* XXX set as static in _properties */

/* functions */
static int _properties(Mime * mime, int filec, char * const filev[]);

/* properties */
static Properties * _properties_new(Mime * mime, char const * filename);
static void _properties_delete(Properties * properties);

/* accessors */
static Mime * _properties_get_mime(Properties * properties);
static int _properties_set_location(Properties * properties,
		char const * filename);

/* useful */
static int _properties_error(Properties * properties, char const * message,
		int ret);
static int _properties_load(Properties * properties, char const * name);

/* helpers */
static int _properties_helper_set_location(Properties * properties,
		char const * filename);

/* callbacks */
static void _properties_on_close(gpointer data);
static gboolean _properties_on_closex(gpointer data);


/* functions */
/* properties */
static int _properties(Mime * mime, int filec, char * const filev[])
{
	int ret = 0;
	int i;
	Properties * properties;
	char * p;

	for(i = 0; i < filec; i++)
	{
		p = (filev[i][0] != '/') ? g_build_filename(g_get_current_dir(),
				"/", filev[i], NULL) : g_strdup(filev[i]);
		if((properties = _properties_new(mime, p)) == NULL)
			ret |= 1;
		else
			_properties_cnt++;
		g_free(p);
	}
	return ret;
}


/* properties_new */
static int _new_load(Properties * properties);

static Properties * _properties_new(Mime * mime, char const * filename)
{
	Properties * properties;
	GtkWidget * vbox;
	GtkWidget * bbox;
	GtkWidget * widget;
	gchar * p;
	char buf[256];

	if(filename == NULL)
		return NULL;
	if((properties = malloc(sizeof(*properties))) == NULL)
	{
		_properties_error(NULL, "malloc", 1);
		return NULL;
	}
	properties->mime = mime;
	properties->filename = strdup(filename);
	properties->helper.browser = properties;
	properties->helper.error = _properties_error;
	properties->helper.get_mime = _properties_get_mime;
	properties->helper.set_location = _properties_helper_set_location;
	properties->window = NULL;
	if(properties->filename == NULL)
	{
		_properties_delete(properties);
		return NULL;
	}
	properties->theme = gtk_icon_theme_get_default();
	/* window */
	properties->window = gtk_dialog_new();
	p = g_filename_display_basename(filename);
	snprintf(buf, sizeof(buf), "%s%s", _("Properties of "), p);
	g_free(p);
	gtk_window_set_default_size(GTK_WINDOW(properties->window), 300, 400);
	gtk_window_set_title(GTK_WINDOW(properties->window), buf);
	g_signal_connect_swapped(properties->window, "delete-event",
			G_CALLBACK(_properties_on_closex), properties);
#if GTK_CHECK_VERSION(2, 14, 0)
	vbox = gtk_dialog_get_content_area(GTK_DIALOG(properties->window));
#else
	vbox = GTK_DIALOG(properties->window)->vbox;
#endif
	/* notebook */
	properties->notebook = gtk_notebook_new();
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(properties->notebook), TRUE);
	gtk_box_pack_start(GTK_BOX(vbox), properties->notebook, TRUE, TRUE, 0);
	gtk_widget_show_all(vbox);
	/* button box */
#if GTK_CHECK_VERSION(2, 14, 0)
	bbox = gtk_dialog_get_action_area(GTK_DIALOG(properties->window));
#else
	bbox = GTK_DIALOG(properties->window)->action_area;
#endif
	widget = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	g_signal_connect_swapped(widget, "clicked",
			G_CALLBACK(_properties_on_close), properties);
	gtk_container_add(GTK_CONTAINER(bbox), widget);
	gtk_widget_show_all(bbox);
	if(_new_load(properties) != 0)
		_properties_error(properties, error_get(), -1);
	else if(filename != NULL)
	{
		if(_properties_set_location(properties, filename) != 0)
			_properties_error(properties, error_get(), -1);
		else
			gtk_widget_show(properties->window);
	}
	return properties;
}

static int _new_load(Properties * properties)
{
	Config * config;
	char const * plugins = NULL;
	char * p;
	char * q;
	size_t i;
	int cnt = 0;

	p = _common_config_filename(BROWSER_CONFIG_FILE);
	if((config = config_new()) != NULL && config_load(config, p) == 0
			&& (plugins = config_get(config, NULL, "properties"))
			== NULL)
		plugins = "properties,preview";
	string_delete(p);
	if(plugins != NULL && strlen(plugins) && (p = strdup(plugins)) != NULL)
	{
		/* XXX if plugins is only commas nothing will be loaded */
		for(q = p, i = 0;;)
		{
			if(q[i] == '\0')
			{
				if(_properties_load(properties, q) == 0)
					cnt++;
				break;
			}
			if(q[i++] != ',')
				continue;
			q[i - 1] = '\0';
			if(_properties_load(properties, q) == 0)
				cnt++;
			q += i;
			i = 0;
		}
		free(p);
	}
	else
	{
		if(_properties_load(properties, "properties") == 0)
			cnt++;
		if(_properties_load(properties, "preview") == 0)
			cnt++;
	}
	if(config != NULL)
		config_delete(config);
	/* consider ourselves successful if at least one plug-in was loaded */
	return (cnt > 0) ? 0 : -1;
}


/* properties_delete */
static void _properties_delete(Properties * properties)
{
	if(properties->window != NULL)
		gtk_widget_destroy(properties->window);
	free(properties->filename);
	free(properties);
	_properties_cnt--;
}


/* accessors */
/* properties_get_mime */
static Mime * _properties_get_mime(Properties * properties)
{
	return properties->mime;
}


/* properties_set_location */
static int _properties_set_location(Properties * properties,
		char const * filename)
{
	char * p;

	if((p = strdup(filename)) == NULL)
		return -error_set_code(1, "%s: %s", filename, strerror(errno));
	free(properties->filename);
	properties->filename = p;
	return 0;
}


/* _properties_error */
static void _error_response(GtkWidget * widget, gint arg, gpointer data);
static int _error_text(char const * message, char const * error, int ret);

static int _properties_error(Properties * properties, char const * message,
		int ret)
{
	GtkWidget * dialog;
	char const * error;

	error = strerror(errno);
	if(properties == NULL)
		return _error_text(message, error, ret);
	dialog = gtk_message_dialog_new((properties != NULL
				&& properties->window != NULL)
			? GTK_WINDOW(properties->window) : NULL, 0,
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
#if GTK_CHECK_VERSION(2, 6, 0)
			"%s", _("Error"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
#endif
			"%s: %s", message, error);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
	if(properties != NULL && properties->window != NULL)
		gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(
					properties->window));
	g_signal_connect(dialog, "response", G_CALLBACK(_error_response),
			(ret < 0) ? &_properties_cnt : NULL);
	gtk_widget_show(dialog);
	return ret;
}

static void _error_response(GtkWidget * widget, gint arg, gpointer data)
{
	unsigned int * cnt = data;

	if(cnt == NULL)
		gtk_widget_destroy(widget);
	else if(--(*cnt) == 0)
		gtk_main_quit();
	else
		gtk_widget_destroy(widget);
}

static int _error_text(char const * message, char const * error, int ret)
{
	fprintf(stderr, "%s: %s: %s\n", "properties", message, error);
	return ret;
}


/* properties_load */
static int _properties_load(Properties * properties, char const * name)
{
	Plugin * p;
	BrowserPluginDefinition * bpd;
	BrowserPlugin * bp;
	GdkPixbuf * icon = NULL;
	GtkWidget * hbox;
	GtkWidget * widget;
	GList * l;

	if((p = plugin_new(LIBDIR, PACKAGE, "plugins", name)) == NULL)
		return -1;
	if((bpd = plugin_lookup(p, "plugin")) == NULL)
	{
		plugin_delete(p);
		return -1;
	}
	if(bpd->init == NULL || bpd->destroy == NULL || bpd->get_widget == NULL
			|| (bp = bpd->init(&properties->helper)) == NULL)
	{
		plugin_delete(p);
		return -1;
	}
	widget = bpd->get_widget(bp);
	l = g_list_append(NULL, properties->filename);
	bpd->refresh(bp, l);
	g_list_free(l);
	/* label */
	if(bpd->icon != NULL)
		icon = gtk_icon_theme_load_icon(properties->theme, bpd->icon,
				24, 0, NULL);
	if(icon == NULL)
		icon = gtk_icon_theme_load_icon(properties->theme,
				"gnome-settings", 24, 0, NULL);
	hbox = gtk_hbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(hbox), gtk_image_new_from_pixbuf(icon),
			FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(_(bpd->name)), TRUE,
			TRUE, 0);
	gtk_widget_show_all(hbox);
	gtk_notebook_append_page(GTK_NOTEBOOK(properties->notebook), widget,
			hbox);
	/* XXX configure the default plug-in somewhere */
	if(strcmp(name, "properties") == 0)
		gtk_notebook_set_current_page(GTK_NOTEBOOK(
					properties->notebook), -1);
	return 0;
}


/* helpers */
static int _properties_helper_set_location(Properties * properties,
		char const * filename)
{
	int res;

	if((res = _properties_set_location(properties, filename)) != 0)
		return -_properties_error(properties, error_get(), 1);
	return 0;
}


/* callbacks */
static void _properties_on_close(gpointer data)
{
	Properties * properties = data;

	_properties_delete(properties);
	if(_properties_cnt == 0)
		gtk_main_quit();
}

static gboolean _properties_on_closex(gpointer data)
{
	_properties_on_close(data);
	return FALSE;
}


/* usage */
static int _usage(void)
{
	fputs(_("Usage: properties file...\n"), stderr);
	return 1;
}


/* public */
/* functions */
/* main */
int main(int argc, char * argv[])
{
	int ret;
	int o;
	Mime * mime;

	if(setlocale(LC_ALL, "") == NULL)
		_properties_error(NULL, "setlocale", 1);
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind == argc)
		return _usage();
	mime = mime_new(NULL);
	ret = _properties(mime, argc - optind, &argv[optind]);
	gtk_main();
	if(mime != NULL)
		mime_delete(mime);
	return (ret == 0) ? 0 : 2;
}
