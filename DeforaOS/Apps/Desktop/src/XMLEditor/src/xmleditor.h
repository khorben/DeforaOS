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



#ifndef XMLEDITOR_XMLEDITOR_H
# define XMLEDITOR_XMLEDITOR_H

# include <gtk/gtk.h>


/* Editor */
/* types */
typedef struct _XMLEditor XMLEditor;


/* functions */
XMLEditor * xmleditor_new(void);
void xmleditor_delete(XMLEditor * xmleditor);

/* useful */
void xmleditor_about(XMLEditor * xmleditor);

int xmleditor_error(XMLEditor * xmleditor, char const * message, int ret);

gboolean xmleditor_close(XMLEditor * xmleditor);
void xmleditor_open(XMLEditor * xmleditor, char const * filename);
void xmleditor_open_dialog(XMLEditor * xmleditor);

void xmleditor_collapse_all(XMLEditor * xmleditor);
void xmleditor_expand_all(XMLEditor * xmleditor);

gboolean xmleditor_save(XMLEditor * xmleditor);
gboolean xmleditor_save_as(XMLEditor * xmleditor, char const * filename);
gboolean xmleditor_save_as_dialog(XMLEditor * xmleditor);

void xmleditor_find(XMLEditor * xmleditor, char const * text);

void xmleditor_tag_set_name(XMLEditor * xmleditor, GtkTreePath * treepath,
		char const * name);

#endif /* !XMLEDITOR_XMLEDITOR_H */
