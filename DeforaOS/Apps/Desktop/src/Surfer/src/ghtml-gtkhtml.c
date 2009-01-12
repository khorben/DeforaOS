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
#include <libgtkhtml/util/rfc1738.h>
#define GNET_EXPERIMENTAL
#include <gnet.h>
#include "ghtml.h"


/* ghtml */
/* private */
typedef struct _GHtml
{
	/* FIXME implement history */

	/* html widget */
	gchar * html_base;
	HtmlDocument * html_document;
	gchar * html_title;
	GtkWidget * html_view;
} GHtml;


/* prototypes */
static gboolean _ghtml_document_load(GHtml * ghtml, gchar const * base,
		gchar const * url);
/* callbacks */
static void _on_link_clicked(HtmlDocument * document, const gchar * url);
static void _on_request_url(HtmlDocument * document, const gchar * url,
		HtmlStream * stream);
static void _on_set_base(HtmlDocument * document, const gchar * url);
static void _on_title_changed(HtmlDocument * document, const gchar * title);


/* public */
/* functions */
GtkWidget * ghtml_new(Surfer * surfer)
{
	GHtml * ghtml;
	GtkWidget * widget;

	if((ghtml = malloc(sizeof(*ghtml))) == NULL)
		return NULL;
	ghtml->html_base = NULL;
	ghtml->html_view = html_view_new();
	ghtml->html_document = html_document_new();
	ghtml->html_title = NULL;
	g_object_set_data(G_OBJECT(ghtml->html_document), "ghtml", ghtml);
	g_signal_connect(G_OBJECT(ghtml->html_document), "link-clicked",
			G_CALLBACK(_on_link_clicked), NULL);
	g_signal_connect(G_OBJECT(ghtml->html_document), "request-url",
			G_CALLBACK(_on_request_url), NULL);
	g_signal_connect(G_OBJECT(ghtml->html_document), "set-base", G_CALLBACK(
				_on_set_base), NULL);
	g_signal_connect(G_OBJECT(ghtml->html_document), "title-changed",
			G_CALLBACK(_on_title_changed), NULL);
	html_view_set_document(HTML_VIEW(ghtml->html_view),
			ghtml->html_document);
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
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


char const * ghtml_get_title(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	return ghtml->html_title;
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


/* ghtml_load_url */
void ghtml_load_url(GtkWidget * widget, char const * url)
{
	GHtml * ghtml;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, url);
#endif
	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	if(_ghtml_document_load(ghtml, "", url) != TRUE)
		return;
	/* FIXME with current code another base may have been set in between */
	g_free(ghtml->html_base);
	ghtml->html_base = g_strdup(url);
}


/* ghtml_refresh */
void ghtml_refresh(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	if(ghtml->html_base == NULL)
		return;
	/* FIXME should differentiate URL and base */
	_ghtml_document_load(ghtml, "", ghtml->html_base);
}


/* ghtml_reload */
void ghtml_reload(GtkWidget * ghtml)
{
	ghtml_refresh(ghtml);
}


void ghtml_stop(GtkWidget * ghtml)
{
	/* FIXME implement */
}


/* private */
/* functions */
/* ghtml_document_load */
static gboolean _load_write_stream(HtmlStream * stream, gchar const * base,
		gchar const * url);

static gboolean _ghtml_document_load(GHtml * ghtml, gchar const * base,
		gchar const * url)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", \"%s\")\n", __func__, base, url);
#endif
	html_document_open_stream(ghtml->html_document, "text/html");
	if(_load_write_stream(ghtml->html_document->current_stream, base, url)
			!= TRUE)
	{
		html_document_write_stream(ghtml->html_document, "500", 3);
		html_document_close_stream(ghtml->html_document);
		return FALSE; /* FIXME report error */
	}
	html_document_close_stream(ghtml->html_document);
	return TRUE;
}

static gboolean _load_write_stream(HtmlStream * stream, gchar const * base,
		gchar const * url)
{
	gchar * buf = NULL;
	gsize len = 0;
	guint response;

	url = rfc1738_make_full_url(base, url);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() url=\"%s\"\n", __func__, url);
#endif
	if(gnet_http_get(url, &buf, &len, &response) != TRUE)
	{
#ifdef DEBUG
		fprintf(stderr, "DEBUG: gnet_http_get() => %u\n", response);
#endif
		return FALSE; /* FIXME report error */
	}
	html_stream_write(stream, buf, len);
	return TRUE;
}


/* callbacks */
static void _on_link_clicked(HtmlDocument * document, const gchar * url)
{
	GHtml * ghtml;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, url);
#endif
	ghtml = g_object_get_data(G_OBJECT(document), "ghtml");
	_ghtml_document_load(ghtml, ghtml->html_base, url);
}


static void _on_request_url(HtmlDocument * document, const gchar * url,
		HtmlStream * stream)
{
	GHtml * ghtml;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, url);
#endif
	ghtml = g_object_get_data(G_OBJECT(document), "ghtml");
	_load_write_stream(stream, ghtml->html_base, url);
	html_stream_close(stream);
}


static void _on_set_base(HtmlDocument * document, const gchar * url)
{
	GHtml * ghtml;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, url);
#endif
	ghtml = g_object_get_data(G_OBJECT(document), "ghtml");
	g_free(ghtml->html_base);
	ghtml->html_base = g_strdup(url);
}


static void _on_title_changed(HtmlDocument * document, const gchar * title)
{
	GHtml * ghtml;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, title);
#endif
	ghtml = g_object_get_data(G_OBJECT(document), "ghtml");
	g_free(ghtml->html_title);
	ghtml->html_title = g_strdup(title);
	/* FIXME emit signal */
}
