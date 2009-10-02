/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Framer */
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
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/Xatom.h>
#include "framer.h"
#include "../config.h"

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))


/* Framer */
/* private */
/* types */
typedef enum _FramerAtom
{
	FA_NET_CURRENT_DESKTOP = 0,
	FA_NET_DESKTOP_GEOMETRY,
	FA_NET_DESKTOP_VIEWPORT,
	FA_NET_NUMBER_OF_DESKTOPS,
	FA_NET_SHOWING_DESKTOP,
	FA_NET_WM_WINDOW_TYPE,
	FA_NET_WM_WINDOW_TYPE_DESKTOP,
	FA_NET_WM_WINDOW_TYPE_DOCK,
	FA_NET_WM_WINDOW_TYPE_NORMAL,
	FA_NET_WORKAREA
} FramerAtom;
#define FA_LAST				FA_NET_WORKAREA
#define FA_COUNT			(FA_LAST + 1)
#define FA_NET_WM_WINDOW_TYPE_FIRST	FA_NET_WM_WINDOW_TYPE_DESKTOP
#define FA_NET_WM_WINDOW_TYPE_LAST	FA_NET_WM_WINDOW_TYPE_NORMAL

struct _Framer
{
	GdkDisplay * display;
	GdkScreen * screen;
	GdkWindow * root;
	int width;
	int height;
	Atom atom[FA_COUNT];
};


/* constants */
static char const * _framer_atom[FA_COUNT] =
{
	"_NET_CURRENT_DESKTOP",
	"_NET_DESKTOP_GEOMETRY",
	"_NET_DESKTOP_VIEWPORT",
	"_NET_NUMBER_OF_DESKTOPS",
	"_NET_SHOWING_DESKTOP",
	"_NET_WM_WINDOW_TYPE",
	"_NET_WM_WINDOW_TYPE_DESKTOP",
	"_NET_WM_WINDOW_TYPE_DOCK",
	"_NET_WM_WINDOW_TYPE_NORMAL",
	"_NET_WORKAREA"
};


/* prototypes */
static int _framer_error(char const * message, int ret);
static int _framer_get_window_property(Framer * framer, Window window,
		FramerAtom property, Atom atom, unsigned long * cnt,
		unsigned char ** ret);

/* callbacks */
static GdkFilterReturn _framer_filter(GdkXEvent * xevent, GdkEvent * event,
		gpointer data);


/* public */
/* functions */
/* framer_new */
static void _new_ewmh(Framer * framer);

Framer * framer_new(void)
{
	Framer * framer;

	if((framer = malloc(sizeof(*framer))) == NULL)
	{
		_framer_error(strerror(errno), 1);
		return NULL;
	}
	if((framer->display = gdk_display_get_default()) == NULL)
	{
		_framer_error("There is no default display", 1);
		free(framer);
		return NULL;
	}
	framer->screen = gdk_display_get_default_screen(framer->display);
	framer->root = gdk_screen_get_root_window(framer->screen);
	framer->width = gdk_screen_get_width(framer->screen);
	framer->height = gdk_screen_get_height(framer->screen);
	_new_ewmh(framer);
	/* register as window manager */
	XSelectInput(gdk_x11_display_get_xdisplay(framer->display),
			GDK_WINDOW_XWINDOW(framer->root), NoEventMask
			| SubstructureRedirectMask);
	gdk_window_add_filter(framer->root, _framer_filter, framer);
	return framer;
}

static void _new_ewmh(Framer * framer)
{
	long data[4];
	size_t i;
	long cnt = 0;

	memset(&data, 0, sizeof(data));
	for(i = 0; i < FA_COUNT; i++)
	{
		framer->atom[i] = gdk_x11_get_xatom_by_name_for_display(
				framer->display, _framer_atom[i]);
		switch(i)
		{
			case FA_NET_CURRENT_DESKTOP:
				cnt = 1;
				break;
			case FA_NET_NUMBER_OF_DESKTOPS:
				data[0] = 1;
				cnt = 1;
				break;
			case FA_NET_DESKTOP_GEOMETRY:
				data[0] = framer->width;
				data[1] = framer->height;
				cnt = 2;
				break;
			case FA_NET_DESKTOP_VIEWPORT:
				cnt = 2;
				break;
			case FA_NET_WORKAREA:
				data[2] = framer->width;
				data[3] = max(framer->height - 64, 1);
				cnt = 4;
				break;
			default:
				continue;
		}
		XChangeProperty(GDK_DISPLAY_XDISPLAY(framer->display),
				GDK_WINDOW_XWINDOW(framer->root),
				framer->atom[i], XA_CARDINAL, 32,
				PropModeReplace, (unsigned char *)&data, cnt);
		cnt = 0;
		memset(&data, 0, sizeof(data));
	}
}


/* framer_delete */
void framer_delete(Framer * framer)
{
	int i;

	for(i = 0; i < FA_COUNT; i++)
		XDeleteProperty(GDK_DISPLAY_XDISPLAY(framer->display),
				GDK_WINDOW_XWINDOW(framer->root),
				framer->atom[i]);
	free(framer);
}


/* accessors */
/* framer_set_show_desktop */
void framer_set_show_desktop(Framer * framer, gboolean shown)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%s)\n", __func__, shown ? "TRUE" : "FALSE");
#endif
	/* FIXME implement */
}


/* private */
/* functions */
/* framer_error */
static int _framer_error(char const * message, int ret)
{
	fprintf(stderr, "%s: %s\n", PACKAGE, message);
	return ret;
}


/* framer_get_window_property */
static int _framer_get_window_property(Framer * framer, Window window,
		FramerAtom property, Atom atom, unsigned long * cnt,
		unsigned char ** ret)
{
	int res;
	Atom type;
	int format;
	unsigned long bytes;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(window, %s, %lu)\n", __func__,
			_framer_atom[property], atom);
#endif
	gdk_error_trap_push();
	res = XGetWindowProperty(GDK_DISPLAY_XDISPLAY(framer->display), window,
			framer->atom[property], 0, G_MAXLONG, False, atom,
			&type, &format, cnt, &bytes, ret);
	if(gdk_error_trap_pop() != 0 || res != Success)
		return 1;
	if(type != atom)
	{
		if(*ret != NULL)
			XFree(*ret);
		*ret = NULL;
		return 1;
	}
	return 0;
}


/* callbacks */
/* framer_filter */
static GdkFilterReturn _filter_client_message(XClientMessageEvent * xclient,
		Framer * framer);
static GdkFilterReturn _filter_configure_notify(XConfigureEvent * xconfigure);
static GdkFilterReturn _filter_configure_request(
		XConfigureRequestEvent * xconfigure, Framer * framer);
static GdkFilterReturn _filter_create_notify(XCreateWindowEvent * xcreate);
static GdkFilterReturn _filter_destroy_notify(XDestroyWindowEvent * xdestroy);
static GdkFilterReturn _filter_leave_notify(XCrossingEvent * xleave);
static GdkFilterReturn _filter_map_notify(XMapEvent * xmap, Framer * framer);
static GdkFilterReturn _filter_map_request(XMapRequestEvent * xmap,
		Framer * framer);
static GdkFilterReturn _filter_motion_notify(XMotionEvent * xmotion);
static GdkFilterReturn _filter_property_notify(XPropertyEvent * xproperty,
		Framer * framer);
static GdkFilterReturn _filter_unmap_notify(XUnmapEvent * xmap);

static GdkFilterReturn _framer_filter(GdkXEvent * xevent, GdkEvent * event,
		gpointer data)
{
	Framer * framer = data;
	XEvent * xev = xevent;

	switch(xev->type)
	{
		case ClientMessage:
			return _filter_client_message(&xev->xclient, framer);
		case ConfigureNotify:
			return _filter_configure_notify(&xev->xconfigure);
		case ConfigureRequest:
			return _filter_configure_request(
					&xev->xconfigurerequest, framer);
		case CreateNotify:
			return _filter_create_notify(&xev->xcreatewindow);
		case DestroyNotify:
			return _filter_destroy_notify(&xev->xdestroywindow);
		case LeaveNotify:
			return _filter_leave_notify(&xev->xcrossing);
		case MapNotify:
			return _filter_map_notify(&xev->xmap, framer);
		case MapRequest:
			return _filter_map_request(&xev->xmaprequest, framer);
		case MotionNotify:
			return _filter_motion_notify(&xev->xmotion);
		case PropertyNotify:
			return _filter_property_notify(&xev->xproperty,
					framer);
		case UnmapNotify:
			return _filter_unmap_notify(&xev->xunmap);
		default:
#ifdef DEBUG
			fprintf(stderr, "DEBUG: %s() type=%d\n", __func__,
					xev->type);
#endif
			break;
	}
	return GDK_FILTER_CONTINUE;
}

static GdkFilterReturn _filter_client_message(XClientMessageEvent * xclient,
		Framer * framer)
{
	GdkAtom atom;
	size_t i;
	char const * name;
	char * p = NULL;

	for(i = 0; i < FA_COUNT; i++)
		if(xclient->message_type == framer->atom[i])
			break;
	switch(i)
	{
		case FA_NET_SHOWING_DESKTOP:
			framer_set_show_desktop(framer, TRUE);
			break;
		default:
			if(i < FA_COUNT)
				name = _framer_atom[i];
			else
			{
				atom = gdk_x11_xatom_to_atom_for_display(
						framer->display,
						xclient->message_type);
				p = gdk_atom_name(atom);
				name = p;
			}
			fprintf(stderr, "%s: %s: %s\n", PACKAGE, name,
					"Unsupported message");
			if(p != NULL)
				g_free(p);
			break;
	}
	return GDK_FILTER_REMOVE;
}

static GdkFilterReturn _filter_configure_notify(XConfigureEvent * xconfigure)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	return GDK_FILTER_CONTINUE;
}

static GdkFilterReturn _filter_configure_request(
		XConfigureRequestEvent * xconfigure, Framer * framer)
{
	FramerAtom type = FA_NET_WM_WINDOW_TYPE_NORMAL;
	Atom typehint;
	Atom * p;
	unsigned long cnt = 0;
	int i;
	XWindowChanges wc;
	unsigned long mask = xconfigure->value_mask;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() (%d,%d) %dx%d %lu\n", __func__,
			xconfigure->x, xconfigure->y, xconfigure->width,
			xconfigure->height, xconfigure->value_mask);
#endif
	if(_framer_get_window_property(framer, xconfigure->window,
		FA_NET_WM_WINDOW_TYPE, XA_ATOM, &cnt, (void*)&p) == 0)
	{
		typehint = *p;
		XFree(p);
		for(i = FA_NET_WM_WINDOW_TYPE_FIRST;
				i < FA_NET_WM_WINDOW_TYPE_LAST; i++)
			if(typehint == framer->atom[i])
			{
				type = i;
				break;
			}
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() %s\n", __func__, _framer_atom[type]);
#endif
	memset(&wc, 0, sizeof(wc));
#ifndef EMBEDDED
	if(xconfigure->value_mask & CWX)
		wc.x = max(xconfigure->x, 0);
	if(xconfigure->value_mask & CWY)
		wc.y = max(xconfigure->y, 0);
	if(xconfigure->value_mask & CWWidth)
		wc.width = min(xconfigure->width, framer->width);
	if(xconfigure->value_mask & CWHeight)
		wc.height = min(xconfigure->height, framer->height - 64);
#else
	if(type == FA_NET_WM_WINDOW_TYPE_DOCK)
	{
		if(xconfigure->value_mask & (CWX | CWWidth))
		{
			wc.x = 0;
			wc.width = framer->width;
			mask |= CWX | CWWidth;
		}
		if(xconfigure->value_mask & (CWY | CWHeight))
		{
			wc.y = framer->height - 64;
			wc.height = 64;
			mask |= CWY | CWHeight;
		}
	}
	else /* other than dock window */
	{
		wc.x = 0;
		wc.width = framer->width;
		wc.y = 0;
		wc.height = framer->height - 64;
		mask |= CWX | CWWidth | CWY | CWHeight;
	}
	if(xconfigure->value_mask & CWBorderWidth)
		wc.border_width = 0;
#endif
	XConfigureWindow(xconfigure->display, xconfigure->window, mask, &wc);
	return GDK_FILTER_REMOVE;
}

static GdkFilterReturn _filter_create_notify(XCreateWindowEvent * xcreate)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	return GDK_FILTER_CONTINUE;
}

static GdkFilterReturn _filter_destroy_notify(XDestroyWindowEvent * xdestroy)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	return GDK_FILTER_CONTINUE;
}

static GdkFilterReturn _filter_leave_notify(XCrossingEvent * xleave)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	return GDK_FILTER_CONTINUE;
}

static GdkFilterReturn _filter_map_notify(XMapEvent * xmap, Framer * framer)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	return GDK_FILTER_CONTINUE;
}

static GdkFilterReturn _filter_map_request(XMapRequestEvent * xmap,
		Framer * framer)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	XMapWindow(xmap->display, xmap->window);
	return GDK_FILTER_CONTINUE;
}

static GdkFilterReturn _filter_motion_notify(XMotionEvent * xmotion)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	return GDK_FILTER_CONTINUE;
}

static GdkFilterReturn _filter_property_notify(XPropertyEvent * xproperty,
		Framer * framer)
{
	GdkAtom atom;
	char * name;

	atom = gdk_x11_xatom_to_atom_for_display(framer->display,
		xproperty->atom);
	name = gdk_atom_name(atom);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() %s\n", __func__, name);
#endif
	g_free(name);
	return GDK_FILTER_CONTINUE;
}

static GdkFilterReturn _filter_unmap_notify(XUnmapEvent * xmap)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	return GDK_FILTER_CONTINUE;
}
