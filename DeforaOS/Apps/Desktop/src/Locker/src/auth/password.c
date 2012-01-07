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
/* TODO:
 * - detect if caps lock is pressed
 * - store the password hashed */



#include <System.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <libintl.h>
#include "Locker.h"
#include "../../config.h"
#define _(string) gettext(string)


/* Password */
/* private */
/* types */
typedef struct _LockerAuth
{
	LockerAuthHelper * helper;

	guint source;

	/* widgets */
	GtkWidget * widget;
	GtkWidget * password;
	GtkWidget * button;
	GtkWidget * wrong;
} Password;


/* prototypes */
/* plug-in */
static Password * _password_init(LockerAuthHelper * helper);
static void _password_destroy(Password * password);
static GtkWidget * _password_get_widget(Password * password);
static int _password_action(Password * password, LockerAction action);

/* callbacks */
static void _password_on_password_activate(gpointer data);
static gboolean _password_on_password_wrong(gpointer data);
static gboolean _password_on_timeout(gpointer data);


/* public */
/* variables */
/* plug-in */
LockerAuthDefinition plugin =
{
	"Password",
	NULL,
	NULL,
	_password_init,
	_password_destroy,
	_password_get_widget,
	_password_action,
};


/* private */
/* functions */
/* password_init */
static Password * _password_init(LockerAuthHelper * helper)
{
	Password * password;
	PangoFontDescription * bold;
	const GdkColor white = { 0x0, 0xffff, 0xffff, 0xffff };
	const GdkColor red = { 0x0, 0xffff, 0x0000, 0x0000 };
	GtkWidget * hbox;
	GtkWidget * widget;
	char buf[256];

	if((password = object_new(sizeof(*password))) == NULL)
		return NULL;
	password->helper = helper;
	password->source = 0;
	bold = pango_font_description_new();
	pango_font_description_set_weight(bold, PANGO_WEIGHT_BOLD);
	password->widget = gtk_vbox_new(FALSE, 4);
	/* centering */
	widget = gtk_label_new(NULL);
	gtk_box_pack_start(GTK_BOX(password->widget), widget, TRUE, TRUE, 0);
	/* hostname */
	if(gethostname(buf, sizeof(buf)) != 0)
		snprintf(buf, sizeof(buf), "%s", "DeforaOS " PACKAGE);
	else
		buf[sizeof(buf) - 1] = '\0';
	widget = gtk_label_new(buf);
	gtk_widget_modify_fg(widget, GTK_STATE_NORMAL, &white);
	gtk_widget_modify_font(widget, bold);
	gtk_box_pack_start(GTK_BOX(password->widget), widget, FALSE, TRUE, 0);
	/* screen */
	snprintf(buf, sizeof(buf), "%s %s", _("This screen is locked by"),
			getenv("USER")); /* XXX better source? */
	widget = gtk_label_new(buf);
	gtk_widget_modify_fg(widget, GTK_STATE_NORMAL, &white);
	gtk_box_pack_start(GTK_BOX(password->widget), widget, FALSE, TRUE, 0);
	/* prompt */
	widget = gtk_label_new(_("Enter password: "));
	gtk_widget_modify_fg(widget, GTK_STATE_NORMAL, &white);
	gtk_box_pack_start(GTK_BOX(password->widget), widget, FALSE, TRUE, 0);
	hbox = gtk_hbox_new(FALSE, 4);
	/* left padding (centering) */
	widget = gtk_label_new(NULL);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	/* entry */
	password->password = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(password->password), FALSE);
	g_signal_connect_swapped(password->password, "activate", G_CALLBACK(
				_password_on_password_activate), password);
	gtk_box_pack_start(GTK_BOX(hbox), password->password, FALSE, TRUE, 0);
	/* button */
	password->button = gtk_button_new_from_stock(GTK_STOCK_OK);
	g_signal_connect_swapped(password->button, "clicked", G_CALLBACK(
				_password_on_password_activate), password);
	gtk_box_pack_start(GTK_BOX(hbox), password->button, FALSE, TRUE, 0);
	/* right padding (centering) */
	widget = gtk_label_new(NULL);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(password->widget), hbox, FALSE, TRUE, 0);
	/* wrong */
	password->wrong = gtk_label_new(_("Wrong password!"));
	gtk_widget_modify_fg(password->wrong, GTK_STATE_NORMAL, &red);
	gtk_widget_modify_font(password->wrong, bold);
	/* FIXME always show but display the current error instead */
	gtk_widget_set_no_show_all(password->wrong, TRUE);
	gtk_box_pack_start(GTK_BOX(password->widget), password->wrong, FALSE,
			TRUE, 0);
	/* centering */
	widget = gtk_label_new(NULL);
	gtk_box_pack_start(GTK_BOX(password->widget), widget, TRUE, TRUE, 0);
	gtk_widget_show_all(password->widget);
	pango_font_description_free(bold);
	return password;
}


/* password_destroy */
static void _password_destroy(Password * password)
{
	if(password->source != 0)
		g_source_remove(password->source);
	object_delete(password);
}


/* password_get_widget */
static GtkWidget * _password_get_widget(Password * password)
{
	return password->widget;
}


/* password_action */
static int _password_action(Password * password, LockerAction action)
{
	LockerAuthHelper * helper = password->helper;
	GtkWidget * entry = password->password;
	char const * p;

	switch(action)
	{
		case LOCKER_ACTION_LOCK:
			if((p = helper->config_get(helper->locker, "password",
							"password")) == NULL)
			{
				gtk_entry_set_text(GTK_ENTRY(entry), "");
				return -helper->error(helper->locker,
						_("No password was set"), 1);
			}
			gtk_widget_set_sensitive(entry, TRUE);
			gtk_widget_set_sensitive(password->button, TRUE);
			gtk_entry_set_text(GTK_ENTRY(entry), "");
			gtk_widget_grab_focus(entry);
			if(password->source != 0)
				g_source_remove(password->source);
			password->source = g_timeout_add(30000,
					_password_on_timeout, password);
			break;
		case LOCKER_ACTION_UNLOCK:
			if(password->source != 0)
				g_source_remove(password->source);
			password->source = 0;
			break;
		default:
			break;
	}
	return 0;
}


/* callbacks */
/* password_on_password_activate */
static void _password_on_password_activate(gpointer data)
{
	Password * password = data;
	LockerAuthHelper * helper = password->helper;
	char const * text;
	char const * p;

	if(password->source != 0)
		g_source_remove(password->source);
	password->source = 0;
	gtk_widget_set_sensitive(password->password, FALSE);
	gtk_widget_set_sensitive(password->button, FALSE);
	text = gtk_entry_get_text(GTK_ENTRY(password->password));
	if((p = helper->config_get(helper->locker, "password", "password"))
			== NULL)
	{
		gtk_entry_set_text(GTK_ENTRY(password->password), "");
		helper->error(NULL, _("No password was set"), 1);
		return;
	}
	if(strcmp(text, p) == 0)
	{
		gtk_entry_set_text(GTK_ENTRY(password->password), "");
		helper->action(helper->locker, LOCKER_ACTION_UNLOCK);
		return;
	}
	gtk_entry_set_text(GTK_ENTRY(password->password), "");
	helper->error(NULL, _("Authentication failed"), 1);
	gtk_widget_show(password->wrong);
	password->source = g_timeout_add(3000, _password_on_password_wrong,
			password);
}


/* password_on_password_wrong */
static gboolean _password_on_password_wrong(gpointer data)
{
	Password * password = data;

	password->source = 0;
	gtk_widget_hide(password->wrong);
	gtk_widget_set_sensitive(password->password, TRUE);
	gtk_widget_set_sensitive(password->button, TRUE);
	gtk_entry_set_text(GTK_ENTRY(password->password), "");
	gtk_widget_grab_focus(password->password);
	return FALSE;
}


/* password_on_timeout */
static gboolean _password_on_timeout(gpointer data)
{
	Password * password = data;
	LockerAuthHelper * helper = password->helper;

	password->source = 0;
	helper->action(helper->locker, LOCKER_ACTION_ACTIVATE);
	return FALSE;
}
