/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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
/* TODO:
 * - re-factor and re-indent adequately
 * - no longer obfuscate the authentication widgets
 * - improve performance */



#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <gdk/gdkx.h>
#include <System.h>
#include "Locker/demo.h"
#include "../../config.h"

#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif

#ifndef DATADIR
# define DATADIR	PREFIX "/share"
#endif

#ifndef DEMODIR
# define DEMODIR	DATADIR "/gtk-2.0/demo"
#endif


/* Gtk-demo */
/* private */
/* types */
typedef enum _GtkDemoImage
{
	GDI_BACKGROUND = 0,
	GDI_APPLE_RED,
	GDI_GNOME_APPLETS,
	GDI_GNOME_CALENDAR,
	GDI_GNOME_FOOT,
	GDI_GNOME_GMUSH,
	GDI_GNOME_GIMP,
	GDI_GNOME_GSAME,
	GDI_GNU_KEYS
} GtkDemoImage;
#define GDI_LAST GDI_GNU_KEYS
#define GDI_COUNT (GDI_LAST + 1)

typedef struct _GtkDemo
{
	GdkPixbuf * images[GDI_COUNT];
	GtkWidget ** windows;
	size_t windows_cnt;
	guint timeout;
	guint frame_num;
} GtkDemo;


/* constants */
static const char * _gtk_demo_images[GDI_COUNT] =
{
	DEMODIR "/background.jpg",
	DEMODIR "/apple-red.png",
	DEMODIR "/gnome-applets.png",
	DEMODIR "/gnome-calendar.png",
	DEMODIR "/gnome-foot.png",
	DEMODIR "/gnome-gmush.png",
	DEMODIR "/gnome-gimp.png",
	DEMODIR "/gnome-gsame.png",
	DEMODIR "/gnu-keys.png"
};


/* prototypes */
/* plug-in */
static int _gtk_demo_init(LockerDemo * demo);
static void _gtk_demo_destroy(LockerDemo * demo);
static int _gtk_demo_add(LockerDemo * demo, GtkWidget * window);
static void _gtk_demo_remove(LockerDemo * demo, GtkWidget * window);

/* callbacks */
static gboolean _gtk_demo_on_timeout(gpointer data);


/* public */
/* variables */
/* plug-in */
LockerDemo demo =
{
	NULL,
	"Gtk-demo",
	_gtk_demo_init,
	_gtk_demo_destroy,
	_gtk_demo_add,
	_gtk_demo_remove,
	NULL
};


/* private */
/* functions */
/* plug-in */
/* gtk_demo_init */
static int _gtk_demo_init(LockerDemo * demo)
{
	LockerDemoHelper * helper = demo->helper;
	GtkDemo * gtkdemo;
	size_t i;
	GError * error = NULL;

	if((gtkdemo = object_new(sizeof(*gtkdemo))) == NULL)
		return -1;
	demo->priv = gtkdemo;
	for(i = 0; i < GDI_COUNT; i++)
		if((gtkdemo->images[i] = gdk_pixbuf_new_from_file(
						_gtk_demo_images[i],
						&error)) == NULL)
			helper->error(NULL, error->message, 1);
	if(error != NULL)
		g_error_free(error);
	gtkdemo->windows = NULL;
	gtkdemo->windows_cnt = 0;
	gtkdemo->timeout = 0;
	gtkdemo->frame_num = 0;
	return 0;
}


/* gtk_demo_destroy */
static void _gtk_demo_destroy(LockerDemo * demo)
{
	GtkDemo * gtkdemo = demo->priv;
	size_t i;

	if(gtkdemo->timeout != 0)
		g_source_remove(gtkdemo->timeout);
	for(i = 0; i < GDI_COUNT; i++)
		if(gtkdemo->images[i] != NULL)
			g_object_unref(gtkdemo->images[i]);
	object_delete(gtkdemo);
}


/* gtk_demo_add */
static int _gtk_demo_add(LockerDemo * demo, GtkWidget * window)
{
	int ret = 0;
	LockerDemoHelper * helper = demo->helper;
	GtkDemo * gtkdemo = demo->priv;
	GtkWidget ** p;
	GdkColor color = { 0xff000000, 0xffff, 0x0, 0x0 };
	GdkGC * gc;
	GdkPixmap * pixmap;
	GdkPixbuf * background = gtkdemo->images[GDI_BACKGROUND];
	GdkRectangle rect;
	int depth;
	int w;
	int h;
	int i;
	int j;
	GError * error = NULL;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() window=%p\n", __func__, (void *)window);
#endif
	if((p = realloc(gtkdemo->windows, sizeof(*p) * (gtkdemo->windows_cnt
						+ 1))) == NULL)
		return -1;
	gtkdemo->windows = p;
	gdk_window_get_geometry(gtk_widget_get_window(window), &rect.x, &rect.y,
			&rect.width, &rect.height, &depth);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() (%dx%d), (%dx%d)@%dbpp\n", __func__,
			rect.x, rect.y, rect.width, rect.height, depth);
#endif
	pixmap = gdk_pixmap_new(gtk_widget_get_window(window), rect.width,
			rect.width, -1);
	w = gdk_pixbuf_get_width(background);
	h = gdk_pixbuf_get_height(background);
	/* draw default color */
	gc = gdk_gc_new(pixmap);
	gdk_gc_set_rgb_fg_color(gc, &color);
	gdk_draw_rectangle(pixmap, gc, TRUE, 0, 0, rect.width, rect.height);
	/* draw background */
	for(j = 0; j < rect.height; j += h)
		for(i = 0; i < rect.width; i += w)
			gdk_draw_pixbuf(pixmap, NULL, background, 0, 0,
					i, j, w, h, GDK_RGB_DITHER_NONE, 0, 0);
	gdk_window_set_back_pixmap(gtk_widget_get_window(window), pixmap,
			FALSE);
	gdk_window_clear(gtk_widget_get_window(window));
	gdk_pixmap_unref(pixmap);
	gtkdemo->windows[gtkdemo->windows_cnt++] = window;
	if(gtkdemo->timeout == 0)
		gtkdemo->timeout = g_timeout_add(40, _gtk_demo_on_timeout,
				demo);
	return ret;
}


/* gtk_demo_remove */
static void _gtk_demo_remove(LockerDemo * demo, GtkWidget * window)
{
	GtkDemo * gtkdemo = demo->priv;
	size_t i;

	for(i = 0; i < gtkdemo->windows_cnt; i++)
		if(gtkdemo->windows[i] == window)
			gtkdemo->windows[i] = NULL;
	/* FIXME reorganize the table, free memory */
	/* de-register timeout if necessary */
	if(gtkdemo->timeout != 0)
	{
		for(i = 0; i < gtkdemo->windows_cnt; i++)
			if(gtkdemo->windows[i] != NULL)
				break;
		if(i == gtkdemo->windows_cnt)
		{
			g_source_remove(gtkdemo->timeout);
			gtkdemo->timeout = 0;
		}
	}
}


/* callbacks */
/* gtk_demo_on_timeout */
static gboolean _gtk_demo_on_timeout(gpointer data)
{
	LockerDemo * demo = data;
	GtkDemo * gtkdemo = demo->priv;
	size_t c;
	GtkWidget * window;
	GdkPixbuf * background = gtkdemo->images[GDI_BACKGROUND];
	gint back_width;
	gint back_height;
	GdkPixbuf * frame;
	GdkRectangle rect;
	int depth;
	GdkPixmap * pixmap;
	int j;
#define CYCLE_LEN 60
	double f;
	int i;
	double xmid, ymid;
	double radius;

	for(c = 0; c < gtkdemo->windows_cnt; c++)
	{
		if((window = gtkdemo->windows[c]) == NULL)
			break;
		gdk_window_get_geometry(gtk_widget_get_window(window), &rect.x,
				&rect.y, &rect.width, &rect.height, &depth);
		frame = gdk_pixbuf_new(GDK_COLORSPACE_RGB, 1, 8,
				rect.width, rect.height);
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() frame=%p\n", __func__,
				(void *)frame);
#endif

	back_width = gdk_pixbuf_get_width(background);
	back_height = gdk_pixbuf_get_height(background);
	for(j = 0; j < rect.height; j += back_height)
		for(i = 0; i < rect.width; i += back_width)
			gdk_pixbuf_copy_area(background, 0, 0,
					MIN(back_width, rect.width - i),
					MIN(back_height, rect.height - j),
					frame, i, j);

	f = (double) (gtkdemo->frame_num % CYCLE_LEN) / CYCLE_LEN;

	back_width = rect.width;
	back_height = rect.height;
	xmid = back_width / 2.0;
	ymid = back_height / 2.0;

	radius = MIN(xmid, ymid) / 2.0;

	for(i = 1; i < GDI_COUNT; i++)
	{
		double ang;
		int xpos, ypos;
		int iw, ih;
		double r;
		GdkRectangle r1, r2, dest;
		double k;

		ang = 2.0 * G_PI * (double) (i - 1) / (GDI_COUNT - 1) - f * 2.0
			* G_PI;

		iw = gdk_pixbuf_get_width(gtkdemo->images[i]);
		ih = gdk_pixbuf_get_height(gtkdemo->images[i]);

		r = radius + (radius / 3.0) * sin (f * 2.0 * G_PI);

		xpos = floor (xmid + r * cos (ang) - iw / 2.0 + 0.5);
		ypos = floor (ymid + r * sin (ang) - ih / 2.0 + 0.5);

		k = (i & 1) ? sin (f * 2.0 * G_PI) : cos (f * 2.0 * G_PI);
		k = 2.0 * k * k;
		k = MAX (0.25, k);

		r1.x = xpos;
		r1.y = ypos;
		r1.width = iw * k;
		r1.height = ih * k;

		r2.x = 0;
		r2.y = 0;
		r2.width = back_width;
		r2.height = back_height;

		if(gdk_rectangle_intersect(&r1, &r2, &dest))
			gdk_pixbuf_composite(gtkdemo->images[i], frame, dest.x,
					dest.y, dest.width, dest.height, xpos,
					ypos, k, k, GDK_INTERP_NEAREST,
					((i & 1)
					 ? MAX (127, fabs (255 * sin (f * 2.0 * G_PI)))
					 : MAX (127, fabs (255 * cos (f * 2.0 * G_PI)))));
	}
	pixmap = gdk_pixmap_new(gtk_widget_get_window(window), rect.width,
			rect.width, -1);
	gdk_draw_pixbuf(pixmap, NULL, frame, 0, 0, 0, 0, rect.width,
			rect.height, GDK_RGB_DITHER_NONE, 0, 0);
	gdk_window_set_back_pixmap(gtk_widget_get_window(window), pixmap,
			FALSE);
	gdk_window_clear(gtk_widget_get_window(window));
	gdk_pixmap_unref(pixmap);
	g_object_unref(frame);

	}
	gtkdemo->frame_num++;
	return TRUE;
}
