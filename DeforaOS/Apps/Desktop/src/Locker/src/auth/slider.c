/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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
typedef struct _Slider
{
	guint source;
	GtkWidget * scale;
} Slider;


/* prototypes */
/* plug-in */
static GtkWidget * _slider_init(LockerAuth * plugin);
static void _slider_destroy(LockerAuth * plugin);
static void _slider_action(LockerAuth * plugin, LockerAction action);

/* callbacks */
static void _slider_on_scale_value_changed(gpointer data);
static gboolean _slider_on_scale_value_changed_timeout(gpointer data);
static gboolean _slider_on_timeout(gpointer data);
static gboolean _slider_on_timeout_suspend(gpointer data);


/* public */
/* variables */
/* plug-in */
LockerAuth plugin =
{
	NULL,
	"Slider",
	_slider_init,
	_slider_destroy,
	_slider_action,
	NULL
};


/* private */
/* functions */
/* slider_init */
static GtkWidget * _slider_init(LockerAuth * plugin)
{
	Slider * slider;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;

	if((slider = object_new(sizeof(*slider))) == NULL)
		return NULL;
	plugin->priv = slider;
	slider->source = 0;
	vbox = gtk_vbox_new(FALSE, 4);
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
				_slider_on_scale_value_changed), plugin);
	gtk_box_pack_start(GTK_BOX(hbox), slider->scale, FALSE, TRUE, 0);
	/* right image */
	widget = gtk_image_new_from_icon_name("stock_lock-open",
			GTK_ICON_SIZE_LARGE_TOOLBAR);
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_misc_set_padding(GTK_MISC(widget), 0, 96);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	gtk_widget_show_all(vbox);
	return vbox;
}


/* slider_destroy */
static void _slider_destroy(LockerAuth * plugin)
{
	Slider * slider = plugin->priv;

	if(slider->source != 0)
		g_source_remove(slider->source);
	object_delete(slider);
}


/* slider_action */
static void _slider_action(LockerAuth * plugin, LockerAction action)
{
	Slider * slider = plugin->priv;

	switch(action)
	{
		case LOCKER_ACTION_LOCK:
			gtk_range_set_value(GTK_RANGE(slider->scale), 0.0);
			if(slider->source != 0)
				g_source_remove(slider->source);
			slider->source = g_timeout_add(10000,
					_slider_on_timeout_suspend, plugin);
			break;
		default:
			break;
	}
}


/* callbacks */
/* slider_on_scale_value_changed */
static void _slider_on_scale_value_changed(gpointer data)
{
	LockerAuth * plugin = data;
	LockerAuthHelper * helper = plugin->helper;
	Slider * slider = plugin->priv;
	gdouble value;

	if(slider->source != 0)
		g_source_remove(slider->source);
	slider->source = 0;
	value = gtk_range_get_value(GTK_RANGE(slider->scale));
	if(value >= 100.0)
		helper->action(helper->locker, LOCKER_ACTION_UNLOCK);
	else if(value > 0.0)
		slider->source = g_timeout_add(1000,
				_slider_on_scale_value_changed_timeout, plugin);
}


/* slider_on_scale_value_changed_timeout */
static gboolean _slider_on_scale_value_changed_timeout(gpointer data)
{
	LockerAuth * plugin = data;
	Slider * slider = plugin->priv;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	gtk_range_set_value(GTK_RANGE(slider->scale), 0.0);
	slider->source = g_timeout_add(3000, _slider_on_timeout, plugin);
	return FALSE;
}


/* slider_on_timeout */
static gboolean _slider_on_timeout(gpointer data)
{
	LockerAuth * plugin = data;
	LockerAuthHelper * helper = plugin->helper;
	Slider * slider = plugin->priv;

	slider->source = 0;
	helper->action(helper->locker, LOCKER_ACTION_ACTIVATE);
	return FALSE;
}


/* slider_on_timeout_suspend */
static gboolean _slider_on_timeout_suspend(gpointer data)
{
	LockerAuth * plugin = data;
	LockerAuthHelper * helper = plugin->helper;
#ifdef __NetBSD__
	int sleep_state = 3;
#else
	int fd;
	char * suspend[] = { "/usr/bin/sudo", "sudo", "/usr/bin/apm", "-s",
		NULL };
	GError * error = NULL;
#endif

#ifdef __NetBSD__
	if(sysctlbyname("machdep.sleep_state", NULL, NULL, &sleep_state,
				sizeof(sleep_state)) != 0)
		helper->error(helper->locker, strerror(errno), 1);
#else
	if((fd = open("/sys/power/state", O_WRONLY)) >= 0)
	{
		write(fd, "mem\n", 4);
		close(fd);
	}
	else if(g_spawn_async(NULL, suspend, NULL, G_SPAWN_FILE_AND_ARGV_ZERO,
				NULL, NULL, NULL, &error) != TRUE)
	{
		helper->error(helper->locker, error->message, 1);
		g_error_free(error);
	}
#endif
	return FALSE;
}
