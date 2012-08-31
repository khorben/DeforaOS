/* $Id$ */
/* Copyright (c) 2010 Sébastien Bocahu <zecrazytux@zecrazytux.net> */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
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



#ifndef PDFVIEWER_CALLBACKS_H
# define PDFVIEWER_CALLBACKS_H

# include <gtk/gtk.h>


/* functions */
gboolean on_closex(gpointer data);
void on_file_close(gpointer data);
void on_file_open(gpointer data);
void on_file_properties(gpointer data);
void on_help_about(gpointer data);
void on_view_fullscreen(gpointer data);
void on_view_normal_size(gpointer data);
void on_view_zoom_in(gpointer data);
void on_view_zoom_out(gpointer data);
void pdf_render_area(GtkWidget * drawing_area, GdkEventExpose * event,
		void * data);
void on_previous(gpointer data);
void on_next(gpointer data);
void on_far_before(gpointer data);
void on_far_after(gpointer data);

/* toolbar */
void on_close(gpointer data);
void on_fullscreen(gpointer data);
void on_open(gpointer data);
void on_pdf_close(gpointer data);
void on_zoom_in(gpointer data);
void on_zoom_out(gpointer data);

#endif /* !PDFVIEWER_CALLBACKS_H */
