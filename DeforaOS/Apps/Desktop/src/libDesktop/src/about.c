/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop libDesktop */
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



#include "Desktop.h"


/* About */
/* desktop_about_dialog_new */
GtkWidget * desktop_about_dialog_new(void)
{
	GtkWidget * ret;

	ret = gtk_about_dialog_new();
	g_signal_connect_swapped(G_OBJECT(ret), "delete-event", G_CALLBACK(
				gtk_widget_hide), ret);
	g_signal_connect_swapped(G_OBJECT(ret), "response", G_CALLBACK(
				gtk_widget_hide), ret);
	return ret;
}


/* desktop_about_dialog_set_artists */
void desktop_about_dialog_set_artists(GtkWidget * about, char const * artists[])
{
	gtk_about_dialog_set_artists(GTK_ABOUT_DIALOG(about), artists);
}


/* desktop_about_dialog_set_authors */
void desktop_about_dialog_set_authors(GtkWidget * about, char const * authors[])
{
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(about), authors);
}


/* desktop_about_dialog_set_comments */
void desktop_about_dialog_set_comments(GtkWidget * about, char const * comments)
{
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about), comments);
}


/* desktop_about_dialog_set_copyright */
void desktop_about_dialog_set_copyright(GtkWidget * about,
		char const * copyright)
{
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about), copyright);
}


/* desktop_about_dialog_set_documenters */
void desktop_about_dialog_set_documenters(GtkWidget * about,
		char const * documenters[])
{
	gtk_about_dialog_set_documenters(GTK_ABOUT_DIALOG(about), documenters);
}


/* desktop_about_dialog_set_license */
void desktop_about_dialog_set_license(GtkWidget * about, char const * license)
{
	gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(about), license);
}


/* desktop_about_dialog_set_logo_icon_name */
void desktop_about_dialog_set_logo_icon_name(GtkWidget * about,
		char const * icon)
{
	gtk_about_dialog_set_logo_icon_name(GTK_ABOUT_DIALOG(about), icon);
}


/* desktop_about_dialog_set_name */
void desktop_about_dialog_set_name(GtkWidget * about, char const * name)
{
	desktop_about_dialog_set_program_name(about, name);
}


/* desktop_about_dialog_set_program_name */
void desktop_about_dialog_set_program_name(GtkWidget * about, char const * name)
{
#if GTK_CHECK_VERSION(2, 12, 0)
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about), name);
#else
	gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(about), name);
#endif
}


/* desktop_about_dialog_set_translator_credits */
void desktop_about_dialog_set_translator_credits(GtkWidget * about,
		char const * translator)
{
	gtk_about_dialog_set_translator_credits(GTK_ABOUT_DIALOG(about),
			translator);
}


/* desktop_about_dialog_set_version */
void desktop_about_dialog_set_version(GtkWidget * about, char const * version)
{
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about), version);
}


/* desktop_about_dialog_set_website */
void desktop_about_dialog_set_website(GtkWidget * about, char const * website)
{
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about), website);
}
