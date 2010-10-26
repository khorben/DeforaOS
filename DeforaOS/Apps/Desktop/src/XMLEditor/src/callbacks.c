/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop XMLEditor */
/* This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>. */



#include <stdlib.h>
#include <libintl.h>
#include <Desktop.h>
#include "xmleditor.h"
#include "callbacks.h"
#include "../config.h"


/* public */
/* functions */
gboolean on_closex(gpointer data)
{
	XMLEditor * xmleditor = data;

	return xmleditor_close(xmleditor);
}


/* on_edit_preferences */
void on_edit_preferences(gpointer data)
{
	/* FIXME implement */
}


/* on_file_close */
void on_file_close(gpointer data)
{
	XMLEditor * xmleditor = data;

	xmleditor_close(xmleditor);
}


/* on_file_new */
void on_file_new(gpointer data)
{
	XMLEditor * xmleditor = data;

	xmleditor_open(xmleditor, NULL);
}


/* on_file_open */
void on_file_open(gpointer data)
{
	XMLEditor * xmleditor = data;

	xmleditor_open_dialog(xmleditor);
}


/* on_file_save */
void on_file_save(gpointer data)
{
	XMLEditor * xmleditor = data;

	xmleditor_save(xmleditor);
}


/* on_file_save_as */
void on_file_save_as(gpointer data)
{
	XMLEditor * xmleditor = data;

	xmleditor_save_as_dialog(xmleditor);
}


/* on_help_about */
void on_help_about(gpointer data)
{
	XMLEditor * xmleditor = data;

	xmleditor_about(xmleditor);
}


/* toolbar */
/* on_close */
void on_close(gpointer data)
{
	XMLEditor * xmleditor = data;

	xmleditor_close(xmleditor);
}


/* on_new */
void on_new(gpointer data)
{
	XMLEditor * xmleditor = data;

	xmleditor_open(xmleditor, NULL);
}


/* on_open */
void on_open(gpointer data)
{
	XMLEditor * xmleditor = data;

	xmleditor_open_dialog(xmleditor);
}


/* on_save */
void on_save(gpointer data)
{
	XMLEditor * xmleditor = data;

	xmleditor_save(xmleditor);
}


/* on_save_as */
void on_save_as(gpointer data)
{
	XMLEditor * xmleditor = data;

	xmleditor_save_as_dialog(xmleditor);
}


/* on_preferences */
void on_preferences(gpointer data)
{
	XMLEditor * xmleditor = data;

	on_edit_preferences(xmleditor);
}
