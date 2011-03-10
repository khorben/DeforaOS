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
#include <libintl.h>
#include <System.h>
#include "Panel.h"
#define _(string) gettext(string)


/* GSM */
/* private */
/* types */
typedef struct _GSM
{
	PanelAppletHelper * helper;
	GtkWidget * hbox;
	GtkWidget * image;
	GtkWidget * operator;
	guint timeout;
#if defined(__linux__)
	int fd;
#endif
} GSM;


/* prototypes */
static GtkWidget * _gsm_init(PanelApplet * applet);
static void _gsm_destroy(PanelApplet * applet);

static gboolean _gsm_get(GSM * gsm);
static void _gsm_set(GSM * gsm, gboolean on);
#if 0
static void _gsm_set_operator(GSM * gsm, char const * operator);
#endif

/* callbacks */
static gboolean _on_timeout(gpointer data);


/* public */
/* variables */
PanelApplet applet =
{
	NULL,
	"GSM",
	NULL,
	_gsm_init,
	_gsm_destroy,
	NULL,
	PANEL_APPLET_POSITION_END,
	FALSE,
	TRUE,
	NULL
};


/* private */
/* functions */
/* gsm_init */
static GtkWidget * _gsm_init(PanelApplet * applet)
{
	GSM * gsm;
	PangoFontDescription * desc;

	if((gsm = malloc(sizeof(*gsm))) == NULL)
		return NULL;
	applet->priv = gsm;
	gsm->helper = applet->helper;
	gsm->timeout = 0;
#if defined(__linux__)
	gsm->fd = -1;
#endif
	gsm->hbox = gtk_hbox_new(FALSE, 0);
	/* XXX find a better image */
	gsm->image = gtk_image_new_from_icon_name("phone",
			applet->helper->icon_size);
#if GTK_CHECK_VERSION(2, 12, 0)
	gtk_widget_set_tooltip_text(gsm->image, _("GSM is enabled"));
#endif
	gtk_widget_show(gsm->image);
	gtk_box_pack_start(GTK_BOX(gsm->hbox), gsm->image, FALSE, TRUE, 0);
	desc = pango_font_description_new();
	pango_font_description_set_weight(desc, PANGO_WEIGHT_BOLD);
	gsm->operator = gtk_label_new(NULL);
	gtk_widget_modify_font(gsm->operator, desc);
	gtk_widget_show(gsm->operator);
	pango_font_description_free(desc);
	gtk_box_pack_start(GTK_BOX(gsm->hbox), gsm->operator, FALSE, TRUE, 0);
	gsm->timeout = g_timeout_add(1000, _on_timeout, gsm);
	_on_timeout(gsm);
	return gsm->hbox;
}


/* gsm_destroy */
static void _gsm_destroy(PanelApplet * applet)
{
	GSM * gsm = applet->priv;

	if(gsm->timeout > 0)
		g_source_remove(gsm->timeout);
#if defined(__linux__)
	if(gsm->fd != -1)
		close(gsm->fd);
#endif
	free(gsm);
}


/* gsm_set */
static void _gsm_set(GSM * gsm, gboolean on)
{
	if(on == TRUE)
		gtk_widget_show(gsm->hbox);
	else
		gtk_widget_hide(gsm->hbox);
}


#if 0
/* gsm_set_operator */
static void _gsm_set_operator(GSM * gsm, char const * operator)
{
	gtk_label_set_text(GTK_LABEL(gsm->operator), operator);
}
#endif


/* callbacks */
/* on_timeout */
#if defined(__linux__)
static gboolean _gsm_get(GSM * gsm)
{
	/* XXX currently hard-coded for the Openmoko Freerunner */
	char const dv1[] = "/sys/bus/platform/devices/gta02-pm-gsm.0/"
		"power_on";
	char const dv2[] = "/sys/bus/platform/devices/neo1973-pm-gsm.0/"
		"power_on";
	char const * dev = dv1;
	char on;

	if(gsm->fd < 0)
	{
		if((gsm->fd = open(dev, O_RDONLY)) < 0)
		{
			dev = dv2;
			gsm->fd = open(dev, O_RDONLY);
		}
		if(gsm->fd < 0)
		{
			error_set_code(1, "%s: %s", dev, strerror(errno));
			return FALSE;
		}
	}
	errno = ENODATA; /* in case the pseudo-file is empty */
	if(lseek(gsm->fd, 0, SEEK_SET) != 0
			|| read(gsm->fd, &on, sizeof(on)) != 1)
	{
		error_set(1, "%s: %s", dev, strerror(errno));
		close(gsm->fd);
		gsm->fd = -1;
		return FALSE;
	}
	return (on == '1') ? TRUE : FALSE;
}
#else
static gboolean _gsm_get(GSM * gsm)
{
	/* FIXME not supported */
	return FALSE;
}
#endif


/* callbacks */
/* on_timeout */
static gboolean _on_timeout(gpointer data)
{
	GSM * gsm = data;

	_gsm_set(gsm, _gsm_get(gsm));
	return TRUE;
}
