/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Surfer */
/* Surfer is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * Surfer is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Surfer; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA */



#include <stdlib.h>
#include <libgtkhtml/gtkhtml.h>
#include <gnet.h>
#include "ghtml.h"


/* ghtml */
/* private */
typedef struct _GHtml
{
	GtkWidget * html_view;
	HtmlDocument * html_document;
} GHtml;


/* prototypes */
static gboolean _ghtml_document_load(HtmlDocument * document,
		const gchar * url);
/* callbacks */
static void _on_link_clicked(HtmlDocument * document, const gchar * url);


/* public */
/* functions */
GtkWidget * ghtml_new(Surfer * surfer)
{
	GHtml * ghtml;
	GtkWidget * widget;

	if((ghtml = malloc(sizeof(*ghtml))) == NULL)
		return NULL;
	ghtml->html_view = html_view_new();
	ghtml->html_document = html_document_new();
	g_signal_connect(G_OBJECT(ghtml->html_document), "link-clicked",
			G_CALLBACK(_on_link_clicked), NULL);
	html_view_set_document(ghtml->html_view, ghtml->html_document);
	widget = gtk_scrolled_window_new(NULL, NULL);
	g_object_set_data(G_OBJECT(widget), "ghtml", ghtml);
	gtk_container_add(GTK_CONTAINER(widget), ghtml->html_view);
	return widget;
}


/* accessors */
gboolean ghtml_can_go_back(GtkWidget * ghtml)
{
	/* FIXME implement */
	return FALSE;
}


gboolean ghtml_can_go_forward(GtkWidget * ghtml)
{
	/* FIXME implement */
	return FALSE;
}


/* ghtml_get_link_message */
char const * ghtml_get_link_message(GtkWidget * ghtml)
{
	/* FIXME implement */
	return NULL;
}


char const * ghtml_get_location(GtkWidget * ghtml)
{
	/* FIXME implement */
	return NULL;
}


char const * ghtml_get_title(GtkWidget * ghtml)
{
	/* FIXME implement */
	return NULL;
}


/* useful */
gboolean ghtml_go_back(GtkWidget * ghtml)
{
	/* FIXME implement */
	return FALSE;
}


gboolean ghtml_go_forward(GtkWidget * ghtml)
{
	/* FIXME implement */
	return FALSE;
}


void ghtml_load_url(GtkWidget * widget, char const * url)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	_ghtml_document_load(ghtml->html_document, url);
}

void ghtml_refresh(GtkWidget * ghtml)
{
	/* FIXME implement */
}


void ghtml_reload(GtkWidget * ghtml)
{
	/* FIXME implement */
}


void ghtml_stop(GtkWidget * ghtml)
{
	/* FIXME implement */
}


/* private */
/* functions */
static gboolean _ghtml_document_load(HtmlDocument * document,
		const gchar * url)
{
	gchar * buf = NULL;
	gsize length = 0;
	guint response;

	if(gnet_http_get(url, &buf, &length, &response) != TRUE)
		return FALSE; /* FIXME report error */
	html_document_open_stream(document, "text/html");
	html_document_write_stream(document, buf, length);
	html_document_close_stream(document);
	return TRUE;
}


/* callbacks */
static void _on_link_clicked(HtmlDocument * document, const gchar * url)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, \"%s\")\n", __func__, document, url);
#endif
	_ghtml_document_load(document, url);
}
