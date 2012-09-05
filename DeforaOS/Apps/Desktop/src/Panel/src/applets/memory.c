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
#include <errno.h>
#if defined(__linux__)
# include <sys/sysinfo.h>
#elif defined(__FreeBSD__) || defined(__NetBSD__)
# include <sys/sysctl.h>
# if defined(__FreeBSD__)
#  include <sys/vmmeter.h>
#  include <vm/vm_param.h>
# endif
#endif
#include <libintl.h>
#include "Panel.h"
#define _(string) gettext(string)


/* Memory */
/* private */
/* types */
typedef struct _PanelApplet
{
	PanelAppletHelper * helper;
	GtkWidget * scale;
	guint timeout;
} Memory;


/* prototypes */
static Memory * _memory_init(PanelAppletHelper * helper, GtkWidget ** widget);
static void _memory_destroy(Memory * memory);

/* callbacks */
#if defined(__FreeBSD__) || defined(__linux__) || defined(__NetBSD__)
static gboolean _on_timeout(gpointer data);
#endif


/* public */
/* variables */
PanelAppletDefinition applet =
{
	"Memory",
	"gnome-monitor",
	NULL,
	_memory_init,
	_memory_destroy,
	NULL,
	FALSE,
	TRUE
};


/* private */
/* functions */
/* memory_init */
static Memory * _memory_init(PanelAppletHelper * helper, GtkWidget ** widget)
{
#if defined(__FreeBSD__) || defined(__linux__) || defined(__NetBSD__)
	Memory * memory;
	GtkWidget * ret;
	PangoFontDescription * desc;
	GtkWidget * label;

	if((memory = malloc(sizeof(*memory))) == NULL)
	{
		helper->error(NULL, "malloc", 1);
		return NULL;
	}
	memory->helper = helper;
	ret = gtk_hbox_new(FALSE, 0);
	desc = pango_font_description_new();
	pango_font_description_set_weight(desc, PANGO_WEIGHT_BOLD);
	label = gtk_label_new(_("RAM:"));
	gtk_widget_modify_font(label, desc);
	gtk_box_pack_start(GTK_BOX(ret), label, FALSE, FALSE, 0);
	memory->scale = gtk_vscale_new_with_range(0, 100, 1);
	gtk_widget_set_sensitive(memory->scale, FALSE);
	gtk_range_set_inverted(GTK_RANGE(memory->scale), TRUE);
	gtk_scale_set_value_pos(GTK_SCALE(memory->scale), GTK_POS_RIGHT);
	gtk_box_pack_start(GTK_BOX(ret), memory->scale, FALSE, FALSE, 0);
	memory->timeout = g_timeout_add(5000, _on_timeout, memory);
	_on_timeout(memory);
	pango_font_description_free(desc);
	gtk_widget_show_all(ret);
	*widget = ret;
	return memory;
#else
	helper->error(NULL, _("memory: Unsupported platform"), 1);
	return NULL;
#endif
}


/* memory_destroy */
static void _memory_destroy(Memory * memory)
{
	g_source_remove(memory->timeout);
	free(memory);
}


/* callbacks */
/* on_timeout */
#if defined(__linux__)
static gboolean _on_timeout(gpointer data)
{
	Memory * memory = data;
	struct sysinfo sy;
	gdouble value;

	if(sysinfo(&sy) != 0)
		return memory->helper->error(memory->helper->panel, "sysinfo",
				TRUE);
	value = sy.sharedram;
	value /= sy.totalram;
	gtk_range_set_value(GTK_RANGE(memory->scale), value);
	return TRUE;
}
#elif defined(__FreeBSD__) || defined(__NetBSD__)
static gboolean _on_timeout(gpointer data)
{
	Memory * memory = data;
	int mib[] = { CTL_VM, VM_METER };
	struct vmtotal vm;
	size_t size = sizeof(vm);
	gdouble value;

	if(sysctl(mib, 2, &vm, &size, NULL, 0) < 0)
		return TRUE;
	value = vm.t_arm * 100;
	value /= (vm.t_rm + vm.t_free);
	gtk_range_set_value(GTK_RANGE(memory->scale), value);
	return TRUE;
}
#endif
