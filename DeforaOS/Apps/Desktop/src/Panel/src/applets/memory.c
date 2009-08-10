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
#include <errno.h>
#ifdef __NetBSD__
# include <sys/sysctl.h>
#endif
#include "panel.h"
#include "../../config.h"


/* Memory */
/* private */
/* types */
typedef struct _Memory
{
	GtkWidget * scale;
	guint timeout;
} Memory;


/* prototypes */
static GtkWidget * _memory_init(PanelApplet * applet);
static void _memory_destroy(PanelApplet * applet);

/* callbacks */
static gboolean _on_timeout(gpointer data);


/* public */
/* variables */
PanelApplet applet =
{
	NULL,
	_memory_init,
	_memory_destroy,
	PANEL_APPLET_POSITION_END,
	FALSE,
	TRUE,
	NULL
};


/* private */
/* functions */
/* memory_init */
static GtkWidget * _memory_init(PanelApplet * applet)
{
	GtkWidget * ret;
	Memory * memory;
	PangoFontDescription * desc;
	GtkWidget * widget;

	if((memory = malloc(sizeof(*memory))) == NULL)
		return NULL;
	applet->priv = memory;
	ret = gtk_hbox_new(FALSE, 0);
	desc = pango_font_description_new();
	pango_font_description_set_weight(desc, PANGO_WEIGHT_BOLD);
	widget = gtk_label_new("RAM:");
	gtk_widget_modify_font(widget, desc);
	gtk_box_pack_start(GTK_BOX(ret), widget, FALSE, FALSE, 0);
	memory->scale = gtk_vscale_new_with_range(0, 100, 1);
	gtk_widget_set_sensitive(memory->scale, FALSE);
	gtk_range_set_inverted(GTK_RANGE(memory->scale), TRUE);
	gtk_scale_set_value_pos(GTK_SCALE(memory->scale), GTK_POS_RIGHT);
	gtk_box_pack_start(GTK_BOX(ret), memory->scale, FALSE, FALSE, 0);
	memory->timeout = g_timeout_add(500, _on_timeout, memory);
	_on_timeout(memory);
	pango_font_description_free(desc);
	return ret;
}


/* memory_destroy */
static void _memory_destroy(PanelApplet * applet)
{
	Memory * memory = applet->priv;

	g_source_remove(memory->timeout);
	free(memory);
}


/* callbacks */
/* on_timeout */
static gboolean _on_timeout(gpointer data)
{
#if 0 /* def __NetBSD__ */
	Memory * memory = data;
	int mib[] = { CTL_KERN, KERN_CP_TIME };
	uint64_t memory_time[CPUSTATES];
	size_t size = sizeof(memory_time);
	int used;
	int total;
	gdouble value;

	if(sysctl(mib, 2, &memory_time, &size, NULL, 0) < 0)
		return TRUE;
	used = memory_time[CP_USER] + memory_time[CP_SYS] + memory_time[CP_NICE]
		+ memory_time[CP_INTR];
	total = used + memory_time[CP_IDLE];
	if(memory->used == 0)
		value = 0;
	else
		value = 100 * (used - memory->used) / (total - memory->total);
	memory->used = used;
	memory->total = total;
	gtk_range_set_value(GTK_RANGE(memory->scale), value);
#endif
	return TRUE;
}
