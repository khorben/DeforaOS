/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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



#if defined(__NetBSD__) || defined(__linux__)
# include <sys/types.h>
# include <sys/ioctl.h>
# include <net/if.h>
# include <unistd.h>
# include <stdio.h>
# include <string.h>
# include <errno.h>
#endif
#include <stdlib.h>
#include <libintl.h>
#include <System.h>
#include "Panel.h"
#define _(string) gettext(string)


/* USB */
/* private */
/* types */
typedef struct _USB
{
	PanelAppletHelper * helper;
	GtkWidget * image;
	guint timeout;
#if defined(__NetBSD__) || defined(__linux__)
	int fd;
#endif
} USB;


/* prototypes */
static GtkWidget * _usb_init(PanelApplet * applet);
static void _usb_destroy(PanelApplet * applet);

static gboolean _usb_get(USB * usb);
static void _usb_set(USB * usb, gboolean on);

/* callbacks */
static gboolean _on_timeout(gpointer data);


/* public */
/* variables */
PanelApplet applet =
{
	NULL,
	"USB",
	NULL,
	_usb_init,
	_usb_destroy,
	NULL,
	FALSE,
	TRUE,
	NULL
};


/* private */
/* functions */
/* usb_init */
static GtkWidget * _usb_init(PanelApplet * applet)
{
	USB * usb;
#if GTK_CHECK_VERSION(2, 12, 0)
	char const * tooltip = NULL;
#endif

	if((usb = malloc(sizeof(*usb))) == NULL)
		return NULL;
	applet->priv = usb;
	usb->helper = applet->helper;
	usb->timeout = 0;
#if defined(__NetBSD__) || defined(__linux__)
	usb->fd = -1;
	tooltip = _("USB networking device connected");
#endif
	usb->image = gtk_image_new_from_icon_name("panel-applet-usb",
			applet->helper->icon_size);
#if GTK_CHECK_VERSION(2, 12, 0)
	if(tooltip != NULL)
		gtk_widget_set_tooltip_text(usb->image, tooltip);
#endif
	usb->timeout = g_timeout_add(1000, _on_timeout, usb);
	_on_timeout(usb);
	return usb->image;
}


/* usb_destroy */
static void _usb_destroy(PanelApplet * applet)
{
	USB * usb = applet->priv;

	if(usb->timeout > 0)
		g_source_remove(usb->timeout);
#if defined(__NetBSD__) || defined(__linux__)
	if(usb->fd >= 0)
		close(usb->fd);
#endif
	free(usb);
}


/* usb_set */
static void _usb_set(USB * usb, gboolean on)
{
	if(on == TRUE)
		gtk_widget_show(usb->image);
	else
		gtk_widget_hide(usb->image);
}


/* callbacks */
/* on_timeout */
#if defined(__NetBSD__) || defined(__linux__)
static gboolean _usb_get(USB * usb)
{
	struct ifreq ifr;
# if defined(__NetBSD__)
	const char name[] = "cdce0";
# elif defined(__linux__)
	const char name[] = "usb0";
#endif

	if(usb->fd < 0 && (usb->fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		error_set("%s: %s", "socket", strerror(errno));
		return FALSE;
	}
	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", name);
	if(ioctl(usb->fd, SIOCGIFFLAGS, &ifr) == -1)
	{
		error_set("%s: %s", name, strerror(errno));
		close(usb->fd);
		usb->fd = -1;
		return FALSE;
	}
	close(usb->fd);
	usb->fd = -1;
	return TRUE;
}
#else
static gboolean _usb_get(USB * usb)
{
	/* FIXME not supported */
	return FALSE;
}
#endif


/* callbacks */
/* on_timeout */
static gboolean _on_timeout(gpointer data)
{
	USB * usb = data;

	_usb_set(usb, _usb_get(usb));
	return TRUE;
}
