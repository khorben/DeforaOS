/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
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
/* TODO:
 * - detect when charging (using apm_read()?) */



#include <System.h>
#include <gtk/gtk.h>
#ifdef DEBUG
# include <stdio.h>
#endif
#if defined(__NetBSD__)
# include <sys/param.h>
# include <sys/sysctl.h>
#elif defined(__linux__)
# include <linux/input.h>
# include <fcntl.h>
# include <unistd.h>
# include <stdint.h>
#else
# include <fcntl.h>
# include <unistd.h>
#endif
#include "Locker.h"


/* Openmoko */
/* private */
/* types */
typedef struct _Openmoko
{
	GtkWidget * window;
#if defined(__linux__)
	GIOChannel * channel;
	guint source;
#endif
} Openmoko;


/* constants */
#if defined(__linux__)
# define AUX_BUTTON_KEYCODE	169
# define POWER_BUTTON_KEYCODE	116
#endif


/* prototypes */
/* plug-in */
static int _openmoko_init(LockerPlugin * plugin);
static void _openmoko_destroy(LockerPlugin * plugin);

/* useful */
static void _openmoko_show_dialog(LockerPlugin * plugin);

/* callbacks */
#if defined(__linux__)
static gboolean _openmoko_on_reset(gpointer data);
static gboolean _openmoko_on_watch_can_read(GIOChannel * source,
		GIOCondition condition, gpointer data);
#endif


/* public */
/* variables */
/* plug-in */
LockerPlugin plugin =
{
	NULL,
	"Openmoko",
	NULL,
	_openmoko_init,
	_openmoko_destroy,
	NULL,
	NULL
};


/* private */
/* functions */
/* openmoko_init */
static int _openmoko_init(LockerPlugin * plugin)
{
	Openmoko * openmoko;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if((openmoko = object_new(sizeof(*openmoko))) == NULL)
		return -1;
	plugin->priv = openmoko;
	openmoko->window = NULL;
#if defined(__linux__)
	openmoko->channel = NULL;
	openmoko->source = 0;
	if(_openmoko_on_reset(plugin) == TRUE)
		openmoko->source = g_timeout_add(1000, _openmoko_on_reset,
				plugin);
#endif
	return 0;
}


/* openmoko_destroy */
static void _openmoko_destroy(LockerPlugin * plugin)
{
	Openmoko * openmoko = plugin->priv;
#if defined(__linux__)
	LockerPluginHelper * helper = plugin->helper;
	GError * error = NULL;
#endif

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(openmoko->window != NULL)
		gtk_widget_destroy(openmoko->window);
#if defined(__linux__)
	if(openmoko->source != 0)
		g_source_remove(openmoko->source);
	if(openmoko->channel != NULL && g_io_channel_shutdown(openmoko->channel,
				TRUE, &error) == G_IO_STATUS_ERROR)
	{
		helper->error(helper->locker, error->message, 1);
		g_error_free(error);
	}
#endif
	object_delete(openmoko);
}


/* useful */
/* openmoko_show_dialog */
static gboolean _dialog_on_closex(gpointer data);
static void _dialog_on_lock(gpointer data);
static void _dialog_on_suspend(gpointer data);
static void _dialog_on_shutdown(gpointer data);
enum { RES_CANCEL, RES_REBOOT, RES_SHUTDOWN };

static void _openmoko_show_dialog(LockerPlugin * plugin)
{
	Openmoko * openmoko = plugin->priv;
	GtkWidget * vbox;
	GtkWidget * widget;
	GtkWidget * image;

	if(openmoko->window != NULL)
	{
		gtk_window_present(GTK_WINDOW(openmoko->window));
		return;
	}
	openmoko->window = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(openmoko->window), "Power menu");
	g_signal_connect_swapped(openmoko->window, "delete-event", G_CALLBACK(
				_dialog_on_closex), plugin);
#if GTK_CHECK_VERSION(2, 14, 0)
	vbox = gtk_dialog_get_content_area(GTK_DIALOG(openmoko->window));
#else
	vbox = GTK_DIALOG(openmoko->window)->vbox;
#endif
	/* lock screen */
	widget = gtk_button_new_with_label("Lock screen");
	image = gtk_image_new_from_icon_name("gnome-lockscreen",
			GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(widget), image);
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(_dialog_on_lock),
			plugin);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	/* suspend */
	widget = gtk_button_new_with_label("Suspend");
	image = gtk_image_new_from_icon_name("gtk-media-pause",
			GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(widget), image);
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(
				_dialog_on_suspend), plugin);
	/* shutdown */
	widget = gtk_button_new_with_label("Shutdown");
	image = gtk_image_new_from_icon_name("gnome-shutdown",
			GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(widget), image);
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(
				_dialog_on_shutdown), plugin);
	/* FIXME implement */
	gtk_widget_show(openmoko->window);
}

static gboolean _dialog_on_closex(gpointer data)
{
	LockerPlugin * plugin = data;
	Openmoko * openmoko = plugin->priv;

	gtk_widget_hide(openmoko->window);
	return TRUE;
}

static void _dialog_on_lock(gpointer data)
{
	LockerPlugin * plugin = data;
	LockerPluginHelper * helper = plugin->helper;

	helper->action(helper->locker, LOCKER_ACTION_LOCK);
}

/* FIXME code duplicated from Panel */
static void _dialog_on_shutdown(gpointer data)
{
	LockerPlugin * plugin = data;
	LockerPluginHelper * helper = plugin->helper;
	GtkWidget * window;
	GtkWidget * widget;
#ifdef EMBEDDED
	const char * message = "This will shutdown your device,"
			" therefore closing any application currently opened"
			" and losing any unsaved data.\n"
			"Do you really want to proceed?";
#else
	const char * message = "This will shutdown your computer,"
			" therefore closing any application currently opened"
			" and losing any unsaved data.\n"
			"Do you really want to proceed?";
#endif
	int res;
	char * reboot[] = { "/sbin/shutdown", "shutdown", "-r", "now", NULL };
	char * shutdown[] = { "/sbin/shutdown", "shutdown",
#if defined(__NetBSD__)
		"-p",
#else
		"-h",
#endif
		"now", NULL };
	char ** argv;
	GError * error = NULL;

	window = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_NONE, "%s",
#if GTK_CHECK_VERSION(2, 6, 0)
			"Shutdown");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(window),
#endif
			"%s", message);
	gtk_dialog_add_buttons(GTK_DIALOG(window), GTK_STOCK_CANCEL,
			RES_CANCEL, "Restart", RES_REBOOT, NULL);
	widget = gtk_button_new_with_label("Shutdown");
	gtk_button_set_image(GTK_BUTTON(widget), gtk_image_new_from_icon_name(
				"gnome-shutdown", GTK_ICON_SIZE_BUTTON));
	gtk_widget_show_all(widget);
	gtk_dialog_add_action_widget(GTK_DIALOG(window), widget, RES_SHUTDOWN);
	gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER_ALWAYS);
	gtk_window_set_title(GTK_WINDOW(window), "Shutdown");
	res = gtk_dialog_run(GTK_DIALOG(window));
	gtk_widget_destroy(window);
	if(res == RES_SHUTDOWN)
		argv = shutdown;
	else if(res == RES_REBOOT)
		argv = reboot;
	else
		return;
	if(g_spawn_async(NULL, argv, NULL, G_SPAWN_FILE_AND_ARGV_ZERO, NULL,
				NULL, NULL, &error) != TRUE)
	{
		helper->error(helper->locker, error->message, 1);
		g_error_free(error);
	}
}

/* FIXME code duplicated from Panel */
static void _dialog_on_suspend(gpointer data)
{
	LockerPlugin * plugin = data;
	LockerPluginHelper * helper = plugin->helper;
#if defined(__NetBSD__)
	int sleep_state = 3;
#else
	int fd;
	char * suspend[] = { "/usr/bin/sudo", "sudo", "/usr/bin/apm", "-s",
		NULL };
	GError * error = NULL;
#endif

#if defined(__NetBSD__)
	if(sysctlbyname("machdep.sleep_state", NULL, NULL, &sleep_state,
				sizeof(sleep_state)) != 0)
	{
		helper->error(helper->locker, "sysctl", 1);
		return;
	}
#else
	if((fd = open("/sys/power/state", O_WRONLY)) >= 0)
	{
		write(fd, "mem\n", 4);
		close(fd);
		return;
	}
	if(g_spawn_async(NULL, suspend, NULL, G_SPAWN_FILE_AND_ARGV_ZERO, NULL,
				NULL, NULL, &error) != TRUE)
	{
		helper->error(helper->locker, error->message, 1);
		g_error_free(error);
		return;
	}
#endif
	/* XXX may already be suspended */
	helper->action(helper->locker, LOCKER_ACTION_LOCK);
}


/* callbacks */
#if defined(__linux__)
/* openmoko_on_reset */
static gboolean _openmoko_on_reset(gpointer data)
{
	LockerPlugin * plugin = data;
	Openmoko * openmoko = plugin->priv;
	int fd;
	GError * error = NULL;

	/* FIXME open all of the relevant input event nodes */
	if((fd = open("/dev/input/event0", O_RDONLY)) < 0)
		return TRUE;
	openmoko->channel = g_io_channel_unix_new(fd);
	if(g_io_channel_set_encoding(openmoko->channel, NULL, &error)
			!= G_IO_STATUS_NORMAL)
	{
		plugin->helper->error(plugin->helper->locker,
				error->message, 1);
		g_error_free(error);
	}
	g_io_channel_set_buffered(openmoko->channel, FALSE);
	openmoko->source = g_io_add_watch(openmoko->channel, G_IO_IN,
			_openmoko_on_watch_can_read, plugin);
	return FALSE;
}


/* openmoko_on_watch_can_read */
static gboolean _watch_can_read_event(LockerPlugin * plugin,
		struct input_event * event);
static void _watch_can_read_event_key(LockerPlugin * plugin, uint16_t code,
		int32_t value);
static gboolean _watch_can_read_reset(LockerPlugin * plugin);

static gboolean _openmoko_on_watch_can_read(GIOChannel * source,
		GIOCondition condition, gpointer data)
{
	LockerPlugin * plugin = data;
	LockerPluginHelper * helper = plugin->helper;
	Openmoko * openmoko = plugin->priv;
	struct input_event event;
	gsize cnt = 0;
	GError * error = NULL;
	GIOStatus status;

	if(condition != G_IO_IN || source != openmoko->channel)
		return FALSE; /* should not happen */
	status = g_io_channel_read_chars(source, (gchar *)&event, sizeof(event),
			&cnt, &error);
	switch(status)
	{
		case G_IO_STATUS_NORMAL:
			break;
		case G_IO_STATUS_ERROR:
			helper->error(helper->locker, error->message, 1);
			g_error_free(error);
		case G_IO_STATUS_EOF:
		default:
			return _watch_can_read_reset(plugin);
	}
	/* FIXME avoid this by using a regular read() */
	if(cnt != sizeof(event))
		return _watch_can_read_reset(plugin);
	return _watch_can_read_event(plugin, &event);
}

static gboolean _watch_can_read_event(LockerPlugin * plugin,
		struct input_event * event)
{
	switch(event->type)
	{
		case EV_KEY:
			_watch_can_read_event_key(plugin, event->code,
					event->value);
			break;
#ifdef DEBUG
		default:
			fprintf(stderr, "DEBUG: %s() Unknown event type %u\n",
					__func__, event->type);
			break;
#endif
	}
	return TRUE;
}

static void _watch_can_read_event_key(LockerPlugin * plugin, uint16_t code,
		int32_t value)
{
	LockerPluginHelper * helper = plugin->helper;

	switch(code)
	{
		case AUX_BUTTON_KEYCODE:
			if(value == 0) /* released */
				helper->action(helper->locker,
						LOCKER_ACTION_LOCK);
			break;
		case POWER_BUTTON_KEYCODE:
			if(value == 0) /* released */
				_openmoko_show_dialog(plugin);
			break;
	}
}

static gboolean _watch_can_read_reset(LockerPlugin * plugin)
{
	LockerPluginHelper * helper = plugin->helper;
	Openmoko * openmoko = plugin->priv;
	GError * error = NULL;

	if(g_io_channel_shutdown(openmoko->channel, TRUE, &error)
			== G_IO_STATUS_ERROR)
	{
		helper->error(helper->locker, error->message, 1);
		g_error_free(error);
	}
	openmoko->source = g_timeout_add(1000, _openmoko_on_reset, plugin);
	return FALSE;
}
#endif
