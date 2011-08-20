/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Phone */
static char const _license[] =
"This program is free software: you can redistribute it and/or modify\n"
"it under the terms of the GNU General Public License as published by\n"
"the Free Software Foundation, version 3 of the License.\n"
"\n"
"This program is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"GNU General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU General Public License\n"
"along with this program.  If not, see <http://www.gnu.org/licenses/>.";



#include <System.h>
#include <Desktop.h>
#ifdef DEBUG
# include <stdio.h>
#endif
#include <gtk/gtk.h>
#include "Phone.h"
#include "../../config.h"


/* Systray */
/* private */
/* types */
typedef struct _Systray
{
	GtkStatusIcon * icon;
	GtkWidget * ab_window;
} Systray;


/* prototypes */
/* plug-in */
static int _systray_init(PhonePlugin * plugin);
static int _systray_destroy(PhonePlugin * plugin);

/* callbacks */
#if GTK_CHECK_VERSION(2, 10, 0)
static void _systray_on_activate(gpointer data);
#endif


/* variables */
static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};

static char _copyright[] =
"Copyright (c) 2011 DeforaOS Project <contact@defora.org>";


/* public */
/* variables */
PhonePlugin plugin =
{
	NULL,
	"System tray",
	"gnome-monitor",
	_systray_init,
	_systray_destroy,
	NULL,
	NULL,
	NULL
};


/* private */
/* functions */
/* systray_init */
static int _systray_init(PhonePlugin * plugin)
{
	Systray * systray;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
#if GTK_CHECK_VERSION(2, 10, 0)
	if((systray = object_new(sizeof(*systray))) == NULL)
		return 1;
	plugin->priv = systray;
	systray->icon = gtk_status_icon_new_from_icon_name("phone-dialer");
	g_signal_connect_swapped(systray->icon, "activate", G_CALLBACK(
				_systray_on_activate), plugin);
	systray->ab_window = NULL;
	return 0;
#else
	return 1;
#endif
}


/* systray_destroy */
static int _systray_destroy(PhonePlugin * plugin)
{
	Systray * systray = plugin->priv;

	g_object_unref(systray->icon);
	return 0;
}


#if GTK_CHECK_VERSION(2, 10, 0)
/* callbacks */
/* systray_on_activate */
static gboolean _activate_on_closex(gpointer data);

static void _systray_on_activate(gpointer data)
{
	PhonePlugin * plugin = data;
	Systray * systray = plugin->priv;

	if(systray->ab_window != NULL)
	{
		gtk_window_present(GTK_WINDOW(systray->ab_window));
		return;
	}
	systray->ab_window = desktop_about_dialog_new();
	desktop_about_dialog_set_authors(systray->ab_window, _authors);
	desktop_about_dialog_set_copyright(systray->ab_window, _copyright);
	desktop_about_dialog_set_license(systray->ab_window, _license);
	desktop_about_dialog_set_logo_icon_name(systray->ab_window,
			"phone-dialer");
	desktop_about_dialog_set_name(systray->ab_window, PACKAGE);
	desktop_about_dialog_set_version(systray->ab_window, VERSION);
	g_signal_connect_swapped(systray->ab_window, "delete-event",
			G_CALLBACK(_activate_on_closex), plugin);
	gtk_widget_show(systray->ab_window);
}

static gboolean _activate_on_closex(gpointer data)
{
	PhonePlugin * plugin = data;
	Systray * systray = plugin->priv;

	gtk_widget_hide(systray->ab_window);
	return TRUE;
}
#endif
