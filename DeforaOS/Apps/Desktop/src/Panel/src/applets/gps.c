/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Panel */
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



#if defined(__linux__)
# include <fcntl.h>
# include <unistd.h>
# include <string.h>
# include <errno.h>
#endif
#include <stdlib.h>
#include <System.h>
#include "Panel.h"


/* GPS */
/* private */
/* types */
typedef struct _GPS
{
	PanelAppletHelper * helper;
	GtkWidget * image;
	guint timeout;
#if defined(__linux__)
	int fd;
#endif
} GPS;


/* prototypes */
static GtkWidget * _gps_init(PanelApplet * applet);
static void _gps_destroy(PanelApplet * applet);

static gboolean _gps_get(GPS * gps);
static void _gps_set(GPS * gps, gboolean on);

/* callbacks */
static gboolean _on_timeout(gpointer data);


/* public */
/* variables */
PanelApplet applet =
{
	NULL,
	"GPS",
	NULL,
	_gps_init,
	_gps_destroy,
	NULL,
	PANEL_APPLET_POSITION_END,
	FALSE,
	TRUE,
	NULL
};


/* private */
/* functions */
/* gps_init */
static GtkWidget * _gps_init(PanelApplet * applet)
{
	GPS * gps;

	if((gps = malloc(sizeof(*gps))) == NULL)
		return NULL;
	applet->priv = gps;
	gps->helper = applet->helper;
	gps->timeout = 0;
#if defined(__linux__)
	gps->fd = -1;
#endif
	/* XXX find a better image */
	gps->image = gtk_image_new_from_icon_name("network-wireless",
			applet->helper->icon_size);
	gps->timeout = g_timeout_add(1000, _on_timeout, gps);
	_on_timeout(gps);
	return gps->image;
}


/* gps_destroy */
static void _gps_destroy(PanelApplet * applet)
{
	GPS * gps = applet->priv;

	if(gps->timeout > 0)
		g_source_remove(gps->timeout);
#if defined(__linux__)
	if(gps->fd != -1)
		close(gps->fd);
#endif
	free(gps);
}


/* gps_set */
static void _gps_set(GPS * gps, gboolean on)
{
	if(on == TRUE)
		gtk_widget_show(gps->image);
	else
		gtk_widget_hide(gps->image);
}


/* callbacks */
/* on_timeout */
#if defined(__linux__)
static gboolean _gps_get(GPS * gps)
{
	/* XXX currently hard-coded for the Openmoko Freerunner */
	const char path1[] = "/sys/bus/platform/drivers/neo1973-pm-gps/"
		"neo1973-pm-gps.0/power_on";
	const char path2[] = "/sys/bus/platform/drivers/neo1973-pm-gps/"
		"neo1973-pm-gps.0/pwron";
	char on;

	if(gps->fd == -1 && (gps->fd = open(path1, O_RDONLY)) == -1
			&& (gps->fd = open(path2, O_RDONLY)) == -1)
	{
		error_set("%s: %s", path1, strerror(errno));
		return FALSE;
	}
	errno = ENODATA; /* in case the pseudo-file is empty */
	if(lseek(gps->fd, 0, SEEK_SET) != 0
			|| read(gps->fd, &on, sizeof(on)) != 1)
	{
		error_set("%s: %s", path1, strerror(errno));
		close(gps->fd);
		gps->fd = -1;
		return FALSE;
	}
	return (on == '1') ? TRUE : FALSE;
}
#else
static gboolean _gps_get(GPS * gps)
{
	/* FIXME not supported */
	return FALSE;
}
#endif


/* callbacks */
/* on_timeout */
static gboolean _on_timeout(gpointer data)
{
	GPS * gps = data;

	_gps_set(gps, _gps_get(gps));
	return TRUE;
}
