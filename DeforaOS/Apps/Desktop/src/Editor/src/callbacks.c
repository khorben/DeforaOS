/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Editor */
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
#include "editor.h"
#include "callbacks.h"
#include "../config.h"
#define _(string) gettext(string)


/* public */
/* functions */
gboolean on_closex(gpointer data)
{
	Editor * editor = data;

	return editor_close(editor);
}


/* on_edit_copy */
void on_edit_copy(gpointer data)
{
	Editor * editor = data;

	editor_copy(editor);
}


/* on_edit_cut */
void on_edit_cut(gpointer data)
{
	Editor * editor = data;

	editor_cut(editor);
}


/* on_edit_find */
void on_edit_find(gpointer data)
{
	Editor * editor = data;

	editor_find(editor, NULL);
}


/* on_edit_paste */
void on_edit_paste(gpointer data)
{
	Editor * editor = data;

	editor_paste(editor);
}


/* on_edit_preferences */
void on_edit_preferences(gpointer data)
{
	Editor * editor = data;

	editor_show_preferences(editor, TRUE);
}


/* on_edit_select_all */
void on_edit_select_all(gpointer data)
{
	Editor * editor = data;

	editor_select_all(editor);
}


/* on_edit_unselect_all */
void on_edit_unselect_all(gpointer data)
{
	Editor * editor = data;

	editor_unselect_all(editor);
}


/* on_file_close */
void on_file_close(gpointer data)
{
	Editor * editor = data;

	editor_close(editor);
}


/* on_file_new */
void on_file_new(gpointer data)
{
	Editor * editor = data;

	editor_open(editor, NULL);
}


/* on_file_open */
void on_file_open(gpointer data)
{
	Editor * editor = data;

	editor_open_dialog(editor);
}


/* on_file_save */
void on_file_save(gpointer data)
{
	Editor * editor = data;

	editor_save(editor);
}


/* on_file_save_as */
void on_file_save_as(gpointer data)
{
	Editor * editor = data;

	editor_save_as_dialog(editor);
}


/* on_help_about */
void on_help_about(gpointer data)
{
	Editor * editor = data;

	editor_about(editor);
}


/* on_insert_file */
void on_insert_file(gpointer data)
{
	Editor * editor = data;

	editor_insert_file_dialog(editor);
}


/* toolbar */
/* on_close */
void on_close(gpointer data)
{
	Editor * editor = data;

	editor_close(editor);
}


/* on_copy */
void on_copy(gpointer data)
{
	Editor * editor = data;

	editor_copy(editor);
}


/* on_cut */
void on_cut(gpointer data)
{
	Editor * editor = data;

	editor_cut(editor);
}


/* on_new */
void on_new(gpointer data)
{
	Editor * editor = data;

	editor_open(editor, NULL);
}


/* on_open */
void on_open(gpointer data)
{
	Editor * editor = data;

	editor_open_dialog(editor);
}


/* on_paste */
void on_paste(gpointer data)
{
	Editor * editor = data;

	editor_paste(editor);
}


/* on_save */
void on_save(gpointer data)
{
	Editor * editor = data;

	editor_save(editor);
}


/* on_save_as */
void on_save_as(gpointer data)
{
	Editor * editor = data;

	editor_save_as_dialog(editor);
}


/* on_preferences */
void on_preferences(gpointer data)
{
	Editor * editor = data;

	on_edit_preferences(editor);
}
