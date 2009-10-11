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
	FA_NET_ACTIVE_WINDOW = 0,
	FA_NET_CLIENT_LIST,
	FA_NET_CURRENT_DESKTOP,
	FA_NET_DESKTOP_GEOMETRY,
	FA_NET_DESKTOP_VIEWPORT,
	FA_NET_NUMBER_OF_DESKTOPS,
	FA_NET_SHOWING_DESKTOP,
	FA_NET_SUPPORTED,
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
	/* essential */
	GdkDisplay * display;
	GdkScreen * screen;
	GdkWindow * root;

	/* screen */
	int width;
	int height;

	/* atoms */
	Atom atom[FA_COUNT];

	/* client list */
	Window * window;
	size_t window_cnt;
};


/* constants */
static char const * _framer_atom[FA_COUNT] =
{
	"_NET_ACTIVE_WINDOW",
	"_NET_CLIENT_LIST",
	"_NET_CURRENT_DESKTOP",
	"_NET_DESKTOP_GEOMETRY",
	"_NET_DESKTOP_VIEWPORT",
	"_NET_NUMBER_OF_DESKTOPS",
	"_NET_SHOWING_DESKTOP",
	"_NET_SUPPORTED",
	"_NET_WM_WINDOW_TYPE",
	"_NET_WM_WINDOW_TYPE_DESKTOP",
	"_NET_WM_WINDOW_TYPE_DOCK",
	"_NET_WM_WINDOW_TYPE_NORMAL",
	"_NET_WORKAREA"
};


/* prototypes */
static int _framer_error(char const * message, int ret);
static int _framer_window_get_property(Framer * framer, Window window,
		FramerAtom property, Atom atom, unsigned long * cnt,
		unsigned char ** ret);
static int _framer_window_is_manageable(Framer * framer, Window window);
static int _framer_window_set_property(Framer * framer, Window window,
		FramerAtom property, Atom atom, unsigned long cnt,
		unsigned char * data);

/* callbacks */
static GdkFilterReturn _framer_filter(GdkXEvent * xevent, GdkEvent * event,
		gpointer data);


/* public */
/* functions */
/* framer_new */
static void _new_atoms(Framer * framer);
static void _new_windows(Framer * framer);
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
	_new_atoms(framer);
	_new_windows(framer);
	_new_ewmh(framer);
	/* register as window manager */
	XSelectInput(gdk_x11_display_get_xdisplay(framer->display),
			GDK_WINDOW_XWINDOW(framer->root), NoEventMask
			| SubstructureNotifyMask
			| SubstructureRedirectMask);
	gdk_window_add_filter(NULL, _framer_filter, framer);
	return framer;
}

static void _new_atoms(Framer * framer)
{
	size_t i;

	for(i = 0; i < FA_COUNT; i++)
		framer->atom[i] = gdk_x11_get_xatom_by_name_for_display(
				framer->display, _framer_atom[i]);
}

static void _new_windows(Framer * framer)
{
	Window root;
	Window parent;
	Window * children = NULL;
	unsigned int nchildren = 0;
	unsigned int i;

	framer->window = NULL;
	framer->window_cnt = 0;
	if(XQueryTree(GDK_DISPLAY_XDISPLAY(framer->display),
				GDK_WINDOW_XWINDOW(framer->root), &root,
				&parent, &children, &nchildren) == 0)
	{
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() failed\n", __func__);
#endif
		return;
	}
	for(i = 0; i < nchildren; i++)
		framer_window_add(framer, children[i]);
	XFree(children);
}

static void _new_ewmh(Framer * framer)
{
	long data[4];
	size_t i;
	Atom type = XA_CARDINAL;
	unsigned char * p = (unsigned char *)&data;
	long cnt = 0;

	memset(&data, 0, sizeof(data));
	for(i = 0; i < FA_COUNT; i++)
	{
		switch(i)
		{
			case FA_NET_ACTIVE_WINDOW:
				type = XA_WINDOW;
				data[0] = None;
				cnt = 1;
				break;
			case FA_NET_CLIENT_LIST:
				type = XA_WINDOW;
				p = (unsigned char *)framer->window;
				cnt = framer->window_cnt;
				break;
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
			case FA_NET_SUPPORTED:
				type = XA_ATOM;
				p = (unsigned char *)framer->atom;
				cnt = FA_COUNT;
				break;
			case FA_NET_WORKAREA:
				data[2] = framer->width;
				data[3] = max(framer->height - 64, 1);
				cnt = 4;
				break;
			default:
				continue;
		}
		_framer_window_set_property(framer, GDK_WINDOW_XWINDOW(
					framer->root), i, type, cnt, p);
		type = XA_CARDINAL;
		cnt = 0;
		memset(&data, 0, sizeof(data));
		p = (unsigned char *)&data;
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
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%s)\n", __func__, shown ? "TRUE" : "FALSE");
#endif
	if(shown != TRUE)
		return;
	for(i = 0; i < framer->window_cnt; i++)
		framer_window_iconify(framer, framer->window[i]);
}


/* framer_window_set_active */
int framer_window_set_active(Framer * framer, Window window)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, (int)window);
#endif
	XMapRaised(GDK_DISPLAY_XDISPLAY(framer->display), window);
	return _framer_window_set_property(framer, window, FA_NET_ACTIVE_WINDOW,
			XA_WINDOW, 1, (unsigned char *)&window);
}


/* useful */
/* framer_window_add */
int framer_window_add(Framer * framer, Window window)
{
	size_t i;
	Window * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, (int)window);
#endif
	if(!_framer_window_is_manageable(framer, window))
		return 0;
	for(i = 0; i < framer->window_cnt; i++)
		if(framer->window[i] == window)
			return 0;
	if((p = realloc(framer->window, sizeof(*p) * (framer->window_cnt + 1)))
			== NULL)
		return -1;
	framer->window = p;
	p = &framer->window[framer->window_cnt++];
	*p = window;
	_framer_window_set_property(framer, GDK_WINDOW_XWINDOW(framer->root),
			FA_NET_CLIENT_LIST, XA_WINDOW, framer->window_cnt,
			(unsigned char *)framer->window);
	return 0;
}


/* framer_window_iconify */
int framer_window_iconify(Framer * framer, Window window)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, (int)window);
#endif
	gdk_error_trap_push();
	XUnmapWindow(GDK_DISPLAY_XDISPLAY(framer->display), window);
	if(gdk_error_trap_pop() != 0)
		return 1;
	return 0;
}


/* framer_window_remove */
int framer_window_remove(Framer * framer, Window window)
{
	size_t i;
	Window * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, (int)window);
#endif
	for(i = 0; i < framer->window_cnt; i++)
		if(framer->window[i] == window)
			break;
	if(i == framer->window_cnt) /* we don't know this window anyway */
		return 0;
	memmove(&framer->window[i], &framer->window[i + 1],
			(--framer->window_cnt - i) * sizeof(*p));
	if((p = realloc(framer->window, framer->window_cnt * sizeof(*p)))
			!= NULL) /* we can ignore errors */
		framer->window = p;
	_framer_window_set_property(framer, GDK_WINDOW_XWINDOW(framer->root),
			FA_NET_CLIENT_LIST, XA_WINDOW, framer->window_cnt,
			(unsigned char *)framer->window);
	return 0;
}


/* private */
/* functions */
/* framer_error */
static int _framer_error(char const * message, int ret)
{
	fprintf(stderr, "%s: %s\n", PACKAGE, message);
	return ret;
}


/* framer_window_get_property */
static int _framer_window_get_property(Framer * framer, Window window,
		FramerAtom property, Atom atom, unsigned long * cnt,
		unsigned char ** ret)
{
	int res;
	Atom type;
	int format;
	unsigned long bytes;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d, %s, %lu)\n", __func__, (int)window,
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


/* framer_window_is_manageable */
static int _framer_window_is_manageable(Framer * framer, Window window)
{
	Atom typehint;
	Atom * p = NULL;
	unsigned long cnt = 0;

	if(_framer_window_get_property(framer, window, FA_NET_WM_WINDOW_TYPE,
				XA_ATOM, &cnt, (void*)&p) != 0)
		return 0;
	typehint = *p;
	XFree(p);
	return typehint == framer->atom[FA_NET_WM_WINDOW_TYPE_NORMAL] ? 1 : 0;
}


/* framer_window_set_property */
static int _framer_window_set_property(Framer * framer, Window window,
		FramerAtom property, Atom atom, unsigned long cnt,
		unsigned char * data)
{
	int res;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d, %s, %lu, %lu)\n", __func__,
			(int)window, _framer_atom[property], atom, cnt);
#endif
	gdk_error_trap_push();
	res = XChangeProperty(GDK_DISPLAY_XDISPLAY(framer->display), window,
			framer->atom[property], atom, 32, PropModeReplace,
			data, cnt);
	if(gdk_error_trap_pop() != 0 || res != Success)
		return 1;
	return 0;
}


/* callbacks */
/* framer_filter */
static GdkFilterReturn _filter_client_message(XClientMessageEvent * xclient,
		Framer * framer);
static GdkFilterReturn _filter_configure_notify(XConfigureEvent * xconfigure);
static GdkFilterReturn _filter_configure_request(
		XConfigureRequestEvent * xconfigure, Framer * framer);
static GdkFilterReturn _filter_create_notify(XCreateWindowEvent * xcreate,
		Framer * framer);
static GdkFilterReturn _filter_destroy_notify(XDestroyWindowEvent * xdestroy,
		Framer * framer);
static GdkFilterReturn _filter_leave_notify(XCrossingEvent * xleave);
static GdkFilterReturn _filter_map_notify(XMapEvent * xmap);
static GdkFilterReturn _filter_map_request(XMapRequestEvent * xmap);
static GdkFilterReturn _filter_motion_notify(XMotionEvent * xmotion);
static GdkFilterReturn _filter_property_notify(XPropertyEvent * xproperty,
		Framer * framer);
static GdkFilterReturn _filter_unmap_notify(XUnmapEvent * xunmap,
		Framer * framer);

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
			return _filter_create_notify(&xev->xcreatewindow,
					framer);
		case DestroyNotify:
			return _filter_destroy_notify(&xev->xdestroywindow,
					framer);
		case LeaveNotify:
			return _filter_leave_notify(&xev->xcrossing);
		case MapNotify:
			return _filter_map_notify(&xev->xmap);
		case MapRequest:
			return _filter_map_request(&xev->xmaprequest);
		case MotionNotify:
			return _filter_motion_notify(&xev->xmotion);
		case PropertyNotify:
			return _filter_property_notify(&xev->xproperty,
					framer);
		case UnmapNotify:
			return _filter_unmap_notify(&xev->xunmap, framer);
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
		case FA_NET_ACTIVE_WINDOW:
			framer_window_set_active(framer, xclient->window);
			break;
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
	return GDK_FILTER_CONTINUE;
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
	if(_framer_window_get_property(framer, xconfigure->window,
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
		wc.x = xconfigure->x;
		wc.y = xconfigure->y;
		wc.width = xconfigure->width;
		wc.height = xconfigure->height;
	}
	else
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
	gdk_error_trap_push();
	XConfigureWindow(xconfigure->display, xconfigure->window, mask, &wc);
	gdk_error_trap_pop();
	return GDK_FILTER_CONTINUE;
}

static GdkFilterReturn _filter_create_notify(XCreateWindowEvent * xcreate,
		Framer * framer)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	framer_window_add(framer, xcreate->window);
	return GDK_FILTER_CONTINUE;
}

static GdkFilterReturn _filter_destroy_notify(XDestroyWindowEvent * xdestroy,
		Framer * framer)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	framer_window_remove(framer, xdestroy->window);
	return GDK_FILTER_CONTINUE;
}

static GdkFilterReturn _filter_leave_notify(XCrossingEvent * xleave)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	return GDK_FILTER_CONTINUE;
}

static GdkFilterReturn _filter_map_notify(XMapEvent * xmap)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	return GDK_FILTER_CONTINUE;
}

static GdkFilterReturn _filter_map_request(XMapRequestEvent * xmap)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	gdk_error_trap_push();
	XMapWindow(xmap->display, xmap->window);
	gdk_error_trap_pop(); /* we ignore errors */
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
	fprintf(stderr, "DEBUG: %s() %s, %s\n", __func__, name,
			xproperty->send_event ? "TRUE" : "FALSE");
#endif
	g_free(name);
	return GDK_FILTER_CONTINUE;
}

static GdkFilterReturn _filter_unmap_notify(XUnmapEvent * xunmap,
		Framer * framer)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	return GDK_FILTER_CONTINUE;
}
