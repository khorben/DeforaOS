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
/* FIXME:
 * - use XActivateScreenSaver() */



#include <stdlib.h>
#include <libintl.h>
#include "Panel.h"
#define _(string) gettext(string)


/* Lock */
/* private */
/* types */
typedef struct _Lock
{
	/* preferences */
	GtkWidget * pr_box;
	GtkWidget * pr_command;
} Lock;


/* prototypes */
static GtkWidget * _lock_init(PanelApplet * applet);
static void _lock_destroy(PanelApplet * applet);
static GtkWidget * _lock_settings(PanelApplet * applet, gboolean apply,
		gboolean reset);

/* callbacks */
static void _on_clicked(gpointer data);


/* constants */
#define PANEL_LOCK_COMMAND_DEFAULT "xset s activate"


/* public */
/* variables */
PanelApplet applet =
{
	NULL,
	"Lock screen",
	NULL,
	_lock_init,
	_lock_destroy,
	_lock_settings,
	PANEL_APPLET_POSITION_START,
	FALSE,
	TRUE,
	NULL
};


/* private */
/* functions */
/* lock_init */
static GtkWidget * _lock_init(PanelApplet * applet)
{
	GtkWidget * ret;
	GtkWidget * image;

	applet->priv = NULL;
	ret = gtk_button_new();
	image = gtk_image_new_from_icon_name("gnome-lockscreen",
			applet->helper->icon_size);
	gtk_button_set_image(GTK_BUTTON(ret), image);
	gtk_button_set_relief(GTK_BUTTON(ret), GTK_RELIEF_NONE);
#if GTK_CHECK_VERSION(2, 12, 0)
	gtk_widget_set_tooltip_text(ret, _("Lock screen"));
#endif
	g_signal_connect_swapped(G_OBJECT(ret), "clicked", G_CALLBACK(
				_on_clicked), applet->helper);
	gtk_widget_show_all(ret);
	return ret;
}


/* lock_destroy */
static void _lock_destroy(PanelApplet * applet)
{
	Lock * lock = applet->priv;

	free(lock);
}


/* lock_settings */
static GtkWidget * _lock_settings(PanelApplet * applet, gboolean apply,
		gboolean reset)
{
	Lock * lock = applet->priv;
	GtkWidget * hbox;
	GtkWidget * widget;
	char const * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, %s, %s)\n", __func__, (void*)applet,
			apply ? "TRUE" : "FALSE", reset ? "TRUE" : "FALSE");
#endif
	if(lock == NULL && (lock = calloc(1, sizeof(*lock))) == NULL)
		return NULL;
	applet->priv = lock;
	if(lock->pr_box == NULL)
	{
		lock->pr_box = gtk_vbox_new(FALSE, 4);
		hbox = gtk_hbox_new(FALSE, 4);
		widget = gtk_label_new("Command:");
		gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
		lock->pr_command = gtk_entry_new();
		gtk_box_pack_start(GTK_BOX(hbox), lock->pr_command, TRUE, TRUE,
				0);
		gtk_box_pack_start(GTK_BOX(lock->pr_box), hbox, FALSE, TRUE, 0);
		gtk_widget_show_all(lock->pr_box);
		reset = TRUE;
	}
	if(reset == TRUE)
	{
		if((p = applet->helper->config_get(applet->helper->panel,
						"lock", "command")) == NULL)
			p = PANEL_LOCK_COMMAND_DEFAULT;
		gtk_entry_set_text(GTK_ENTRY(lock->pr_command), p);
	}
	if(apply == TRUE)
	{
		p = gtk_entry_get_text(GTK_ENTRY(lock->pr_command));
		applet->helper->config_set(applet->helper->panel, "lock",
				"command", p);
	}
	return lock->pr_box;
}


/* callbacks */
/* on_clicked */
static void _on_clicked(gpointer data)
{
	PanelAppletHelper * helper = data;
	char const * command = PANEL_LOCK_COMMAND_DEFAULT;
	char const * p;
	GError * error = NULL;

	if((p = helper->config_get(helper->panel, "lock", "command")) != NULL)
		command = p;
	if(g_spawn_command_line_async(command, &error) != TRUE)
		helper->error(NULL, error->message, 0);
}
