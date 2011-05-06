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



#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "compose.h"
#include "mailer.h"
#include "compose-callbacks.h"


/* functions */
/* public */
/* callbacks */
/* compose window */
void on_compose_attach(gpointer data)
{
	Compose * compose = data;

	compose_attach_dialog(compose);
}


gboolean on_compose_closex(gpointer data)
{
	Compose * compose = data;

	if(compose_close(compose) == TRUE)
	{
		if(compose_get_mailer(compose) == NULL)
			gtk_main_quit(); /* XXX ugly */
		else
			compose_delete(compose);
	}
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
	/* FIXME set the font */
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


/* compose edit menu */
/* on_compose_edit_copy */
void on_compose_edit_copy(gpointer data)
{
	Compose * compose = data;

	compose_copy(compose);
}


/* on_compose_edit_cut */
void on_compose_edit_cut(gpointer data)
{
	Compose * compose = data;

	compose_cut(compose);
}


/* on_compose_edit_paste */
void on_compose_edit_paste(gpointer data)
{
	Compose * compose = data;

	compose_paste(compose);
}


/* on_compose_edit_select_all */
void on_compose_edit_select_all(gpointer data)
{
	Compose * compose = data;

	compose_select_all(compose);
}


/* compose view menu */
/* on_compose_view_add_field */
void on_compose_view_add_field(gpointer data)
{
	Compose * compose = data;

	compose_add_field(compose, NULL, NULL);
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
