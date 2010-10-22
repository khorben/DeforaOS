/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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
/* window */
gboolean on_closex(gpointer data)
{
	Mailer * mailer = data;

	/* FIXME may be composing or viewing messages */
	gtk_main_quit();
	return FALSE;
}


/* file menu */
void on_file_new_mail(gpointer data)
{
	Mailer * mailer = data;

	compose_new(mailer);
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
	Mailer * mailer = data;

	/* FIXME implement only if selection */
	compose_new(mailer);
}


void on_new_mail(gpointer data)
{
	Mailer * mailer = data;

	compose_new(mailer);
}


void on_preferences(gpointer data)
{
	Mailer * mailer = data;

	mailer_show_preferences(mailer, TRUE);
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


/* compose window */
gboolean on_compose_closex(gpointer data)
{
	Compose * compose = data;

#if 0 /* XXX disabled for now */
	if(compose_save(compose) != 0)
		return TRUE;
#endif
	compose_delete(compose);
	return TRUE;
}


void on_compose_save(gpointer data)
{
	Compose * c = data;

	compose_save(c);
}


void on_compose_send(gpointer data)
{
	Compose * c = data;

	compose_send(c);
}


/* compose file menu */
void on_compose_file_new(gpointer data)
{
	Compose * compose = data;
	Mailer * mailer;

	mailer = compose_get_mailer(compose);
	compose_new(mailer);
}


void on_compose_file_send(gpointer data)
{
	Compose * c = data;

	compose_send(c);
}


void on_compose_file_close(gpointer data)
{
	Compose * compose = data;

	on_compose_closex(compose);
}


/* compose view menu */
/* on_compose_view_cc */
void on_compose_view_cc(gpointer data)
{
	Compose * compose = data;

	compose_toggle_show_cc(compose);
}


/* on_compose_view_bcc */
void on_compose_view_bcc(gpointer data)
{
	Compose * compose = data;

	compose_toggle_show_bcc(compose);
}


/* on_compose_help_about */
void on_compose_help_about(gpointer data)
{
	Compose * compose = data;

	compose_show_about(compose, TRUE);
}


/* send mail */
gboolean on_send_closex(gpointer data)
{
	Compose * compose = data;

	on_send_cancel(compose);
	return FALSE;
}


/* on_send_cancel */
void on_send_cancel(gpointer data)
{
	Compose * compose = data;

	compose_send_cancel(compose);
}
