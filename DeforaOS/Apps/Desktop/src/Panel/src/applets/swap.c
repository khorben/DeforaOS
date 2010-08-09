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
#include <errno.h>
#if defined(__linux__)
# include <sys/sysinfo.h>
#elif defined(__NetBSD__)
# include <sys/sysctl.h>
#endif
#include <libintl.h>
#include "Panel.h"
#define _(string) gettext(string)


/* Swap */
/* private */
/* types */
typedef struct _Swap
{
	PanelAppletHelper * helper;
	GtkWidget * scale;
	guint timeout;
} Swap;


/* prototypes */
static GtkWidget * _swap_init(PanelApplet * applet);
static void _swap_destroy(PanelApplet * applet);

/* callbacks */
#if defined(__linux__) || defined(__NetBSD__)
static gboolean _on_timeout(gpointer data);
#endif


/* public */
/* variables */
PanelApplet applet =
{
	NULL,
	_swap_init,
	_swap_destroy,
	PANEL_APPLET_POSITION_END,
	FALSE,
	TRUE,
	NULL
};


/* private */
/* functions */
/* swap_init */
static GtkWidget * _swap_init(PanelApplet * applet)
{
#if defined(__linux__) || defined(__NetBSD__)
	GtkWidget * ret;
	Swap * swap;
	PangoFontDescription * desc;
	GtkWidget * widget;

	if((swap = malloc(sizeof(*swap))) == NULL)
	{
		applet->helper->error(applet->helper->panel, "malloc", 0);
		return NULL;
	}
	applet->priv = swap;
	swap->helper = applet->helper;
	ret = gtk_hbox_new(FALSE, 0);
	desc = pango_font_description_new();
	pango_font_description_set_weight(desc, PANGO_WEIGHT_BOLD);
	widget = gtk_label_new(_("Swap:"));
	gtk_widget_modify_font(widget, desc);
	gtk_box_pack_start(GTK_BOX(ret), widget, FALSE, FALSE, 0);
	swap->scale = gtk_vscale_new_with_range(0, 100, 1);
	gtk_widget_set_sensitive(swap->scale, FALSE);
	gtk_range_set_inverted(GTK_RANGE(swap->scale), TRUE);
	gtk_scale_set_value_pos(GTK_SCALE(swap->scale), GTK_POS_RIGHT);
	gtk_box_pack_start(GTK_BOX(ret), swap->scale, FALSE, FALSE, 0);
	swap->timeout = g_timeout_add(5000, _on_timeout, swap);
	_on_timeout(swap);
	pango_font_description_free(desc);
	gtk_widget_show_all(ret);
	return ret;
#else
	error_set("%s: %s", "swap", _("Unsupported platform"));
	return NULL;
#endif
}


/* swap_destroy */
static void _swap_destroy(PanelApplet * applet)
{
	Swap * swap = applet->priv;

	g_source_remove(swap->timeout);
	free(swap);
}


/* callbacks */
/* on_timeout */
#if defined(__linux__)
static gboolean _on_timeout(gpointer data)
{
	Swap * swap = data;
	struct sysinfo sy;
	gdouble value;

	if(sysinfo(&sy) != 0)
		return swap->helper->error(swap->helper->panel, "sysinfo",
				TRUE);
	if((value = sy.totalswap - sy.freeswap) != 0.0 && sy.totalswap != 0)
		value /= sy.totalswap;
	gtk_range_set_value(GTK_RANGE(swap->scale), value);
	return TRUE;
}
#elif defined(__NetBSD__)
static gboolean _on_timeout(gpointer data)
{
	Swap * swap = data;
	int mib[] = { CTL_VM, VM_UVMEXP };
	struct uvmexp ue;
	size_t size = sizeof(ue);
	gdouble value;

	if(sysctl(mib, 2, &ue, &size, NULL, 0) < 0)
		return TRUE;
	if((value = ue.swpgonly) != 0.0 && ue.swpages != 0)
		value /= ue.swpages;
	gtk_range_set_value(GTK_RANGE(swap->scale), value);
	return TRUE;
}
#endif
