/* $Id$ */
/* Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Locker */
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



#include <System.h>
#ifdef __NetBSD__
# include <sys/param.h>
# include <sys/sysctl.h>
# include <errno.h>
#else
# include <fcntl.h>
# include <unistd.h>
#endif
#include <string.h>
#include "Locker.h"


/* Slider */
/* private */
/* types */
typedef struct _LockerAuth
{
	LockerAuthHelper * helper;
	guint source;

	/* widgets */
	GtkWidget * widget;
	GtkWidget * scale;
} Slider;


/* prototypes */
/* plug-in */
static Slider * _slider_init(LockerAuthHelper * helper);
static void _slider_destroy(Slider * slider);
static GtkWidget * _slider_get_widget(Slider * slider);
static int _slider_action(Slider * slider, LockerAction action);

/* callbacks */
static void _slider_on_scale_value_changed(gpointer data);
static gboolean _slider_on_scale_value_changed_timeout(gpointer data);
static gboolean _slider_on_timeout(gpointer data);


/* public */
/* variables */
/* plug-in */
LockerAuthDefinition plugin =
{
	"Slider",
	NULL,
	NULL,
	_slider_init,
	_slider_destroy,
	_slider_get_widget,
	_slider_action
};


/* private */
/* functions */
/* slider_init */
static Slider * _slider_init(LockerAuthHelper * helper)
{
	Slider * slider;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;

	if((slider = object_new(sizeof(*slider))) == NULL)
		return NULL;
	slider->helper = helper;
	slider->source = 0;
	vbox = gtk_vbox_new(FALSE, 4);
	slider->widget = vbox;
	hbox = gtk_hbox_new(FALSE, 4);
	/* left image */
	widget = gtk_image_new_from_icon_name("stock_lock",
			GTK_ICON_SIZE_LARGE_TOOLBAR);
	gtk_misc_set_alignment(GTK_MISC(widget), 1.0, 0.5);
	gtk_misc_set_padding(GTK_MISC(widget), 0, 96);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	/* scale */
	slider->scale = gtk_hscale_new_with_range(0.0, 100.0, 1.0);
	gtk_range_set_value(GTK_RANGE(slider->scale), 0.0);
	gtk_scale_set_draw_value(GTK_SCALE(slider->scale), FALSE);
	gtk_widget_set_size_request(slider->scale, 240, -1);
	g_signal_connect_swapped(slider->scale, "value-changed", G_CALLBACK(
				_slider_on_scale_value_changed), slider);
	gtk_box_pack_start(GTK_BOX(hbox), slider->scale, FALSE, TRUE, 0);
	/* right image */
	widget = gtk_image_new_from_icon_name("stock_lock-open",
			GTK_ICON_SIZE_LARGE_TOOLBAR);
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_misc_set_padding(GTK_MISC(widget), 0, 96);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	gtk_widget_show_all(vbox);
	return slider;
}


/* slider_destroy */
static void _slider_destroy(Slider * slider)
{
	if(slider->source != 0)
		g_source_remove(slider->source);
	object_delete(slider);
}


/* slider_get_widget */
static GtkWidget * _slider_get_widget(Slider * slider)
{
	return slider->widget;
}


/* slider_action */
static int _slider_action(Slider * slider, LockerAction action)
{
	switch(action)
	{
		case LOCKER_ACTION_DEACTIVATE:
			gtk_widget_grab_focus(slider->scale);
			gtk_widget_show(slider->widget);
			break;
		case LOCKER_ACTION_LOCK:
			gtk_widget_hide(slider->widget);
			gtk_range_set_value(GTK_RANGE(slider->scale), 0.0);
			if(slider->source != 0)
				g_source_remove(slider->source);
			slider->source = 0;
			break;
		case LOCKER_ACTION_START:
		case LOCKER_ACTION_UNLOCK:
			gtk_widget_hide(slider->widget);
			break;
		default:
			break;
	}
	return 0;
}


/* callbacks */
/* slider_on_scale_value_changed */
static void _slider_on_scale_value_changed(gpointer data)
{
	Slider * slider = data;
	LockerAuthHelper * helper = slider->helper;
	gdouble value;

	if(slider->source != 0)
		g_source_remove(slider->source);
	slider->source = 0;
	value = gtk_range_get_value(GTK_RANGE(slider->scale));
	if(value >= 100.0)
		helper->action(helper->locker, LOCKER_ACTION_UNLOCK);
	else if(value > 0.0)
		slider->source = g_timeout_add(1000,
				_slider_on_scale_value_changed_timeout, slider);
}


/* slider_on_scale_value_changed_timeout */
static gboolean _slider_on_scale_value_changed_timeout(gpointer data)
{
	Slider * slider = data;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	gtk_range_set_value(GTK_RANGE(slider->scale), 0.0);
	slider->source = g_timeout_add(3000, _slider_on_timeout, slider);
	return FALSE;
}


/* slider_on_timeout */
static gboolean _slider_on_timeout(gpointer data)
{
	Slider * slider = data;
	LockerAuthHelper * helper = slider->helper;

	slider->source = 0;
	helper->action(helper->locker, LOCKER_ACTION_ACTIVATE);
	return FALSE;
}
