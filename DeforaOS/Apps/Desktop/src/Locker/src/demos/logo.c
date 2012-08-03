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



#include <stdlib.h>
#ifdef DEBUG
# include <stdio.h>
#endif
#include <System.h>
#include "Locker/demo.h"


/* Logo */
/* private */
/* types */
typedef struct _LockerDemo
{
	LockerDemoHelper * helper;
	GdkPixbuf * logo;
	GtkWidget ** windows;
	size_t windows_cnt;
	guint timeout;
} Logo;


/* prototypes */
/* plug-in */
static Logo * _logo_init(LockerDemoHelper * helper);
static void _logo_destroy(Logo * logo);
static int _logo_add(Logo * logo, GtkWidget * window);
static void _logo_remove(Logo * logo, GtkWidget * window);
static void _logo_start(Logo * logo);
static void _logo_stop(Logo * logo);

/* callbacks */
static gboolean _logo_on_timeout(gpointer data);


/* public */
/* variables */
/* plug-in */
LockerDemoDefinition plugin =
{
	"Logo",
	NULL,
	NULL,
	_logo_init,
	_logo_destroy,
	_logo_add,
	_logo_remove,
	_logo_start,
	_logo_stop
};


/* private */
/* functions */
/* plug-in */
/* logo_init */
static Logo * _logo_init(LockerDemoHelper * helper)
{
	Logo * logo;
	char const * p;
	GError * error = NULL;

	if((logo = object_new(sizeof(*logo))) == NULL)
		return NULL;
	/* initialization */
	logo->helper = helper;
	logo->logo = NULL;
	logo->windows = NULL;
	logo->windows_cnt = 0;
	logo->timeout = 0;
	/* load the logo if configured */
	if((p = helper->config_get(helper->locker, "logo", "logo")) != NULL)
		if((logo->logo = gdk_pixbuf_new_from_file(p, &error)) == NULL)
		{
			helper->error(NULL, error->message, 1);
			g_error_free(error);
		}
	return logo;
}


/* logo_destroy */
static void _logo_destroy(Logo * logo)
{
	object_delete(logo);
}


/* logo_add */
static int _logo_add(Logo * logo, GtkWidget * window)
{
	GdkWindow * w;
	GtkWidget ** p;
	GdkColor color = { 0xff000000, 0xffff, 0x0, 0x0 };

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() window=%p\n", __func__, (void *)window);
#endif
	if((p = realloc(logo->windows, sizeof(*p) * (logo->windows_cnt + 1)))
			== NULL)
		return -1;
	logo->windows = p;
	w = gtk_widget_get_window(window);
	/* set the default color */
	gdk_window_set_background(w, &color);
	gdk_window_clear(w);
	logo->windows[logo->windows_cnt++] = window;
	return 0;
}


/* logo_remove */
static void _logo_remove(Logo * logo, GtkWidget * window)
{
	size_t i;

	for(i = 0; i < logo->windows_cnt; i++)
		if(logo->windows[i] == window)
			logo->windows[i] = NULL;
	/* FIXME reorganize the array and free memory */
	for(i = 0; i < logo->windows_cnt; i++)
		if(logo->windows[i] != NULL)
			break;
	if(i == logo->windows_cnt)
	{
		/* there are no windows left */
		_logo_stop(logo);
		free(logo->windows);
		logo->windows = NULL;
		logo->windows_cnt = 0;
	}
}


/* logo_start */
static void _logo_start(Logo * logo)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(logo->timeout == 0 && _logo_on_timeout(logo) == TRUE)
		logo->timeout = g_timeout_add(10000, _logo_on_timeout, logo);
}


/* logo_stop */
static void _logo_stop(Logo * logo)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(logo->timeout != 0)
		g_source_remove(logo->timeout);
	logo->timeout = 0;
}


/* callbacks */
/* logo_on_timeout */
static gboolean _logo_on_timeout(gpointer data)
{
	Logo * logo = data;

	/* FIXME really implement */
	logo->timeout = FALSE;
	return FALSE;
}
