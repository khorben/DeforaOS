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
#ifdef DEBUG
# include <stdio.h>
#endif
#include <time.h>
#include <System.h>
#include "Locker/demo.h"
#include "../../config.h"

/* constants */
#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif
#ifndef DATADIR
# define DATADIR	PREFIX "/share"
#endif


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
	/* load the logo */
	if((p = helper->config_get(helper->locker, "logo", "logo")) == NULL)
	{
		helper->error(NULL, "No logo configured", 1);
		p = DATADIR "/icons/gnome/256x256/places/start-here.png";
	}
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
	GdkColor color = { 0x0, 0x0, 0x0, 0x0 };

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
static void _timeout_window(Logo * logo, GtkWidget * widget);

static gboolean _logo_on_timeout(gpointer data)
{
	Logo * logo = data;
	size_t i;

	for(i = 0; i < logo->windows_cnt; i++)
		_timeout_window(logo, logo->windows[i]);
	return TRUE;
}

static void _timeout_window(Logo * logo, GtkWidget * widget)
{
	GdkWindow * window;
	GdkRectangle rect;
	int depth;
	GdkPixbuf * frame;
	GdkPixmap * pixmap;
	int width;
	int height;
	int x;
	int y;
	int seed = time(NULL) ^ getpid() ^ getppid() ^ getuid() ^ getgid();
	const int black = 0x000000ff;

	if(widget == NULL)
		return;
#if GTK_CHECK_VERSION(2, 14, 0)
	window = gtk_widget_get_window(widget);
#else
	window = widget->window;
#endif
	gdk_window_get_geometry(window, &rect.x, &rect.y,
			&rect.width, &rect.height, &depth);
	frame = gdk_pixbuf_new(GDK_COLORSPACE_RGB, 1, 8, rect.width,
			rect.height);
	gdk_pixbuf_fill(frame, black);
	/* draw the logo */
	if(logo->logo != NULL)
	{
		width = gdk_pixbuf_get_width(logo->logo);
		height = gdk_pixbuf_get_height(logo->logo);
		x = (rand() ^ seed) % (rect.width - width);
		y = (rand() ^ seed) % (rect.height - height);
		gdk_pixbuf_copy_area(logo->logo, 0, 0, width, height, frame, x,
				y);
	}
	pixmap = gdk_pixmap_new(window, rect.width, rect.width, -1);
	gdk_draw_pixbuf(pixmap, NULL, frame, 0, 0, 0, 0, rect.width,
			rect.height, GDK_RGB_DITHER_NONE, 0, 0);
	gdk_window_set_back_pixmap(window, pixmap, FALSE);
	gdk_window_clear(window);
	gdk_pixmap_unref(pixmap);
	g_object_unref(frame);
}
