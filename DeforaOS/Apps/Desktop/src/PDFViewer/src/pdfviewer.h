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



#ifndef PDFVIEWER_PDFVIEWER_H
# define PDFVIEWER_PDFVIEWER_H

# include <gtk/gtk.h>
# include <poppler.h>


/* Editor */
/* types */

typedef struct _PDF PDF;
typedef struct _PDFviewer PDFviewer;


/* functions */
PDFviewer * pdfviewer_new(void);
void pdfviewer_delete(PDFviewer * pdfviewer);

/* useful */
int pdf_open(PDFviewer * pdfviewer, const char * filename);
void pdf_close(PDFviewer * pdfviewer);

void pdf_update_scale(PDFviewer * pdfviewer, const char op, double n);
void pdf_load_page(PDFviewer * pdfviewer);
void pdf_update_current(PDFviewer * pdfviewer, const char op, int n);
void set_prevnext_sensitivity(PDFviewer * pdfviewer);

void pdfviewer_about(PDFviewer * pdfviewer);
int pdfviewer_error(PDFviewer * pdfviewer, char const * message, int ret);

gboolean pdfviewer_close(PDFviewer * pdfviewer);
void pdfviewer_open(PDFviewer * pdfviewer, char const * filename);
void pdfviewer_open_dialog(PDFviewer * pdfviewer);

/* FIXME noy implemented */
void pdfviewer_find(PDFviewer * pdfviewer, char const * text);

#endif /* !PDFVIEWER_PDFVIEWER_H */
