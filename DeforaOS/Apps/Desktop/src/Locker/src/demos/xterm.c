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


/* XTerm */
/* private */
/* types */
typedef struct _LockerDemo
{
	LockerDemoHelper * helper;
} XTerm;


/* prototypes */
/* plug-in */
static XTerm * _xterm_init(LockerDemoHelper * helper);
static void _xterm_destroy(XTerm * xterm);
static int _xterm_add(XTerm * xterm, GtkWidget * window);
static void _xterm_remove(XTerm * xterm, GtkWidget * window);
static void _xterm_start(XTerm * xterm);
static void _xterm_stop(XTerm * xterm);


/* public */
/* variables */
/* plug-in */
LockerDemoDefinition demo =
{
	"XTerm",
	NULL,
	NULL,
	_xterm_init,
	_xterm_destroy,
	_xterm_add,
	_xterm_remove
};


/* private */
/* functions */
/* plug-in */
/* xterm_init */
static XTerm * _xterm_init(LockerDemoHelper * helper)
{
	XTerm * xterm;

	if((xterm = object_new(sizeof(*xterm))) == NULL)
		return NULL;
	xterm->helper = helper;
	return xterm;
}


/* xterm_destroy */
static void _xterm_destroy(XTerm * xterm)
{
	object_delete(xterm);
}


/* xterm_add */
static int _xterm_add(XTerm * xterm, GtkWidget * window)
{
	int ret = 0;
	LockerDemoHelper * helper = xterm->helper;
	unsigned int id = GDK_WINDOW_XWINDOW(window->window);
	GError * error = NULL;
	char * argv[] = { NULL, "-into", NULL, "-e", NULL, NULL };
	char const * p;
	char buf[16];

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() %u\n", __func__, id);
#endif
	if((p = helper->config_get(helper->locker, "xterm", "xterm")) != NULL)
		argv[0] = strdup(p);
	else
		argv[0] = strdup(PREFIX "/bin/xterm");
	if((p = helper->config_get(helper->locker, "xterm", "command")) != NULL)
		argv[4] = strdup(p);
	else
		argv[4] = strdup("top");
	snprintf(buf, sizeof(buf), "%u", id);
	argv[2] = buf;
	if(argv[0] == NULL || argv[4] == NULL)
	{
		free(argv[0]);
		free(argv[4]);
		return -helper->error(NULL, strerror(errno), 1);
	}
	if(g_spawn_async(NULL, argv, NULL, 0, NULL, NULL, NULL, &error) != TRUE)
	{
		ret = -helper->error(NULL, error->message, 1);
		g_error_free(error);
	}
	return ret;
}


/* xterm_remove */
static void _xterm_remove(XTerm * xterm, GtkWidget * window)
{
	/* FIXME implement */
}


/* xterm_start */
static void _xterm_start(XTerm * xterm)
{
	/* FIXME implement */
}


/* xterm_stop */
static void _xterm_stop(XTerm * xterm)
{
	/* FIXME implement */
}
