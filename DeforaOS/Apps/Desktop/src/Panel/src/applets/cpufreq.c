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



#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#ifdef __NetBSD__
# include <sys/param.h>
# include <sys/sched.h>
# include <sys/sysctl.h>
#endif
#include <libintl.h>
#include <System.h>
#include "Panel.h"
#define _(string) gettext(string)


/* Cpufreq */
/* private */
/* types */
typedef struct _Cpufreq
{
	GtkWidget * hbox;
	GtkWidget * label;
	guint timeout;
	int min;
	int max;
	int step;
#ifdef __NetBSD__
	char const * name;
#endif
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
	"CPU frequency",
	"gnome-monitor",
	_cpufreq_init,
	_cpufreq_destroy,
	NULL,
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
	PanelAppletHelper * helper = applet->helper;
	Cpufreq * cpufreq;
	PangoFontDescription * desc;
	GtkWidget * widget;
	char freq[256];
	size_t freqsize = sizeof(freq);
	char const * p;

	/* detect est or powernow */
	if(sysctlbyname("machdep.est.frequency.available", &freq, &freqsize,
				NULL, 0) == 0)
		p = "machdep.est.frequency.current";
	else if(sysctlbyname("machdep.powernow.frequency.available", &freq,
				&freqsize, NULL, 0) == 0)
		p = "machdep.powernow.frequency.current";
	else
	{
		error_set("%s: %s", "cpufreq", _("No support detected"));
		return NULL;
	}
	if((cpufreq = malloc(sizeof(*cpufreq))) == NULL)
	{
		helper->error(helper->panel, "malloc", 0);
		return NULL;
	}
	applet->priv = cpufreq;
	desc = pango_font_description_new();
	pango_font_description_set_weight(desc, PANGO_WEIGHT_BOLD);
	cpufreq->hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_image_new_from_icon_name("gnome-monitor",
			helper->icon_size);
	gtk_box_pack_start(GTK_BOX(cpufreq->hbox), widget, FALSE, TRUE, 0);
	cpufreq->min = 0;
	cpufreq->max = 0;
	cpufreq->step = 1;
	cpufreq->name = p;
	cpufreq->max = atoi(freq);
	cpufreq->min = (p = strrchr(freq, ' ')) != NULL ? atoi(p)
		: cpufreq->max;
	cpufreq->label = gtk_label_new(" ");
	gtk_widget_modify_font(cpufreq->label, desc);
	gtk_box_pack_start(GTK_BOX(cpufreq->hbox), cpufreq->label, FALSE, TRUE,
			0);
	widget = gtk_label_new(_("MHz"));
	gtk_box_pack_start(GTK_BOX(cpufreq->hbox), widget, FALSE, TRUE, 0);
	if(_on_timeout(applet) == TRUE)
		cpufreq->timeout = g_timeout_add(1000, _on_timeout, applet);
	pango_font_description_free(desc);
	gtk_widget_show_all(cpufreq->hbox);
	return cpufreq->hbox;
#else
	error_set("%s: %s", "cpufreq", _("Unsupported platform"));
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
	PanelApplet * applet = data;
	PanelAppletHelper * helper = applet->helper;
	Cpufreq * cpufreq = applet->priv;
	uint64_t freq;
	size_t freqsize = sizeof(freq);
	char buf[256];

	if(sysctlbyname(cpufreq->name, &freq, &freqsize, NULL, 0) < 0)
		return helper->error(NULL, cpufreq->name, TRUE);
	snprintf(buf, sizeof(buf), "%u", (unsigned int)freq);
	gtk_label_set_text(GTK_LABEL(cpufreq->label), buf);
# if GTK_CHECK_VERSION(2, 12, 0)
	snprintf(buf, sizeof(buf), "%s%u%s", "CPU frequency: ",
			(unsigned int)freq, " MHz");
	gtk_widget_set_tooltip_text(cpufreq->hbox, buf);
# endif
	return TRUE;
}
#endif
