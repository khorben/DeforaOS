/* $Id$ */
/* Copyright (c) 2010-2012 Pierre Pronchery <khorben@defora.org> */
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
#if defined(__FreeBSD__) || defined(__NetBSD__)
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
typedef struct _PanelApplet
{
	PanelAppletHelper * helper;
	GtkWidget * hbox;
	GtkWidget * label;
	guint timeout;
	int min;
	int max;
	int step;
#if defined(__FreeBSD__) || defined(__NetBSD__)
	char const * name;
#endif
} Cpufreq;


/* prototypes */
static Cpufreq * _cpufreq_init(PanelAppletHelper * helper, GtkWidget ** widget);
static void _cpufreq_destroy(Cpufreq * cpufreq);

/* callbacks */
#if defined(__FreeBSD__) || defined(__NetBSD__)
static gboolean _on_timeout(gpointer data);
#endif


/* public */
/* variables */
PanelAppletDefinition applet =
{
	"CPU frequency",
	"gnome-monitor",
	NULL,
	_cpufreq_init,
	_cpufreq_destroy,
	NULL,
	FALSE,
	TRUE
};


/* private */
/* functions */
/* cpufreq_init */
static Cpufreq * _cpufreq_init(PanelAppletHelper * helper, GtkWidget ** widget)
{
#if defined(__FreeBSD__) || defined(__NetBSD__)
	Cpufreq * cpufreq;
	PangoFontDescription * desc;
	GtkWidget * image;
	GtkWidget * label;
	char freq[256];
	size_t freqsize = sizeof(freq);
	char const * p;

	/* detect the correct sysctl */
	if(sysctlbyname("hw.clockrate", &freq, &freqsize, NULL, 0) == 0)
		p = "hw.clockrate";
	else if(sysctlbyname("machdep.est.frequency.available", &freq,
				&freqsize, NULL, 0) == 0)
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
	cpufreq->helper = helper;
	desc = pango_font_description_new();
	pango_font_description_set_weight(desc, PANGO_WEIGHT_BOLD);
	cpufreq->hbox = gtk_hbox_new(FALSE, 4);
	image = gtk_image_new_from_icon_name("gnome-monitor",
			helper->icon_size);
	gtk_box_pack_start(GTK_BOX(cpufreq->hbox), image, FALSE, TRUE, 0);
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
	label = gtk_label_new(_("MHz"));
	gtk_box_pack_start(GTK_BOX(cpufreq->hbox), label, FALSE, TRUE, 0);
	if(_on_timeout(cpufreq) == TRUE)
		cpufreq->timeout = g_timeout_add(1000, _on_timeout, cpufreq);
	pango_font_description_free(desc);
	gtk_widget_show_all(cpufreq->hbox);
	*widget = cpufreq->hbox;
	return cpufreq;
#else
	error_set("%s: %s", "cpufreq", _("Unsupported platform"));
	return NULL;
#endif
}


/* cpufreq_destroy */
static void _cpufreq_destroy(Cpufreq * cpufreq)
{
	g_source_remove(cpufreq->timeout);
	free(cpufreq);
}


/* callbacks */
#if defined(__FreeBSD__) || defined(__NetBSD__)
/* on_timeout */
static gboolean _on_timeout(gpointer data)
{
	Cpufreq * cpufreq = data;
	PanelAppletHelper * helper = cpufreq->helper;
	uint64_t freq;
	size_t freqsize = sizeof(freq);
	char buf[256];

	if(sysctlbyname(cpufreq->name, &freq, &freqsize, NULL, 0) < 0)
		return helper->error(NULL, cpufreq->name, TRUE);
	snprintf(buf, sizeof(buf), "%u", (unsigned int)freq);
	gtk_label_set_text(GTK_LABEL(cpufreq->label), buf);
# if GTK_CHECK_VERSION(2, 12, 0)
	snprintf(buf, sizeof(buf), "%s%u %s", _("CPU frequency: "),
			(unsigned int)freq, _("MHz"));
	gtk_widget_set_tooltip_text(cpufreq->hbox, buf);
# endif
	return TRUE;
}
#endif
