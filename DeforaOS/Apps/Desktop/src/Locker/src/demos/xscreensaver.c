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



#include <stdlib.h>
#include <stdio.h>
#include <gdk/gdkx.h>
#include "Locker/demo.h"
#include "../../config.h"

#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif


/* XScreensaver */
/* private */
/* types */


/* prototypes */
/* plug-in */
static int _xscreensaver_init(LockerDemo * demo);
static int _xscreensaver_add(LockerDemo * demo, GtkWidget * window);
static void _xscreensaver_remove(LockerDemo * demo, GtkWidget * window);


/* public */
/* variables */
/* plug-in */
LockerDemo demo =
{
	NULL,
	"XScreensaver",
	_xscreensaver_init,
	NULL,
	_xscreensaver_add,
	_xscreensaver_remove,
	NULL
};


/* private */
/* functions */
/* plug-in */
/* xscreensaver_init */
static int _xscreensaver_init(LockerDemo * demo)
{
	demo->priv = NULL;
	return 0;
}


/* xscreensaver_add */
static int _xscreensaver_add(LockerDemo * demo, GtkWidget * window)
{
	int ret = 0;
	LockerDemoHelper * helper = demo->helper;
	unsigned int id = GDK_WINDOW_XWINDOW(window->window);
	GError * error = NULL;
	char * argv[] = { NULL, "-window-id", NULL, NULL };
	char buf[16];

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() %u\n", __func__, id);
#endif
	if((argv[0] = helper->config_get(helper->locker, "xscreensaver",
					"xscreensaver")) == NULL)
		argv[0] = PREFIX "/libexec/xscreensaver/bsod";
	snprintf(buf, sizeof(buf), "%u", id);
	argv[2] = buf;
	if(g_spawn_async(NULL, argv, NULL, 0, NULL, NULL, NULL, &error) != TRUE)
	{
		ret = helper->error(NULL, error->message, 1);
		g_error_free(error);
	}
	return ret;
}


/* xscreensaver_remove */
static void _xscreensaver_remove(LockerDemo * demo, GtkWidget * window)
{
	/* FIXME implement */
}
