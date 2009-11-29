/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
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



#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <System.h>
#ifdef __NetBSD__
# include <sys/param.h>
# include <sys/sched.h>
# include <sys/sysctl.h>
#endif
#include "Panel.h"


/* Cpufreq */
/* private */
/* types */
typedef struct _Cpufreq
{
	PanelAppletHelper * helper;
	GtkWidget * scale;
	guint timeout;
	int min;
	int max;
	int step;
} Cpufreq;


/* prototypes */
static GtkWidget * _cpufreq_init(PanelApplet * applet);
static void _cpufreq_destroy(PanelApplet * applet);

/* callbacks */
#ifdef __NetBSD__
static gboolean _on_timeout(gpointer data);
#endif


/* public */
/* variables */
PanelApplet applet =
{
	NULL,
	_cpufreq_init,
	_cpufreq_destroy,
	PANEL_APPLET_POSITION_END,
	FALSE,
	TRUE,
	NULL
};


/* private */
/* functions */
/* cpufreq_init */
static GtkWidget * _cpufreq_init(PanelApplet * applet)
{
#ifdef __NetBSD__
	GtkWidget * ret;
	Cpufreq * cpufreq;
	PangoFontDescription * desc;
	GtkWidget * widget;
	char freq[256];
	size_t freqsize = sizeof(freq);
	char const * p;

	if((cpufreq = malloc(sizeof(*cpufreq))) == NULL)
	{
		applet->helper->error(applet->helper->priv, "malloc", 0);
		return NULL;
	}
	applet->priv = cpufreq;
	cpufreq->helper = applet->helper;
	ret = gtk_hbox_new(FALSE, 0);
	desc = pango_font_description_new();
	pango_font_description_set_weight(desc, PANGO_WEIGHT_BOLD);
	widget = gtk_label_new("Frequency:");
	gtk_widget_modify_font(widget, desc);
	gtk_box_pack_start(GTK_BOX(ret), widget, FALSE, FALSE, 0);
	cpufreq->min = 0;
	cpufreq->max = 0;
	cpufreq->step = 1;
	if(sysctlbyname("machdep.est.frequency.available", &freq, &freqsize,
				NULL, 0) >= 0
			|| sysctlbyname("machdep.powernow.frequency.available",
				&freq, &freqsize, NULL, 0) >= 0)
	{
		cpufreq->max = atoi(freq);
		cpufreq->min = (p = strrchr(freq, ' ')) != NULL ? atoi(p)
			: cpufreq->max;
	}
	cpufreq->scale = gtk_vscale_new_with_range(cpufreq->min, cpufreq->max,
			cpufreq->step);
	gtk_widget_set_sensitive(cpufreq->scale, FALSE);
	gtk_range_set_inverted(GTK_RANGE(cpufreq->scale), TRUE);
	gtk_scale_set_value_pos(GTK_SCALE(cpufreq->scale), GTK_POS_RIGHT);
	gtk_box_pack_start(GTK_BOX(ret), cpufreq->scale, FALSE, FALSE, 0);
	cpufreq->timeout = g_timeout_add(500, _on_timeout, cpufreq);
	_on_timeout(cpufreq);
	pango_font_description_free(desc);
	return ret;
#else
	error_set("%s", "cpufreq: Unsupported platform");
	return NULL;
#endif
}


/* cpufreq_destroy */
static void _cpufreq_destroy(PanelApplet * applet)
{
	Cpufreq * cpufreq = applet->priv;

	g_source_remove(cpufreq->timeout);
	free(cpufreq);
}


/* callbacks */
#ifdef __NetBSD__
/* on_timeout */
static gboolean _on_timeout(gpointer data)
{
	Cpufreq * cpufreq = data;
	uint64_t freq;
	size_t freqsize = sizeof(freq);
	unsigned int f;

	if(sysctlbyname("machdep.est.frequency.current", &freq, &freqsize, NULL,
				0) >= 0)
		f = freq;
	if(sysctlbyname("machdep.est.frequency.current", &freq, &freqsize, NULL,
				0) >= 0)
		f = freq;
	else
		return cpufreq->helper->error(cpufreq->helper->priv, "sysctl",
				TRUE);
	gtk_range_set_value(GTK_RANGE(cpufreq->scale), (double)f);
	return TRUE;
}
#endif
