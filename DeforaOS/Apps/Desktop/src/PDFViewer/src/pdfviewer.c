/* $Id$ */
static char const _copyright[] =
"Copyright (c) 2010 Sébastien Bocahu <zecrazytux@zecrazytux.net>";
static char const _license[] =
"This program is free software; you can redistribute it and/or modify\n"
"it under the terms of the GNU General Public License as published by the\n"
"Free Software Foundation, version 3 of the License.\n"
"\n"
"This program is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"GNU General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU General Public License\n"
"along with this program. If not, see <http://www.gnu.org/licenses/>.\n";



#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <gdk/gdkkeysyms.h>
#include <poppler.h>
#include <Desktop.h>
#include "callbacks.h"
#include "pdfviewer.h"
#include "../config.h"


/* PDFviewer */
/* private */
/* types */
struct _PDFviewer
{
	PDF * pdf;

	/* widgets */
	GtkWidget * window;
	GtkWidget * view;
	GtkWidget * statusbar;
	GtkWidget * toolbar;
	/* about */
	GtkWidget * ab_window;
};

struct _PDF
{
	PopplerDocument *document;

	int pages;
	int current;

	GtkWidget       *area;
	cairo_surface_t *surface;
	double scale;
};



/* variables */
static char const * _authors[] =
{
	"Sébastien Bocahu <zecrazytux@zecrazytux.net>",
	NULL
};

#ifdef EMBEDDED
static DesktopAccel _pdfviewer_accel[] =
{
	{ G_CALLBACK(on_pdf_close), GDK_CONTROL_MASK, GDK_w },
	{ G_CALLBACK(on_open), GDK_CONTROL_MASK, GDK_o },
	{ G_CALLBACK(on_previous), GDK_CONTROL_MASK, GDK_p },
	{ G_CALLBACK(on_next), GDK_CONTROL_MASK, GDK_n },
	{ NULL, 0, 0 }
};
#endif

#ifndef EMBEDDED
static DesktopMenu _pdfviewer_menu_file[] =
{
	{ "_Open", G_CALLBACK(on_file_open), GTK_STOCK_OPEN, GDK_CONTROL_MASK,
		GDK_o },
	{ "", NULL, NULL, 0, 0 },
	{ "_Close", G_CALLBACK(on_file_close), GTK_STOCK_CLOSE,
		GDK_CONTROL_MASK, GDK_w },
	{ NULL, NULL, NULL, 0, 0 }
};

static DesktopMenu _pdfviewer_menu_edit[] =
{
	{ NULL, NULL, NULL, 0, 0 }
};

static DesktopMenu _pdfviewer_menu_view[] =
{
	{ NULL, NULL, NULL, 0, 0 }
};

static DesktopMenu _pdfviewer_menu_help[] =
{
	{ "_About", G_CALLBACK(on_help_about),
#if GTK_CHECK_VERSION(2, 6, 0)
		GTK_STOCK_ABOUT, 0, 0 },
#else
		NULL, 0, 0 },
#endif
	{ NULL, NULL, NULL, 0, 0 }
};

static DesktopMenubar _pdfviewer_menubar[] =
{
	{ "_File", _pdfviewer_menu_file },
	{ "_Edit", _pdfviewer_menu_edit },
	{ "_View", _pdfviewer_menu_view },
	{ "_Help", _pdfviewer_menu_help },
	{ NULL, NULL }
};
#endif

static DesktopToolbar _pdfviewer_toolbar[] =
{
	{ "Open", G_CALLBACK(on_open), GTK_STOCK_OPEN, 0, 0, NULL },
	{ "FarBefore", G_CALLBACK(on_far_before), GTK_STOCK_MEDIA_PREVIOUS,
		 0, 0, NULL },
	{ "Previous", G_CALLBACK(on_previous), GTK_STOCK_GO_BACK,
		 0, 0, NULL },
	{ "Next", G_CALLBACK(on_next), GTK_STOCK_GO_FORWARD, 0, 0, NULL },
	{ "FarAfter", G_CALLBACK(on_far_after), GTK_STOCK_MEDIA_NEXT, 0, 0, NULL },
	{ "ZoomIn", G_CALLBACK(on_zoom_in), GTK_STOCK_ZOOM_IN, 0, 0, NULL },
	{ "ZoomOut", G_CALLBACK(on_zoom_out), GTK_STOCK_ZOOM_OUT, 0, 0, NULL },
	{ NULL, NULL, NULL, 0, 0, NULL }
};


/* public */
/* functions */
/* pdfviewer_new */
static void _new_set_title(PDFviewer * pdfviewer);

PDFviewer * pdfviewer_new(void)
{
	PDFviewer * pdfviewer;
	GtkAccelGroup * group;
	GtkSettings * settings;
	GtkWidget * vbox;
	GtkWidget * widget;

	if((pdfviewer = malloc(sizeof(*pdfviewer))) == NULL)
		return NULL;
	settings = gtk_settings_get_default();
	pdfviewer->pdf = NULL;
	/* widgets */
	group = gtk_accel_group_new();
	pdfviewer->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_add_accel_group(GTK_WINDOW(pdfviewer->window), group);
	gtk_window_set_default_size(GTK_WINDOW(pdfviewer->window), 600, 400);
	_new_set_title(pdfviewer);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(pdfviewer->window),
		"gnome-mime-application-pdf");
#endif
	g_signal_connect_swapped(G_OBJECT(pdfviewer->window), "delete-event",
			G_CALLBACK(on_closex), pdfviewer);
	vbox = gtk_vbox_new(FALSE, 0);
	/* menubar */
#ifndef EMBEDDED
	widget = desktop_menubar_create(_pdfviewer_menubar, pdfviewer, group);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, FALSE, 0);
#else
	desktop_accel_create(_pdfviewer_accel, pdfviewer, group);
#endif
	/* toolbar */
	pdfviewer->toolbar = desktop_toolbar_create(_pdfviewer_toolbar,
		pdfviewer, group);
	set_prevnext_sensitivity(pdfviewer);
	gtk_box_pack_start(GTK_BOX(vbox), pdfviewer->toolbar, FALSE, FALSE, 0);
	/* view */
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	
	pdfviewer->view = gtk_drawing_area_new();
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(widget),
		pdfviewer->view);
	gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);

	/* statusbar */
	pdfviewer->statusbar = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(vbox), pdfviewer->statusbar,
		FALSE, FALSE, 0);

	/* about */
	pdfviewer->ab_window = NULL;
	gtk_container_add(GTK_CONTAINER(pdfviewer->window), vbox);
	gtk_widget_show_all(pdfviewer->window);
	return pdfviewer;
}

static void _new_set_title(PDFviewer * pdfviewer)
{
	char buf[256];

	snprintf(buf, sizeof(buf), "%s%s", "PDF viewer - ",
			(pdfviewer->pdf == NULL) ? "(Untitled)"
			: "FIXME"); /* FIXME */
	gtk_window_set_title(GTK_WINDOW(pdfviewer->window), buf);
}


/* pdfviewer_delete */
void pdfviewer_delete(PDFviewer * pdfviewer)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
#if 0 /* FIXME */
	if(pdfviewer->pdf != NULL)
		pdf_delete(pdfviewer->pdf);
#endif
	free(pdfviewer);
}

/* useful */
/* pdfviewer_about */
static gboolean _about_on_closex(GtkWidget * widget);

void pdfviewer_about(PDFviewer * pdfviewer)
{
	if(pdfviewer->ab_window != NULL)
	{
		gtk_widget_show(pdfviewer->ab_window);
		return;
	}
	pdfviewer->ab_window = desktop_about_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(pdfviewer->ab_window),
			GTK_WINDOW(pdfviewer->window));
	g_signal_connect(G_OBJECT(pdfviewer->ab_window), "delete-event",
			G_CALLBACK(_about_on_closex), NULL);
	desktop_about_dialog_set_authors(pdfviewer->ab_window, _authors);
	desktop_about_dialog_set_copyright(pdfviewer->ab_window, _copyright);
	desktop_about_dialog_set_license(pdfviewer->ab_window, _license);
	desktop_about_dialog_set_logo_icon_name(pdfviewer->ab_window,
			"gnome-mime-application-pdf");
	desktop_about_dialog_set_name(pdfviewer->ab_window, PACKAGE);
	desktop_about_dialog_set_version(pdfviewer->ab_window, VERSION);
	gtk_widget_show(pdfviewer->ab_window);
}

static gboolean _about_on_closex(GtkWidget * widget)
{
	gtk_widget_hide(widget);
	return TRUE;
}


/* pdfviewer_error */
int pdfviewer_error(PDFviewer * pdfviewer, char const * message, int ret)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(pdfviewer->window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s",
#if GTK_CHECK_VERSION(2, 6, 0)
			"Error");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s",
#endif
			message);
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(
				gtk_widget_destroy), NULL);
	gtk_widget_show(dialog);
	return ret;
}


/* pdfviewer_close */
gboolean pdfviewer_close(PDFviewer * pdfviewer)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	/* FIXME implement */
	gtk_main_quit();
	return FALSE;
}


/* pdfviewer_open */
void pdfviewer_open(PDFviewer * pdfviewer, char const * uri)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	/* FIXME handle errors */
	if(uri == NULL)
		return;
	if(pdfviewer->pdf != NULL)
		pdf_close(pdfviewer);
	if(pdf_open(pdfviewer, uri) != 0)
		return;
	_new_set_title(pdfviewer); /* XXX make it a generic private function */
}


/* pdfviewer_open_dialog */
void pdfviewer_open_dialog(PDFviewer * pdfviewer)
{
	GtkWidget * dialog;
	char * uri = NULL;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif

	dialog = gtk_file_chooser_dialog_new("Open file...",
			GTK_WINDOW(pdfviewer->window),
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(
					dialog));
	gtk_widget_destroy(dialog);
	if(uri == NULL)
		return;
	pdfviewer_open(pdfviewer, uri);
	g_free(uri);
}

/* pdf_open */
int pdf_open(PDFviewer * pdfviewer, const char * uri)
{
	GError * err = NULL;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif

	pdfviewer->pdf = g_new0(PDF, 1);
	pdfviewer->pdf->document = 
		poppler_document_new_from_file(uri, NULL, &err);

	if(err != NULL) {
		fprintf(stderr, "error: %s", err->message);
		g_error_free (err);
		return 1;
	}

	pdfviewer->pdf->pages = 
		poppler_document_get_n_pages(pdfviewer->pdf->document);

	pdf_update_current(pdfviewer, '=', 0);


/*	pdfviewer->pdf->scale = 1.0;
*/	
	pdf_load_page(pdfviewer);

	return 0;
}

/* pdf_close */
void pdf_close(PDFviewer * pdfviewer)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif

	gdk_window_clear(gtk_widget_get_window(pdfviewer->view));
	if(pdfviewer->pdf != NULL)
		free(pdfviewer->pdf);
}

/* pdf_load_page */
void pdf_load_page(PDFviewer * pdfviewer)
{
	PopplerPage *page;
	cairo_t *cr;
	gdouble      width, height;
	GtkAllocation view_allocation;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif

	page = poppler_document_get_page(pdfviewer->pdf->document,
		pdfviewer->pdf->current);
	poppler_page_get_size(page, &width, &height);

	if(!pdfviewer->pdf->scale) {
		/* gdk_drawable_get_size(gtk_widget_get_window(pdfviewer->view), &w, &h); */
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() scale not set!\n", __func__);
#endif
		gtk_widget_get_allocation(pdfviewer->view, &view_allocation);
		pdfviewer->pdf->scale = ((view_allocation.width - 20) / width); 
#if 0
		pdfviewer->pdf->scale = (view_allocation.height / height); /* view whole page */
#endif
	}

	if (!page)
		return;

	gtk_statusbar_push(GTK_STATUSBAR(pdfviewer->statusbar),
		gtk_statusbar_get_context_id(
			GTK_STATUSBAR(pdfviewer->statusbar), "read-page"),
		g_strdup_printf("Page %d/%d",
			pdfviewer->pdf->current + 1, pdfviewer->pdf->pages));

	if (pdfviewer->pdf->surface)
		cairo_surface_destroy (pdfviewer->pdf->surface);
	pdfviewer->pdf->surface = NULL;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() scale: %f\n", __func__,
			pdfviewer->pdf->scale);
#endif
	pdfviewer->pdf->surface = cairo_image_surface_create(
		CAIRO_FORMAT_ARGB32, ceil(pdfviewer->pdf->scale * width),
		ceil(pdfviewer->pdf->scale * height));
	cr = cairo_create(pdfviewer->pdf->surface);
	cairo_save(cr);
	cairo_scale(cr, pdfviewer->pdf->scale, pdfviewer->pdf->scale);
	poppler_page_render(page, cr);
	cairo_restore(cr);
	cairo_destroy(cr);
	g_object_unref(page);

	g_signal_connect(G_OBJECT(pdfviewer->view), "expose-event",
		G_CALLBACK(pdf_render_area),
		(gpointer) pdfviewer->pdf);

	gtk_widget_set_size_request(pdfviewer->view,
		pdfviewer->pdf->scale * ceil(width),
		pdfviewer->pdf->scale * ceil(height));
	gtk_widget_queue_draw(pdfviewer->view);
}

/* pdf_render_area */
void pdf_render_area(GtkWidget *area, GdkEventExpose *event, void * data)
{
	PDF * pdf = data;
        cairo_t *cr;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif

        gdk_window_clear(gtk_widget_get_window(area));

	if(pdf == NULL)
		return;

        cr = gdk_cairo_create(gtk_widget_get_window(area));
        cairo_set_source_surface(cr, pdf->surface, 0, 0);
        cairo_paint(cr);
        cairo_destroy(cr);
}

/* pdf_update_current */
void pdf_update_current(PDFviewer * pdfviewer, const char op, int n)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	switch(op) {
		case '=':	if((n >= 0) && 
				  (n <= (pdfviewer->pdf->pages - 1)))
					pdfviewer->pdf->current = n;
					break;
		case '+':	if((pdfviewer->pdf->current + n)
				   <= pdfviewer->pdf->pages)
					pdfviewer->pdf->current =
						pdfviewer->pdf->current + n;
					break;
		case '-':	if((pdfviewer->pdf->current - n) >= 0)	
					pdfviewer->pdf->current =
						pdfviewer->pdf->current - n;
					break;
	}
	set_prevnext_sensitivity(pdfviewer);
}

/* set_prevnext_sensitivity */
void set_prevnext_sensitivity(PDFviewer * pdfviewer)
{
	gboolean farbefore, prev, next, farafter;

	if(pdfviewer->pdf) { /* XXX s/5/preferences/ */
		farbefore = (pdfviewer->pdf->current > 5) ? TRUE : FALSE;
		prev = (pdfviewer->pdf->current > 0) ? TRUE : FALSE;
		next = (pdfviewer->pdf->current+1 < pdfviewer->pdf->pages) ?
			TRUE : FALSE;
		farafter = (pdfviewer->pdf->current+5 < pdfviewer->pdf->pages) ?
			TRUE : FALSE;
	} else {
		farbefore = FALSE;
		prev = FALSE;
		next = FALSE;
		farafter = FALSE;
	}
	gtk_widget_set_sensitive(GTK_WIDGET(
		gtk_toolbar_get_nth_item(GTK_TOOLBAR(pdfviewer->toolbar), 1)
		), farbefore);
	gtk_widget_set_sensitive(GTK_WIDGET(
		gtk_toolbar_get_nth_item(GTK_TOOLBAR(pdfviewer->toolbar), 2)
		), prev);
	gtk_widget_set_sensitive(GTK_WIDGET(
		gtk_toolbar_get_nth_item(GTK_TOOLBAR(pdfviewer->toolbar), 3)
		), next);
	gtk_widget_set_sensitive(GTK_WIDGET(
		gtk_toolbar_get_nth_item(GTK_TOOLBAR(pdfviewer->toolbar), 4)
		), farafter);
}

/* pdf_update_scale */
void pdf_update_scale(PDFviewer * pdfviewer, const char op, double n)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	switch(op) {
		case '=':	pdfviewer->pdf->scale = n;
				break;
		case '+':	pdfviewer->pdf->scale =
						pdfviewer->pdf->scale + n;
				break;
		case '-':	if((pdfviewer->pdf->scale - n) > 0)	
					pdfviewer->pdf->scale =
						pdfviewer->pdf->scale - n;
				break;
	}
	pdf_load_page(pdfviewer);
}
