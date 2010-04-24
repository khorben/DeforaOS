/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Phone */
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



#include "phone.h"
#include "callbacks.h"


/* callbacks */
/* on_phone_closex */
gboolean on_phone_closex(gpointer data)
{
	/* FIXME is that all? */
	gtk_main_quit();
	return FALSE;
}


/* dialpad */
/* on_phone_dialpad_call */
void on_phone_dialpad_call(gpointer data)
{
	Phone * phone = data;

	phone_call(phone, NULL);
}


/* on_phone_dialpad_clicked */
void on_phone_dialpad_clicked(GtkWidget * widget, gpointer data)
{
	Phone * phone = data;
	char const * character;

	character = g_object_get_data(G_OBJECT(widget), "character");
	phone_dialpad_append(phone, *character);
}


/* on_phone_dialpad_hangup */
void on_phone_dialpad_hangup(gpointer data)
{
	Phone * phone = data;

	phone_hangup(phone);
}
