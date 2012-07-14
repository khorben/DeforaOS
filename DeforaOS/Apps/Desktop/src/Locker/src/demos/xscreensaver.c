/* $Id$ */
/* Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org> */
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
 * - send SIGSTOP/SIGSTART to the child process when its window is removed? */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <gdk/gdkx.h>
#include <System.h>
#include "Locker/demo.h"
#include "../../config.h"

#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif


/* XScreensaver */
/* private */
/* types */
typedef struct _LockerDemo
{
	LockerDemoHelper * helper;
} XScreensaver;


/* prototypes */
/* plug-in */
static XScreensaver * _xscreensaver_init(LockerDemoHelper * helper);
static void _xscreensaver_destroy(XScreensaver * xscreensaver);
static int _xscreensaver_add(XScreensaver * xscreensaver, GtkWidget * window);
static void _xscreensaver_remove(XScreensaver * xscreensaver,
		GtkWidget * window);
static void _xscreensaver_start(XScreensaver * xscreensaver);
static void _xscreensaver_stop(XScreensaver * xscreensaver);


/* public */
/* variables */
/* plug-in */
LockerDemoDefinition plugin =
{
	"XScreensaver",
	NULL,
	NULL,
	_xscreensaver_init,
	_xscreensaver_destroy,
	_xscreensaver_add,
	_xscreensaver_remove,
	_xscreensaver_start,
	_xscreensaver_stop
};


/* private */
/* functions */
/* plug-in */
/* xscreensaver_init */
static XScreensaver * _xscreensaver_init(LockerDemoHelper * helper)
{
	XScreensaver * xscreensaver;

	if((xscreensaver = object_new(sizeof(*xscreensaver))) == NULL)
		return NULL;
	xscreensaver->helper = helper;
	return xscreensaver;
}


/* xscreensaver_destroy */
static void _xscreensaver_destroy(XScreensaver * xscreensaver)
{
	object_delete(xscreensaver);
}


/* xscreensaver_add */
static int _xscreensaver_add(XScreensaver * xscreensaver, GtkWidget * window)
{
	int ret = 0;
	LockerDemoHelper * helper = xscreensaver->helper;
	unsigned int id = GDK_WINDOW_XWINDOW(window->window);
	GError * error = NULL;
	char * argv[] = { NULL, "-window-id", NULL, NULL };
	char const * p;
	char buf[16];

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() %u\n", __func__, id);
#endif
	if((p = helper->config_get(helper->locker, "xscreensaver",
					"xscreensaver")) != NULL)
		argv[0] = strdup(p);
	else
		argv[0] = strdup(PREFIX "/libexec/xscreensaver/bsod");
	snprintf(buf, sizeof(buf), "%u", id);
	argv[2] = buf;
	if(argv[0] == NULL)
		return -helper->error(NULL, strerror(errno), 1);
	if(g_spawn_async(NULL, argv, NULL, 0, NULL, NULL, NULL, &error) != TRUE)
	{
		ret = helper->error(NULL, error->message, 1);
		g_error_free(error);
	}
	return ret;
}


/* xscreensaver_remove */
static void _xscreensaver_remove(XScreensaver * xscreensaver,
		GtkWidget * window)
{
	/* FIXME implement */
}


/* xscreensaver_start */
static void _xscreensaver_start(XScreensaver * xscreensaver)
{
	/* FIXME implement */
}


/* xscreensaver_stop */
static void _xscreensaver_stop(XScreensaver * xscreensaver)
{
	/* FIXME implement */
}
