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
/* TODO:
 * - also let the timeout be configured */



#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <libintl.h>
#include "Panel.h"
#define _(string) gettext(string)


/* Clock */
/* private */
/* types */
typedef struct _Clock
{
	PanelAppletHelper * helper;
	char const * format;
	GtkWidget * label;
	guint timeout;
} Clock;


/* prototypes */
static GtkWidget * _clock_init(PanelApplet * applet);
static void _clock_destroy(PanelApplet * applet);

/* callbacks */
static gboolean _on_timeout(gpointer data);


/* public */
/* variables */
PanelApplet applet =
{
	NULL,
	"Clock",
	NULL,
	_clock_init,
	_clock_destroy,
	NULL,
	PANEL_APPLET_POSITION_LAST,
	FALSE,
	TRUE,
	NULL
};


/* private */
/* functions */
/* clock_init */
static GtkWidget * _clock_init(PanelApplet * applet)
{
	GtkWidget * ret;
	Clock * clock;
#ifdef EMBEDDED
	PangoFontDescription * desc;
#endif

	if((clock = malloc(sizeof(*clock))) == NULL)
		return NULL;
	applet->priv = clock;
	clock->helper = applet->helper;
	clock->label = gtk_label_new(" \n ");
	if((clock->format = applet->helper->config_get(applet->helper->panel,
					"clock", "format")) == NULL)
#ifdef EMBEDDED
		clock->format = _("%H:%M");
	ret = clock->label;
	desc = pango_font_description_new();
	pango_font_description_set_weight(desc, PANGO_WEIGHT_BOLD);
	gtk_widget_modify_font(clock->label, desc);
	pango_font_description_free(desc);
#else
	{
		if(clock->helper->icon_size == GTK_ICON_SIZE_LARGE_TOOLBAR)
			clock->format = _("%H:%M:%S\n%d/%m/%Y");
		else
			clock->format = _("%H:%M");
	}
	ret = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(ret), GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(ret), clock->label);
#endif
	gtk_label_set_justify(GTK_LABEL(clock->label), GTK_JUSTIFY_CENTER);
	clock->timeout = g_timeout_add(1000, _on_timeout, clock);
	_on_timeout(clock);
	gtk_widget_show_all(ret);
	return ret;
}


/* clock_destroy */
static void _clock_destroy(PanelApplet * applet)
{
	Clock * clock = applet->priv;

	g_source_remove(clock->timeout);
	free(clock);
}


/* callbacks */
/* on_timeout */
static gboolean _on_timeout(gpointer data)
{
	Clock * clock = data;
	struct timeval tv;
	time_t t;
	struct tm tm;
	char buf[32];

	if(gettimeofday(&tv, NULL) != 0)
		return clock->helper->error(clock->helper->panel,
				"gettimeofday", TRUE);
	t = tv.tv_sec;
	localtime_r(&t, &tm);
	strftime(buf, sizeof(buf), clock->format, &tm);
	gtk_label_set_text(GTK_LABEL(clock->label), buf);
	return TRUE;
}
