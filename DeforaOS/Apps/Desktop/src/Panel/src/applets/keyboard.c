/* $Id$ */
/* Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org> */
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
 * - move and resize when the root window (or size) changes
 * - dlopen() keyboard's binary by default */



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


/* Keyboard */
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
} Keyboard;


/* prototypes */
/* plug-in */
static Keyboard * _keyboard_init(PanelAppletHelper * helper,
		GtkWidget ** widget);
static void _keyboard_destroy(Keyboard * keyboard);
static GtkWidget * _keyboard_settings(Keyboard * keyboard, gboolean apply,
		gboolean reset);

/* useful */
static int _keyboard_spawn(Keyboard * keyboard, unsigned long * xid);

/* callbacks */
static void _keyboard_on_child(GPid pid, gint status, gpointer data);
static gboolean _keyboard_on_removed(void);
static void _keyboard_on_toggled(GtkWidget * widget, gpointer data);


/* constants */
#define PANEL_KEYBOARD_COMMAND_DEFAULT "keyboard -x"


/* public */
/* variables */
PanelAppletDefinition applet =
{
	"Keyboard",
	"input-keyboard",
	NULL,
	_keyboard_init,
	_keyboard_destroy,
	_keyboard_settings,
	FALSE,
	TRUE
};


/* private */
/* functions */
/* keyboard_init */
static void _init_size(Keyboard * keyboard, PanelAppletHelper * helper);
/* callbacks */
static gboolean _init_idle(gpointer data);

static Keyboard * _keyboard_init(PanelAppletHelper * helper,
		GtkWidget ** widget)
{
	Keyboard * keyboard;
	GtkWidget * ret;
	GtkWidget * image;

	if((keyboard = malloc(sizeof(*keyboard))) == NULL)
		return NULL;
	keyboard->helper = helper;
	keyboard->source = 0;
	keyboard->pid = -1;
	keyboard->width = -1;
	keyboard->height = -1;
	keyboard->window = NULL;
	keyboard->pr_box = NULL;
	_init_size(keyboard, helper);
	ret = gtk_toggle_button_new();
#if GTK_CHECK_VERSION(2, 12, 0)
	gtk_widget_set_tooltip_text(ret, _("Show keyboard"));
#endif
	gtk_button_set_relief(GTK_BUTTON(ret), GTK_RELIEF_NONE);
	g_signal_connect(G_OBJECT(ret), "toggled", G_CALLBACK(
				_keyboard_on_toggled), keyboard);
	image = gtk_image_new_from_icon_name("input-keyboard",
			helper->icon_size);
	gtk_container_add(GTK_CONTAINER(ret), image);
	gtk_widget_show_all(ret);
	keyboard->source = g_idle_add(_init_idle, keyboard);
	*widget = ret;
	return keyboard;
}

static void _init_size(Keyboard * keyboard, PanelAppletHelper * helper)
{
	char const * p;
	char * q;
	long l;

	if((p = helper->config_get(helper->panel, "keyboard", "width")) != NULL
			&& p[0] != '\0' && (l = strtol(p, &q, 0)) > 0
			&& *q == '\0')
		keyboard->width = l;
	if((p = helper->config_get(helper->panel, "keyboard", "height")) != NULL
			&& p[0] != '\0' && (l = strtol(p, &q, 0)) > 0
			&& *q == '\0')
		keyboard->height = l;
	if(keyboard->width == -1 && keyboard->height == -1)
	{
		keyboard->width = 480;
		keyboard->height = 160;
	}
	else if(keyboard->width == -1)
		keyboard->width = keyboard->height * 3;
	else if(keyboard->height == -1)
		keyboard->height = keyboard->width / 3;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() width=%d height=%d\n", __func__,
			keyboard->width, keyboard->height);
#endif
}

/* callbacks */
static gboolean _init_idle(gpointer data)
{
	Keyboard * keyboard = data;

	keyboard->source = 0;
	if(keyboard->window != NULL)
		return FALSE;
	keyboard->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_accept_focus(GTK_WINDOW(keyboard->window), FALSE);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_focus_on_map(GTK_WINDOW(keyboard->window), FALSE);
#endif
	/* XXX let this be configurable (resize applications automatically) */
	gtk_window_set_type_hint(GTK_WINDOW(keyboard->window),
			GDK_WINDOW_TYPE_HINT_DOCK);
	keyboard->socket = gtk_socket_new();
	gtk_widget_set_size_request(keyboard->socket, keyboard->width,
			keyboard->height);
	g_signal_connect(keyboard->socket, "plug-removed", G_CALLBACK(
				_keyboard_on_removed), NULL);
	gtk_container_add(GTK_CONTAINER(keyboard->window), keyboard->socket);
	gtk_widget_show(keyboard->socket);
	return FALSE;
}


/* keyboard_destroy */
static void _keyboard_destroy(Keyboard * keyboard)
{
	if(keyboard->source > 0)
		g_source_remove(keyboard->source);
	if(keyboard->pid > 0)
		g_spawn_close_pid(keyboard->pid);
	free(keyboard);
}


/* keyboard_settings */
static void _settings_apply(Keyboard * keyboard, PanelAppletHelper * helper);
static void _settings_reset(Keyboard * keyboard, PanelAppletHelper * helper);
static GtkWidget * _settings_widget(Keyboard * keyboard);
/* callbacks */
static void _settings_on_width_value_changed(gpointer data);
static void _settings_on_height_value_changed(gpointer data);

static GtkWidget * _keyboard_settings(Keyboard * keyboard, gboolean apply,
		gboolean reset)
{
	PanelAppletHelper * helper = keyboard->helper;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, %s, %s)\n", __func__, (void *)applet,
			apply ? "TRUE" : "FALSE", reset ? "TRUE" : "FALSE");
#endif
	if(keyboard->pr_box == NULL)
	{
		keyboard->pr_box = _settings_widget(keyboard);
		reset = TRUE;
	}
	if(reset == TRUE)
		_settings_reset(keyboard, helper);
	if(apply == TRUE)
		_settings_apply(keyboard, helper);
	return keyboard->pr_box;
}

static void _settings_apply(Keyboard * keyboard, PanelAppletHelper * helper)
{
	char const * p;
	char buf[16];

	p = gtk_entry_get_text(GTK_ENTRY(keyboard->pr_command));
	helper->config_set(helper->panel, "keyboard", "command", p);
	keyboard->width = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(
				keyboard->pr_width));
	snprintf(buf, sizeof(buf), "%u", (unsigned)keyboard->width);
	helper->config_set(helper->panel, "keyboard", "width", buf);
	keyboard->height = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(
				keyboard->pr_height));
	snprintf(buf, sizeof(buf), "%u", (unsigned)keyboard->height);
	helper->config_set(helper->panel, "keyboard", "height", buf);
	gtk_widget_set_size_request(keyboard->socket, keyboard->width,
			keyboard->height);
}

static void _settings_reset(Keyboard * keyboard, PanelAppletHelper * helper)
{
	char const * p;
	gdouble value;

	if((p = helper->config_get(helper->panel, "keyboard", "command"))
			== NULL)
		p = PANEL_KEYBOARD_COMMAND_DEFAULT;
	gtk_entry_set_text(GTK_ENTRY(keyboard->pr_command), p);
	value = keyboard->width;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(keyboard->pr_width), value);
	value = keyboard->height;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(keyboard->pr_height), value);
}

static GtkWidget * _settings_widget(Keyboard * keyboard)
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
	keyboard->pr_command = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), keyboard->pr_command, TRUE, TRUE, 0);
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
	keyboard->pr_width = gtk_spin_button_new_with_range(1.0, 65535.0, 1.0);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(keyboard->pr_width), 0);
	g_signal_connect_swapped(keyboard->pr_width, "value-changed",
			G_CALLBACK(_settings_on_width_value_changed), keyboard);
	gtk_box_pack_start(GTK_BOX(hbox), keyboard->pr_width, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, TRUE, 0);
	/* height */
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new(_("Height:"));
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	keyboard->pr_height = gtk_spin_button_new_with_range(1.0, 65535.0, 1.0);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(keyboard->pr_height), 0);
	g_signal_connect_swapped(keyboard->pr_height, "value-changed",
			G_CALLBACK(_settings_on_height_value_changed),
			keyboard);
	gtk_box_pack_start(GTK_BOX(hbox), keyboard->pr_height, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, TRUE, 0);
	/* ratio */
	keyboard->pr_ratio = gtk_check_button_new_with_label(_("Keep ratio"));
	gtk_box_pack_start(GTK_BOX(vbox2), keyboard->pr_ratio, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(frame), vbox2);
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, TRUE, 0);
	gtk_widget_show_all(vbox);
	return vbox;
}

/* callbacks */
static void _settings_on_width_value_changed(gpointer data)
{
	Keyboard * keyboard = data;
	gdouble value;

	value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(keyboard->pr_width));
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(keyboard->pr_ratio)))
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(keyboard->pr_height),
				value / 3);
}

static void _settings_on_height_value_changed(gpointer data)
{
	Keyboard * keyboard = data;
	gdouble value;

	value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(keyboard->pr_height));
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(keyboard->pr_ratio)))
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(keyboard->pr_width),
				value * 3);
}


/* useful */
/* keyboard_spawn */
static int _keyboard_spawn(Keyboard * keyboard, unsigned long * xid)
{
	PanelAppletHelper * helper = keyboard->helper;
	char * argv[] = { "sh", "-c", PANEL_KEYBOARD_COMMAND_DEFAULT, NULL };
	GSpawnFlags flags = G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD;
	char const * p;
	char * q = NULL;
	gboolean res;
	gint out = -1;
	GError * error = NULL;
	char buf[32];
	ssize_t size;

	if((p = helper->config_get(helper->panel, "keyboard", "command"))
			!= NULL && (q = strdup(p)) != NULL)
		argv[2] = q;
	res = g_spawn_async_with_pipes(NULL, argv, NULL, flags, NULL, NULL,
			&keyboard->pid, NULL, &out, NULL, &error);
	free(q);
	if(res != TRUE)
	{
		helper->error(helper->panel, error->message, 1);
		g_error_free(error);
		return -1;
	}
	g_child_watch_add(keyboard->pid, _keyboard_on_child, keyboard);
	if((size = read(out, buf, sizeof(buf) - 1)) <= 0) /* XXX may block */
		/* XXX not very explicit... */
		return -helper->error(helper->panel, "read", 1);
	buf[size] = '\0';
	if(sscanf(buf, "%lu", xid) != 1)
		return -1; /* XXX warn the user */
	return 0;
}


/* callbacks */
/* keyboard_on_child */
static gboolean _on_child_timeout(gpointer data);

static void _keyboard_on_child(GPid pid, gint status, gpointer data)
{
	Keyboard * keyboard = data;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u) %u\n", __func__, pid, keyboard->pid);
#endif
	if(keyboard->source != 0 || keyboard->pid != pid)
		return;
	if(WIFEXITED(status) || WIFSIGNALED(status))
	{
		g_spawn_close_pid(keyboard->pid);
		keyboard->source = g_timeout_add(1000, _on_child_timeout,
				keyboard);
	}
}

static gboolean _on_child_timeout(gpointer data)
{
	Keyboard * keyboard = data;
	unsigned long xid;

	keyboard->source = 0;
	if(_keyboard_spawn(keyboard, &xid) == 0)
		gtk_socket_add_id(GTK_SOCKET(keyboard->socket), xid);
	return FALSE;
}


/* keyboard_on_removed */
static gboolean _keyboard_on_removed(void)
{
	return TRUE;
}


/* keyboard_on_toggled */
static void _keyboard_on_toggled(GtkWidget * widget, gpointer data)
{
	Keyboard * keyboard = data;
	PanelAppletHelper * helper = keyboard->helper;
	gint x = 0;
	gint y = 0;
	gboolean push_in;
	unsigned long xid;

	if(keyboard->window == NULL)
		_init_idle(keyboard);
	if(keyboard->window == NULL)
		return;
	helper->position_menu(helper->panel, (GtkMenu*)keyboard->window, &x, &y,
			&push_in);
	gtk_window_move(GTK_WINDOW(keyboard->window), x, y);
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
	{
		if(keyboard->pid == -1)
		{
			_keyboard_spawn(keyboard, &xid);
			gtk_socket_add_id(GTK_SOCKET(keyboard->socket), xid);
		}
		gtk_widget_show(keyboard->window);
	}
	else
		gtk_widget_hide(keyboard->window);
}
