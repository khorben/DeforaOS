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
#include <string.h>
#include <libintl.h>
#include "Locker.h"
#define _(string) gettext(string)


/* Password */
/* private */
/* types */
typedef struct _Password
{
	guint source;

	/* widgets */
	GtkWidget * widget;
	GtkWidget * password;
	GtkWidget * button;
	GtkWidget * wrong;
} Password;


/* prototypes */
/* plug-in */
static GtkWidget * _password_init(LockerAuth * plugin);
static void _password_destroy(LockerAuth * plugin);
static void _password_action(LockerAuth * plugin, LockerAction action);

/* callbacks */
static void _password_on_password_activate(gpointer data);
static gboolean _password_on_password_wrong(gpointer data);
static gboolean _password_on_timeout(gpointer data);


/* public */
/* variables */
/* plug-in */
LockerAuth plugin =
{
	NULL,
	"Password",
	_password_init,
	_password_destroy,
	_password_action,
	NULL
};


/* private */
/* functions */
/* password_init */
static GtkWidget * _password_init(LockerAuth * plugin)
{
	Password * password;
	PangoFontDescription * font;
	GdkColor white = { 0x0, 0xffff, 0xffff, 0xffff };
	GdkColor red = { 0x0, 0xffff, 0x0000, 0x0000 };
	GtkWidget * hbox;
	GtkWidget * widget;

	if((password = object_new(sizeof(*password))) == NULL)
		return NULL;
	plugin->priv = password;
	password->source = 0;
	font = pango_font_description_new();
	pango_font_description_set_weight(font, PANGO_WEIGHT_BOLD);
	password->widget = gtk_vbox_new(FALSE, 4);
	hbox = gtk_hbox_new(FALSE, 4);
	/* label */
	widget = gtk_label_new(_("Enter password: "));
	gtk_misc_set_alignment(GTK_MISC(widget), 1.0, 0.5);
	gtk_widget_modify_fg(widget, GTK_STATE_NORMAL, &white);
	gtk_widget_modify_font(widget, font);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	/* entry */
	password->password = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(password->password), FALSE);
	g_signal_connect_swapped(password->password, "activate", G_CALLBACK(
				_password_on_password_activate), plugin);
	gtk_box_pack_start(GTK_BOX(hbox), password->password, FALSE, TRUE, 0);
	/* button */
	password->button = gtk_button_new_from_stock(GTK_STOCK_OK);
	g_signal_connect_swapped(password->button, "clicked", G_CALLBACK(
				_password_on_password_activate), plugin);
	gtk_box_pack_start(GTK_BOX(hbox), password->button, FALSE, TRUE, 0);
	widget = gtk_label_new(NULL);
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(password->widget), hbox, FALSE, TRUE, 0);
	gtk_widget_show_all(password->widget);
	/* wrong */
	password->wrong = gtk_label_new(_("Wrong password!"));
	gtk_widget_modify_fg(password->wrong, GTK_STATE_NORMAL, &red);
	gtk_widget_modify_font(password->wrong, font);
	gtk_box_pack_start(GTK_BOX(password->widget), password->wrong, FALSE,
			TRUE, 0);
	pango_font_description_free(font);
	return password->widget;
}


/* password_destroy */
static void _password_destroy(LockerAuth * plugin)
{
	Password * password = plugin->priv;

	if(password->source != 0)
		g_source_remove(password->source);
	object_delete(password);
}


/* password_action */
static void _password_action(LockerAuth * plugin, LockerAction action)
{
	Password * password = plugin->priv;

	switch(action)
	{
		case LOCKER_ACTION_LOCK:
			gtk_widget_set_sensitive(password->password, TRUE);
			gtk_widget_set_sensitive(password->button, TRUE);
			gtk_entry_set_text(GTK_ENTRY(password->password), "");
			gtk_widget_grab_focus(password->password);
			if(password->source != 0)
				g_source_remove(password->source);
			password->source = g_timeout_add(30000,
					_password_on_timeout, plugin);
			break;
		default:
			break;
	}
}


/* callbacks */
/* password_on_password_activate */
static void _password_on_password_activate(gpointer data)
{
	LockerAuth * plugin = data;
	LockerAuthHelper * helper = plugin->helper;
	Password * password = plugin->priv;
	gchar const * text;

	if(password->source != 0)
		g_source_remove(password->source);
	password->source = 0;
	gtk_widget_set_sensitive(password->password, FALSE);
	gtk_widget_set_sensitive(password->button, FALSE);
	text = gtk_entry_get_text(GTK_ENTRY(password->password));
	/* FIXME really check */
	if(strcmp(text, "password") == 0)
	{
		helper->action(helper->locker, LOCKER_ACTION_UNLOCK);
		return;
	}
	helper->error(helper->locker, _("Authentication failed"), 1);
	gtk_widget_show(password->wrong);
	password->source = g_timeout_add(3000, _password_on_password_wrong,
			plugin);
}


/* password_on_password_wrong */
static gboolean _password_on_password_wrong(gpointer data)
{
	LockerAuth * plugin = data;
	Password * password = plugin->priv;

	password->source = 0;
	gtk_widget_hide(password->wrong);
	gtk_widget_set_sensitive(password->password, TRUE);
	gtk_widget_set_sensitive(password->button, TRUE);
	return FALSE;
}


/* password_on_timeout */
static gboolean _password_on_timeout(gpointer data)
{
	LockerAuth * plugin = data;
	LockerAuthHelper * helper = plugin->helper;
	Password * password = plugin->priv;

	password->source = 0;
	helper->action(helper->locker, LOCKER_ACTION_ACTIVATE);
	return FALSE;
}
