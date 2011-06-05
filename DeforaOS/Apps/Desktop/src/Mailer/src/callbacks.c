/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Mailer */
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
 * - check that the messages font button is initialized correctly */



#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "compose.h"
#include "mailer.h"
#include "callbacks.h"


/* functions */
/* public */
/* callbacks */
/* on_closex */
gboolean on_closex(gpointer data)
{
	Mailer * mailer = data;

	/* FIXME may be composing or viewing messages */
	gtk_main_quit();
	return FALSE;
}


/* on_headers_closex */
gboolean on_headers_closex(gpointer data)
{
	Mailer * mailer = data;

	mailer_show_headers(mailer, FALSE);
	return TRUE;
}


/* on_body_closex */
gboolean on_body_closex(gpointer data)
{
	Mailer * mailer = data;

	mailer_show_body(mailer, FALSE);
	return TRUE;
}


/* file menu */
void on_file_new_mail(gpointer data)
{
	on_new_mail(data);
}


void on_file_quit(gpointer data)
{
	on_closex(data);
}


/* message menu */
void on_message_delete(gpointer data)
{
	Mailer * mailer = data;

	mailer_delete_selected(mailer);
}


void on_message_reply(gpointer data)
{
	Mailer * mailer = data;

	mailer_reply_selected(mailer);
}


void on_message_reply_to_all(gpointer data)
{
	Mailer * mailer = data;

	mailer_reply_selected_to_all(mailer);
}


void on_message_forward(gpointer data)
{
	on_forward(data);
}


void on_message_view_source(gpointer data)
{
	Mailer * mailer = data;

	mailer_open_selected_source(mailer);
}


/* edit menu */
void on_edit_cut(gpointer data)
{
	Mailer * mailer = data;

	mailer_cut(mailer);
}


void on_edit_copy(gpointer data)
{
	Mailer * mailer = data;

	mailer_copy(mailer);
}


void on_edit_paste(gpointer data)
{
	Mailer * mailer = data;

	mailer_paste(mailer);
}


void on_edit_preferences(gpointer data)
{
	Mailer * mailer = data;

	mailer_show_preferences(mailer, TRUE);
}


void on_edit_select_all(gpointer data)
{
	Mailer * mailer = data;

	mailer_select_all(mailer);
}


void on_edit_unselect_all(gpointer data)
{
	Mailer * mailer = data;

	mailer_unselect_all(mailer);
}


/* help menu */
void on_help_about(gpointer data)
{
	Mailer * mailer = data;

	mailer_show_about(mailer, TRUE);
}


/* toolbar */
void on_delete(gpointer data)
{
	Mailer * mailer = data;

	mailer_delete_selected(mailer);
}


void on_forward(gpointer data)
{
	/* FIXME return directly if there is no selection */
	on_new_mail(data);
	/* FIXME really implement */
}


void on_new_mail(gpointer data)
{
	Mailer * mailer = data;
	Compose * compose;
	char const * p;

	if((compose = compose_new(mailer)) == NULL)
		return;
	if((p = mailer_get_config(mailer, "messages_font")) != NULL)
		compose_set_font(compose, p);
}


void on_preferences(gpointer data)
{
	Mailer * mailer = data;

	mailer_show_preferences(mailer, TRUE);
}


void on_quit(gpointer data)
{
	on_closex(data);
}


void on_reply(gpointer data)
{
	Mailer * mailer = data;

	mailer_reply_selected(mailer);
}


void on_reply_to_all(gpointer data)
{
	Mailer * mailer = data;

	mailer_reply_selected_to_all(mailer);
}


void on_view_source(gpointer data)
{
	Mailer * mailer = data;

	mailer_open_selected_source(mailer);
}
