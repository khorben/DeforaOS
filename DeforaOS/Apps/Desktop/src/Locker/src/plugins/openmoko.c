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



#if defined(__linux__)
# include <linux/input.h>
# include <sys/ioctl.h>
# include <fcntl.h>
# include <unistd.h>
# include <stdint.h>
# include <string.h>
# include <errno.h>
#endif
#include <stdio.h>
#include <System.h>
#include <gtk/gtk.h>
#include "Locker.h"


/* Openmoko */
/* private */
/* types */
typedef struct _LockerPlugin
{
	LockerPluginHelper * helper;
	GtkWidget * window;
#if defined(__linux__)
	GIOChannel * channel;
	guint source;
#endif
} Openmoko;


/* prototypes */
/* plug-in */
static Openmoko * _openmoko_init(LockerPluginHelper * helper);
static void _openmoko_destroy(Openmoko * openmoko);
static int _openmoko_event(Openmoko * openmoko, LockerEvent event);

/* useful */
#if defined(__linux__)
static void _openmoko_show_dialog(Openmoko * openmoko);
#endif

/* callbacks */
#if defined(__linux__)
static gboolean _openmoko_on_reset(gpointer data);
static gboolean _openmoko_on_watch_can_read(GIOChannel * source,
		GIOCondition condition, gpointer data);
#endif


/* public */
/* variables */
/* plug-in */
LockerPluginDefinition plugin =
{
	"Openmoko",
	"phone-openmoko", /* XXX provide the icon ourselves */
	NULL,
	_openmoko_init,
	_openmoko_destroy,
	_openmoko_event
};


/* private */
/* functions */
/* openmoko_init */
static Openmoko * _openmoko_init(LockerPluginHelper * helper)
{
	Openmoko * openmoko;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if((openmoko = object_new(sizeof(*openmoko))) == NULL)
		return NULL;
	openmoko->helper = helper;
	openmoko->window = NULL;
#if defined(__linux__)
	openmoko->channel = NULL;
	openmoko->source = 0;
	if(_openmoko_on_reset(openmoko) == TRUE)
		openmoko->source = g_timeout_add(1000, _openmoko_on_reset,
				openmoko);
#endif
	return openmoko;
}


/* openmoko_destroy */
static void _openmoko_destroy(Openmoko * openmoko)
{
#if defined(__linux__)
	LockerPluginHelper * helper = openmoko->helper;
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
		helper->error(NULL, error->message, 1);
		g_error_free(error);
	}
#endif
	object_delete(openmoko);
}


/* openmoko_event */
static int _event_suspending(Openmoko * openmoko);

static int _openmoko_event(Openmoko * openmoko, LockerEvent event)
{
	switch(event)
	{
		case LOCKER_EVENT_SUSPENDING:
			return _event_suspending(openmoko);
		default:
			break;
	}
	return 0;
}

static int _event_suspending(Openmoko * openmoko)
{
#if defined(__linux__)
	LockerPluginHelper * helper = openmoko->helper;
	int fd;
	const char apm[] = "/proc/apm";
	char buf[80];
	ssize_t buf_cnt;
	double d;
	unsigned int u;
	unsigned int charging = 0;
	int i;

	if((fd = open(apm, O_RDONLY)) < 0)
	{
		error_set("%s: %s", apm, strerror(errno));
		helper->error(NULL, error_get(), 1);
		return 0;
	}
	errno = ENODATA;
	if((buf_cnt = read(fd, buf, sizeof(buf))) <= 0)
	{
		error_set("%s: %s", apm, strerror(errno));
		close(fd);
		return 0.0 / 0.0;
	}
	buf[--buf_cnt] = '\0';
	if(sscanf(buf, "%lf %lf %x %x %x %x %d%% %d min", &d, &d, &u, &charging,
				&u, &u, &i, &i) != 8)
		error_set("%s: %s", apm, strerror(errno));
	close(fd);
	return (charging != 0) ? -1 : 0;
#endif
	return 0;
}


/* useful */
#if defined(__linux__)
/* openmoko_show_dialog */
static gboolean _dialog_on_closex(gpointer data);
static void _dialog_on_lock(gpointer data);
static void _dialog_on_suspend(gpointer data);
static void _dialog_on_shutdown(gpointer data);
enum { RES_CANCEL, RES_REBOOT, RES_SHUTDOWN };

static void _openmoko_show_dialog(Openmoko * openmoko)
{
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
				_dialog_on_closex), openmoko);
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
			openmoko);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	/* suspend */
	widget = gtk_button_new_with_label("Suspend");
	image = gtk_image_new_from_icon_name("gtk-media-pause",
			GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(widget), image);
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(
				_dialog_on_suspend), openmoko);
	/* shutdown */
	widget = gtk_button_new_with_label("Shutdown");
	image = gtk_image_new_from_icon_name("gnome-shutdown",
			GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(widget), image);
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(
				_dialog_on_shutdown), openmoko);
	/* FIXME implement */
	gtk_widget_show(openmoko->window);
}

static gboolean _dialog_on_closex(gpointer data)
{
	Openmoko * openmoko = data;

	gtk_widget_hide(openmoko->window);
	return TRUE;
}

static void _dialog_on_lock(gpointer data)
{
	Openmoko * openmoko = data;
	LockerPluginHelper * helper = openmoko->helper;

	helper->action(helper->locker, LOCKER_ACTION_LOCK);
}

/* FIXME code duplicated from Panel */
static void _dialog_on_shutdown(gpointer data)
{
	Openmoko * openmoko = data;
	LockerPluginHelper * helper = openmoko->helper;
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
	Openmoko * openmoko = data;
	LockerPluginHelper * helper = openmoko->helper;

	helper->action(helper->locker, LOCKER_ACTION_SUSPEND);
}
#endif


/* callbacks */
#if defined(__linux__)
/* openmoko_on_reset */
static gboolean _openmoko_on_reset(gpointer data)
{
	Openmoko * openmoko = data;
	LockerPluginHelper * helper = openmoko->helper;
	char const * device;
	int fd;
	char buf[256];
	GError * error = NULL;

	/* FIXME open all of the relevant input event nodes */
	if((device = helper->config_get(helper->locker, "openmoko", "device"))
			== NULL)
		device = "/dev/input/event0";
	if((fd = open(device, O_RDONLY)) < 0)
	{
		snprintf(buf, sizeof(buf), "%s: %s", device, strerror(errno));
		helper->error(NULL, buf, 1);
		return TRUE;
	}
#ifdef DEBUG
	snprintf(buf, sizeof(buf), "%s", "Unknown");
	if(ioctl(fd, EVIOCGNAME(sizeof(buf)), buf) == 0)
		fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__, buf);
#endif
	openmoko->channel = g_io_channel_unix_new(fd);
	if(g_io_channel_set_encoding(openmoko->channel, NULL, &error)
			!= G_IO_STATUS_NORMAL)
	{
		helper->error(NULL, error->message, 1);
		g_error_free(error);
	}
	g_io_channel_set_buffered(openmoko->channel, FALSE);
	openmoko->source = g_io_add_watch(openmoko->channel, G_IO_IN,
			_openmoko_on_watch_can_read, openmoko);
	return FALSE;
}


/* openmoko_on_watch_can_read */
static gboolean _watch_can_read_event(Openmoko * openmoko,
		struct input_event * event);
static void _watch_can_read_event_key(Openmoko * openmoko, uint16_t code,
		int32_t value);
static gboolean _watch_can_read_reset(Openmoko * openmoko);

static gboolean _openmoko_on_watch_can_read(GIOChannel * source,
		GIOCondition condition, gpointer data)
{
	Openmoko * openmoko = data;
	LockerPluginHelper * helper = openmoko->helper;
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
			helper->error(NULL, error->message, 1);
			g_error_free(error);
		case G_IO_STATUS_EOF:
		default:
			return _watch_can_read_reset(openmoko);
	}
	/* FIXME avoid this by using a regular read() */
	if(cnt != sizeof(event))
		return _watch_can_read_reset(openmoko);
	return _watch_can_read_event(openmoko, &event);
}

static gboolean _watch_can_read_event(Openmoko * openmoko,
		struct input_event * event)
{
	switch(event->type)
	{
		case EV_KEY:
			_watch_can_read_event_key(openmoko, event->code,
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

static void _watch_can_read_event_key(Openmoko * openmoko, uint16_t code,
		int32_t value)
{
	LockerPluginHelper * helper = openmoko->helper;

	switch(code)
	{
		case KEY_PHONE:
			if(value == 0) /* released */
				helper->action(helper->locker,
						LOCKER_ACTION_LOCK);
			break;
		case KEY_POWER:
			if(value == 0) /* released */
				_openmoko_show_dialog(openmoko);
			break;
	}
}

static gboolean _watch_can_read_reset(Openmoko * openmoko)
{
	LockerPluginHelper * helper = openmoko->helper;
	GError * error = NULL;

	if(g_io_channel_shutdown(openmoko->channel, TRUE, &error)
			== G_IO_STATUS_ERROR)
	{
		helper->error(NULL, error->message, 1);
		g_error_free(error);
	}
	openmoko->source = g_timeout_add(1000, _openmoko_on_reset, openmoko);
	return FALSE;
}
#endif
