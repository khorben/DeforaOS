/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
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
/* TODO:
 * - factor code with the keyboard applet
 * - move and resize when the root window (or size) changes
 * - dlopen() mixer's binary by default */



#define DEBUG
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#ifdef DEBUG
# include <stdio.h>
#endif
#include <string.h>
#include <errno.h>
#include <libintl.h>
#include <gdk/gdkx.h>
#include "Panel.h"
#define _(string) gettext(string)


/* Mixer */
/* private */
/* types */
typedef struct _PanelApplet
{
	PanelAppletHelper * helper;

	guint source;
	GPid pid;

	/* settings */
	gint width;
	gint height;

	/* widgets */
	GtkWidget * window;
	GtkWidget * socket;
	/* preferences */
	GtkWidget * pr_box;
	GtkWidget * pr_command;
	GtkWidget * pr_width;
	GtkWidget * pr_height;
	GtkWidget * pr_ratio;
} Mixer;


/* prototypes */
/* plug-in */
static Mixer * _mixer_init(PanelAppletHelper * helper,
		GtkWidget ** widget);
static void _mixer_destroy(Mixer * mixer);
static GtkWidget * _mixer_settings(Mixer * mixer, gboolean apply,
		gboolean reset);

/* useful */
static int _mixer_spawn(Mixer * mixer, unsigned long * xid);

/* callbacks */
static void _mixer_on_child(GPid pid, gint status, gpointer data);
static gboolean _mixer_on_removed(void);
static void _mixer_on_toggled(GtkWidget * widget, gpointer data);


/* constants */
#define PANEL_MIXER_COMMAND_DEFAULT "mixer -x"


/* public */
/* variables */
PanelAppletDefinition applet =
{
	"Mixer",
	"stock_volume",
	NULL,
	_mixer_init,
	_mixer_destroy,
	_mixer_settings,
	FALSE,
	TRUE
};


/* private */
/* functions */
/* mixer_init */
static void _init_size(Mixer * mixer, PanelAppletHelper * helper);
/* callbacks */
static gboolean _init_idle(gpointer data);

static Mixer * _mixer_init(PanelAppletHelper * helper,
		GtkWidget ** widget)
{
	Mixer * mixer;
	GtkWidget * ret;
	GtkWidget * image;

	if((mixer = malloc(sizeof(*mixer))) == NULL)
		return NULL;
	mixer->helper = helper;
	mixer->source = 0;
	mixer->pid = -1;
	mixer->width = -1;
	mixer->height = -1;
	mixer->window = NULL;
	mixer->pr_box = NULL;
	_init_size(mixer, helper);
	ret = gtk_toggle_button_new();
#if GTK_CHECK_VERSION(2, 12, 0)
	gtk_widget_set_tooltip_text(ret, _("Show mixer"));
#endif
	gtk_button_set_relief(GTK_BUTTON(ret), GTK_RELIEF_NONE);
	g_signal_connect(G_OBJECT(ret), "toggled", G_CALLBACK(
				_mixer_on_toggled), mixer);
	image = gtk_image_new_from_icon_name(applet.icon, helper->icon_size);
	gtk_container_add(GTK_CONTAINER(ret), image);
	gtk_widget_show_all(ret);
	mixer->source = g_idle_add(_init_idle, mixer);
	*widget = ret;
	return mixer;
}

static void _init_size(Mixer * mixer, PanelAppletHelper * helper)
{
	char const * p;
	char * q;
	long l;

	if((p = helper->config_get(helper->panel, "mixer", "width")) != NULL
			&& p[0] != '\0' && (l = strtol(p, &q, 0)) > 0
			&& *q == '\0')
		mixer->width = l;
	if((p = helper->config_get(helper->panel, "mixer", "height")) != NULL
			&& p[0] != '\0' && (l = strtol(p, &q, 0)) > 0
			&& *q == '\0')
		mixer->height = l;
	if(mixer->width == -1 && mixer->height == -1)
	{
		mixer->width = 480;
		mixer->height = 160;
	}
	else if(mixer->width == -1)
		mixer->width = mixer->height * 3;
	else if(mixer->height == -1)
		mixer->height = mixer->width / 3;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() width=%d height=%d\n", __func__,
			mixer->width, mixer->height);
#endif
}

/* callbacks */
static gboolean _init_idle(gpointer data)
{
	Mixer * mixer = data;

	mixer->source = 0;
	if(mixer->window != NULL)
		return FALSE;
	mixer->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_accept_focus(GTK_WINDOW(mixer->window), FALSE);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_focus_on_map(GTK_WINDOW(mixer->window), FALSE);
#endif
	/* XXX let this be configurable (resize applications automatically) */
	gtk_window_set_type_hint(GTK_WINDOW(mixer->window),
			GDK_WINDOW_TYPE_HINT_DOCK);
	mixer->socket = gtk_socket_new();
	gtk_widget_set_size_request(mixer->socket, mixer->width,
			mixer->height);
	g_signal_connect(mixer->socket, "plug-removed", G_CALLBACK(
				_mixer_on_removed), NULL);
	gtk_container_add(GTK_CONTAINER(mixer->window), mixer->socket);
	gtk_widget_show(mixer->socket);
	return FALSE;
}


/* mixer_destroy */
static void _mixer_destroy(Mixer * mixer)
{
	if(mixer->source > 0)
		g_source_remove(mixer->source);
	if(mixer->pid > 0)
		g_spawn_close_pid(mixer->pid);
	free(mixer);
}


/* mixer_settings */
static void _settings_apply(Mixer * mixer, PanelAppletHelper * helper);
static void _settings_reset(Mixer * mixer, PanelAppletHelper * helper);
static GtkWidget * _settings_widget(Mixer * mixer);
/* callbacks */
static void _settings_on_width_value_changed(gpointer data);
static void _settings_on_height_value_changed(gpointer data);

static GtkWidget * _mixer_settings(Mixer * mixer, gboolean apply,
		gboolean reset)
{
	PanelAppletHelper * helper = mixer->helper;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, %s, %s)\n", __func__, (void *)mixer,
			apply ? "TRUE" : "FALSE", reset ? "TRUE" : "FALSE");
#endif
	if(mixer->pr_box == NULL)
	{
		mixer->pr_box = _settings_widget(mixer);
		reset = TRUE;
	}
	if(reset == TRUE)
		_settings_reset(mixer, helper);
	if(apply == TRUE)
		_settings_apply(mixer, helper);
	return mixer->pr_box;
}

static void _settings_apply(Mixer * mixer, PanelAppletHelper * helper)
{
	char const * p;
	char buf[16];

	p = gtk_entry_get_text(GTK_ENTRY(mixer->pr_command));
	helper->config_set(helper->panel, "mixer", "command", p);
	mixer->width = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(
				mixer->pr_width));
	snprintf(buf, sizeof(buf), "%u", (unsigned)mixer->width);
	helper->config_set(helper->panel, "mixer", "width", buf);
	mixer->height = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(
				mixer->pr_height));
	snprintf(buf, sizeof(buf), "%u", (unsigned)mixer->height);
	helper->config_set(helper->panel, "mixer", "height", buf);
	gtk_widget_set_size_request(mixer->socket, mixer->width,
			mixer->height);
}

static void _settings_reset(Mixer * mixer, PanelAppletHelper * helper)
{
	char const * p;
	gdouble value;

	if((p = helper->config_get(helper->panel, "mixer", "command"))
			== NULL)
		p = PANEL_MIXER_COMMAND_DEFAULT;
	gtk_entry_set_text(GTK_ENTRY(mixer->pr_command), p);
	value = mixer->width;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(mixer->pr_width), value);
	value = mixer->height;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(mixer->pr_height), value);
}

static GtkWidget * _settings_widget(Mixer * mixer)
{
	GtkSizeGroup * group;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * vbox2;
	GtkWidget * frame;
	GtkWidget * widget;

	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	vbox = gtk_vbox_new(FALSE, 4);
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new(_("Command:"));
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	mixer->pr_command = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), mixer->pr_command, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	/* frame */
	frame = gtk_frame_new("Size:");
	vbox2 = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vbox2), 4);
	/* width */
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new(_("Width:"));
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	mixer->pr_width = gtk_spin_button_new_with_range(1.0, 65535.0, 1.0);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(mixer->pr_width), 0);
	g_signal_connect_swapped(mixer->pr_width, "value-changed",
			G_CALLBACK(_settings_on_width_value_changed), mixer);
	gtk_box_pack_start(GTK_BOX(hbox), mixer->pr_width, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, TRUE, 0);
	/* height */
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new(_("Height:"));
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	mixer->pr_height = gtk_spin_button_new_with_range(1.0, 65535.0, 1.0);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(mixer->pr_height), 0);
	g_signal_connect_swapped(mixer->pr_height, "value-changed",
			G_CALLBACK(_settings_on_height_value_changed),
			mixer);
	gtk_box_pack_start(GTK_BOX(hbox), mixer->pr_height, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, TRUE, 0);
	/* ratio */
	mixer->pr_ratio = gtk_check_button_new_with_label(_("Keep ratio"));
	gtk_box_pack_start(GTK_BOX(vbox2), mixer->pr_ratio, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(frame), vbox2);
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, TRUE, 0);
	gtk_widget_show_all(vbox);
	return vbox;
}

/* callbacks */
static void _settings_on_width_value_changed(gpointer data)
{
	Mixer * mixer = data;
	gdouble value;

	value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(mixer->pr_width));
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mixer->pr_ratio)))
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(mixer->pr_height),
				value / 3);
}

static void _settings_on_height_value_changed(gpointer data)
{
	Mixer * mixer = data;
	gdouble value;

	value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(mixer->pr_height));
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mixer->pr_ratio)))
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(mixer->pr_width),
				value * 3);
}


/* useful */
/* mixer_spawn */
static int _mixer_spawn(Mixer * mixer, unsigned long * xid)
{
	PanelAppletHelper * helper = mixer->helper;
	char * argv[] = { "sh", "-c", PANEL_MIXER_COMMAND_DEFAULT, NULL };
	GSpawnFlags flags = G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD;
	char const * p;
	char * q = NULL;
	gboolean res;
	gint out = -1;
	GError * error = NULL;
	char buf[32];
	ssize_t size;

	if((p = helper->config_get(helper->panel, "mixer", "command"))
			!= NULL && (q = strdup(p)) != NULL)
		argv[2] = q;
	res = g_spawn_async_with_pipes(NULL, argv, NULL, flags, NULL, NULL,
			&mixer->pid, NULL, &out, NULL, &error);
	free(q);
	if(res != TRUE)
	{
		helper->error(helper->panel, error->message, 1);
		g_error_free(error);
		return -1;
	}
	g_child_watch_add(mixer->pid, _mixer_on_child, mixer);
	if((size = read(out, buf, sizeof(buf) - 1)) <= 0) /* XXX may block */
		/* XXX not very explicit... */
		return -helper->error(helper->panel, "read", 1);
	buf[size] = '\0';
	if(sscanf(buf, "%lu", xid) != 1)
		return -1; /* XXX warn the user */
	return 0;
}


/* callbacks */
/* mixer_on_child */
static gboolean _on_child_timeout(gpointer data);

static void _mixer_on_child(GPid pid, gint status, gpointer data)
{
	Mixer * mixer = data;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u) %u\n", __func__, pid, mixer->pid);
#endif
	if(mixer->source != 0 || mixer->pid != pid)
		return;
	if(WIFEXITED(status) || WIFSIGNALED(status))
	{
		g_spawn_close_pid(mixer->pid);
		mixer->source = g_timeout_add(1000, _on_child_timeout,
				mixer);
	}
}

static gboolean _on_child_timeout(gpointer data)
{
	Mixer * mixer = data;
	unsigned long xid;

	mixer->source = 0;
	if(_mixer_spawn(mixer, &xid) == 0)
		gtk_socket_add_id(GTK_SOCKET(mixer->socket), xid);
	return FALSE;
}


/* mixer_on_removed */
static gboolean _mixer_on_removed(void)
{
	return TRUE;
}


/* mixer_on_toggled */
static void _mixer_on_toggled(GtkWidget * widget, gpointer data)
{
	Mixer * mixer = data;
	PanelAppletHelper * helper = mixer->helper;
	gint x = 0;
	gint y = 0;
	gboolean push_in;
	unsigned long xid;

	if(mixer->window == NULL)
		_init_idle(mixer);
	if(mixer->window == NULL)
		return;
	helper->position_menu(helper->panel, (GtkMenu*)mixer->window, &x, &y,
			&push_in);
	gtk_window_move(GTK_WINDOW(mixer->window), x, y);
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
	{
		if(mixer->pid == -1)
		{
			_mixer_spawn(mixer, &xid);
			gtk_socket_add_id(GTK_SOCKET(mixer->socket), xid);
		}
		gtk_widget_show(mixer->window);
	}
	else
		gtk_widget_hide(mixer->window);
}
