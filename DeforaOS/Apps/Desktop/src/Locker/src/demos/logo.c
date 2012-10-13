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

/* macros */
#ifndef MIN
# define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif


/* Logo */
/* private */
/* types */
typedef struct _LogoWindow
{
	GdkWindow * window;
	GdkPixbuf * frame;
	GdkPixmap * pixmap;
} LogoWindow;

typedef struct _LockerDemo
{
	LockerDemoHelper * helper;
	GdkPixbuf * background;
	GdkPixbuf * logo;
	LogoWindow * windows;
	size_t windows_cnt;
	guint source;
	guint frame_num;

	/* settings */
	int scroll;
	int opacity;
} Logo;


/* prototypes */
/* plug-in */
static Logo * _logo_init(LockerDemoHelper * helper);
static void _logo_destroy(Logo * logo);
static int _logo_add(Logo * logo, GdkWindow * window);
static void _logo_remove(Logo * logo, GdkWindow * window);
static void _logo_start(Logo * logo);
static void _logo_stop(Logo * logo);

/* callbacks */
static gboolean _logo_on_idle(gpointer data);
static gboolean _logo_on_timeout(gpointer data);


/* public */
/* variables */
/* plug-in */
LockerDemoDefinition plugin =
{
	"Logo",
	"start-here",
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
	logo->background = NULL;
	logo->logo = NULL;
	logo->windows = NULL;
	logo->windows_cnt = 0;
	logo->source = 0;
	logo->frame_num = 0;
	logo->scroll = 0;
	logo->opacity = 255;
	/* load the background */
	if((p = helper->config_get(helper->locker, "logo", "background"))
			== NULL)
	{
		helper->error(NULL, "No background configured", 1);
		p = DATADIR "/pixmaps/backgrounds/gnome/background-default.jpg";
	}
	if(p != NULL && (logo->background = gdk_pixbuf_new_from_file(p, &error))
			== NULL)
	{
		helper->error(NULL, error->message, 1);
		g_error_free(error);
		error = NULL;
	}
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
		error = NULL;
	}
	return logo;
}


/* logo_destroy */
static void _logo_destroy(Logo * logo)
{
	object_delete(logo);
}


/* logo_add */
static int _logo_add(Logo * logo, GdkWindow * window)
{
	LogoWindow * p;
	GdkColor color = { 0x0, 0x0, 0x0, 0x0 };

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() window=%p\n", __func__, (void *)window);
#endif
	if((p = realloc(logo->windows, sizeof(*p) * (logo->windows_cnt + 1)))
			== NULL)
		return -1;
	logo->windows = p;
	/* set the default color */
	gdk_window_set_background(window, &color);
	gdk_window_clear(window);
	logo->windows[logo->windows_cnt].window = window;
	logo->windows[logo->windows_cnt].frame = NULL;
	logo->windows[logo->windows_cnt++].pixmap = NULL;
	return 0;
}


/* logo_remove */
static void _logo_remove(Logo * logo, GdkWindow * window)
{
	size_t i;

	for(i = 0; i < logo->windows_cnt; i++)
		if(logo->windows[i].window == window)
		{
			logo->windows[i].window = NULL;
			if(logo->windows[i].frame != NULL)
				g_object_unref(logo->windows[i].frame);
			logo->windows[i].frame = NULL;
			if(logo->windows[i].pixmap != NULL)
				gdk_pixmap_unref(logo->windows[i].pixmap);
			logo->windows[i].pixmap = NULL;
		}
	/* FIXME reorganize the array and free memory */
	for(i = 0; i < logo->windows_cnt; i++)
		if(logo->windows[i].window != NULL)
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
	LockerDemoHelper * helper = logo->helper;
	char const * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	/* settings */
	/* scrolling */
	logo->scroll = 0;
	if((p = helper->config_get(helper->locker, "logo", "scroll")) != NULL)
		logo->scroll = strtol(p, NULL, 10);
	/* opacity */
	logo->opacity = 255;
	if((p = helper->config_get(helper->locker, "logo", "opacity")) != NULL)
	{
		logo->opacity = strtol(p, NULL, 10);
		if(logo->opacity < 0 || logo->opacity > 255)
			logo->opacity = 255;
	}
	if(logo->source == 0)
		_logo_on_timeout(logo);
}


/* logo_stop */
static void _logo_stop(Logo * logo)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(logo->source != 0)
		g_source_remove(logo->source);
	logo->source = 0;
}


/* callbacks */
/* logo_on_idle */
static gboolean _logo_on_idle(gpointer data)
{
	Logo * logo = data;

	logo->source = g_timeout_add((logo->scroll != 0) ? 40 : 10000,
			_logo_on_timeout, logo);
	return FALSE;
}


/* logo_on_timeout */
static void _timeout_window(Logo * logo, LogoWindow * window);

static gboolean _logo_on_timeout(gpointer data)
{
	Logo * logo = data;
	size_t i;

	for(i = 0; i < logo->windows_cnt; i++)
		_timeout_window(logo, &logo->windows[i]);
	logo->frame_num += logo->scroll;
	logo->source = g_idle_add(_logo_on_idle, logo);
	return FALSE;
}

static void _timeout_window(Logo * logo, LogoWindow * window)
{
	GdkWindow * w;
	GdkRectangle rect;
	int depth;
	GdkPixbuf * frame;
	GdkPixmap * pixmap;
	int width = 0;
	int height = 0;
	int i;
	int j;
	int x = 0;
	int y = 0;
	int offset_x = 0;
	int offset_y = 0;
	int src_x;
	int src_y;
	int src_w;
	int src_h;
	int seed = time(NULL) ^ getpid() ^ getppid() ^ getuid() ^ getgid();
	const int black = 0x000000ff;

	if((w = window->window) == NULL)
		return;
	gdk_window_get_geometry(w, &rect.x, &rect.y, &rect.width,
			&rect.height, &depth);
	/* reallocate the frame and background if necessary */
	if(window->frame == NULL
			|| gdk_pixbuf_get_width(window->frame) != rect.width
			|| gdk_pixbuf_get_height(window->frame) != rect.height)
	{
		if(window->frame != NULL)
			g_object_unref(window->frame);
		window->frame = gdk_pixbuf_new(GDK_COLORSPACE_RGB, 1, 8,
				rect.width, rect.height);
		if(window->pixmap != NULL)
			gdk_pixmap_unref(window->pixmap);
		window->pixmap = gdk_pixmap_new(w, rect.width, rect.width, -1);
	}
	frame = window->frame;
	pixmap = window->pixmap;
	gdk_pixbuf_fill(frame, black);
	/* draw the background */
	if(logo->background != NULL)
	{
		width = gdk_pixbuf_get_width(logo->background);
		height = gdk_pixbuf_get_height(logo->background);
		if((logo->scroll != 0) && width > 0 && height > 0)
		{
			offset_x = logo->frame_num % width;
			offset_y = logo->frame_num % height;
		}
	}
	src_y = offset_y;
	for(j = 0; height > 0 && j < rect.height; j += src_h)
	{
		src_h = MIN(height - src_y, rect.height - j);
		src_x = offset_x;
		for(i = 0; width > 0 && i < rect.width; i += src_w)
		{
			src_w = MIN(width - src_x, rect.width - i);
			gdk_pixbuf_copy_area(logo->background, src_x, src_y,
					src_w, src_h, frame, i, j);
			src_x = 0;
		}
		src_y = 0;
	}
	/* draw the logo */
	if(logo->logo != NULL)
	{
		width = gdk_pixbuf_get_width(logo->logo);
		width = MIN(rect.width, width);
		height = gdk_pixbuf_get_height(logo->logo);
		height = MIN(rect.height, height);
		if(logo->scroll == 0)
		{
			if(rect.width > width)
				x = (rand() ^ seed) % (rect.width - width);
			if(rect.height > height)
				y = (rand() ^ seed) % (rect.height - height);
		}
		else
		{
			if(rect.width > width)
				x = (rect.width - width) / 2;
			if(rect.height > height)
				y = (rect.height - height) / 2;
		}
		gdk_pixbuf_composite(logo->logo, frame, 0, 0,
				rect.width, rect.height, x, y,
				1.0, 1.0, GDK_INTERP_NEAREST, logo->opacity);
	}
	gdk_draw_pixbuf(pixmap, NULL, frame, 0, 0, 0, 0, rect.width,
			rect.height, GDK_RGB_DITHER_NONE, 0, 0);
	gdk_window_set_back_pixmap(w, pixmap, FALSE);
	gdk_window_clear(w);
}
