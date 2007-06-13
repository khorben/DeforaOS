/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Editor */
/* Editor is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * Editor is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Editor; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA */



#ifndef EDITOR_EDITOR_H
# define EDITOR_EDITOR_H

# include <gtk/gtk.h>


/* types */
typedef struct _Editor
{
	char * filename;
	/* widgets */
	GtkWidget * window;
	GtkWidget * view;
	GtkWidget * statusbar;
	/* preferences */
	GtkWidget * pr_window;
} Editor;


/* functions */
Editor * editor_new(void);
void editor_delete(Editor * editor);

/* useful */
gboolean editor_close(Editor * editor);
void editor_open(Editor * editor, char const * filename);
void editor_open_dialog(Editor * editor);
gboolean editor_save(Editor * editor);
gboolean editor_save_as(Editor * editor, char const * filename);
gboolean editor_save_as_dialog(Editor * editor);

#endif /* !EDITOR_EDITOR_H */
