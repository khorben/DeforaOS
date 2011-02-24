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



#ifndef LIBDESKTOP_ABOUT_H
# define LIBDESKTOP_ABOUT_H


/* About */
GtkWidget * desktop_about_dialog_new(void);
void desktop_about_dialog_set_artists(GtkWidget * about,
		char const * artists[]);
void desktop_about_dialog_set_authors(GtkWidget * about,
		char const * authors[]);
void desktop_about_dialog_set_comments(GtkWidget * about,
		char const * comments);
void desktop_about_dialog_set_copyright(GtkWidget * about,
		char const * copyright);
void desktop_about_dialog_set_license(GtkWidget * about, char const * license);
void desktop_about_dialog_set_logo_icon_name(GtkWidget * about,
		char const * icon);
/* XXX desktop_about_dialog_set_name() is deprecated */
void desktop_about_dialog_set_name(GtkWidget * about, char const * name);
void desktop_about_dialog_set_program_name(GtkWidget * about,
		char const * name);
void desktop_about_dialog_set_translator_credits(GtkWidget * about,
		char const * translator);
void desktop_about_dialog_set_version(GtkWidget * about, char const * version);
void desktop_about_dialog_set_website(GtkWidget * about, char const * website);

#endif /* !LIBDESKTOP_ABOUT_H */
