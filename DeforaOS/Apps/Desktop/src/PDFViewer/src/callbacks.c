/* $Id$ */
/* Copyright (c) 2010 Sébastien Bocahu <zecrazytux@zecrazytux.net> */
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
#include "pdfviewer.h"
#include "callbacks.h"
#include "../config.h"


/* public */
/* functions */
gboolean on_closex(gpointer data)
{
	PDFviewer * pdfviewer = data;

	return pdfviewer_close(pdfviewer);
}


/* on_edit_preferences */
void on_edit_preferences(gpointer data)
{
	/* FIXME implement */
}


/* on_file_close */
void on_file_close(gpointer data)
{
	PDFviewer * pdfviewer = data;

	pdfviewer_close(pdfviewer);
}


/* on_file_new */
void on_file_new(gpointer data)
{
	PDFviewer * pdfviewer = data;

	pdfviewer_open(pdfviewer, NULL);
}


/* on_file_open */
void on_file_open(gpointer data)
{
	PDFviewer * pdfviewer = data;

	pdfviewer_open_dialog(pdfviewer);
}


/* on_view_fullscreen */
void on_view_fullscreen(gpointer data)
{
	PDFviewer * pdfviewer = data;

	on_fullscreen(pdfviewer);
}


/* on_view_zoom_in */
void on_view_zoom_in(gpointer data)
{
	on_zoom_in(data);
}


/* on_view_zoom_out */
void on_view_zoom_out(gpointer data)
{
	on_zoom_out(data);
}


/* on_help_about */
void on_help_about(gpointer data)
{
	PDFviewer * pdfviewer = data;

	pdfviewer_about(pdfviewer);
}


/* toolbar */
/* on_close */
void on_close(gpointer data)
{
	PDFviewer * pdfviewer = data;

	pdfviewer_close(pdfviewer);
}


/* on_fullscreen */
void on_fullscreen(gpointer data)
{
	PDFviewer * pdfviewer = data;

	pdfviewer_fullscreen_toggle(pdfviewer);
}


/* on_new */
void on_new(gpointer data)
{
	PDFviewer * pdfviewer = data;

	pdfviewer_open(pdfviewer, NULL);
}


/* on_open */
void on_open(gpointer data)
{
	PDFviewer * pdfviewer = data;

	pdfviewer_open_dialog(pdfviewer);
}


/* on_preferences */
void on_preferences(gpointer data)
{
	PDFviewer * pdfviewer = data;

	on_edit_preferences(pdfviewer);
}


/* on_previous */
void on_previous(gpointer data)
{
	PDFviewer * pdfviewer = data;

	pdf_update_current(pdfviewer, '-', 1);
	pdf_load_page(pdfviewer);
}


/* on_next */
void on_next(gpointer data)
{
	PDFviewer * pdfviewer = data;

	pdf_update_current(pdfviewer, '+', 1);
	pdf_load_page(pdfviewer);
}


/* on_far_before */
void on_far_before(gpointer data)
{
	PDFviewer * pdfviewer = data;

	/* should be set in preferences */
	pdf_update_current(pdfviewer, '-', 5); 
	pdf_load_page(pdfviewer);
}


/* on_far_after */
void on_far_after(gpointer data)
{
	PDFviewer * pdfviewer = data;

	pdf_update_current(pdfviewer, '+', 5);
	pdf_load_page(pdfviewer);
}


/* on_pdf_close */
void on_pdf_close(gpointer data)
{
	PDFviewer * pdfviewer = data;

	pdf_close(pdfviewer);
}


/* on_zoom_in */
void on_zoom_in(gpointer data)
{
	PDFviewer * pdfviewer = data;

	pdf_update_scale(pdfviewer, '+', 0.1);
}


/* on_zoom_out */
void on_zoom_out(gpointer data)
{
	PDFviewer * pdfviewer = data;

	pdf_update_scale(pdfviewer, '-', 0.1);
}
