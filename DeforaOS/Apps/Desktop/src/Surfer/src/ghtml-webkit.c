/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
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



#include <webkit/webkit.h>
#include "ghtml.h"


/* private */
/* prototypes */
/* functions */
/* callbacks */
static void _on_hovering_over_link(WebKitWebView * view, const gchar * title,
		const gchar * url, gpointer data);
static void _on_load_finished(WebKitWebView * view, WebKitWebFrame * frame,
		gpointer data);
static void _on_load_progress_changed(WebKitWebView * view, gint progress,
		gpointer data);
static void _on_load_started(WebKitWebView * view, WebKitWebFrame * frame,
		gpointer data);
static void _on_title_changed(WebKitWebView * view, WebKitWebFrame * frame,
		const gchar * title, gpointer data);


/* public */
/* functions */
/* ghtml_new */
GtkWidget * ghtml_new(Surfer * surfer)
{
	GtkWidget * view;
	GtkWidget * widget;

	/* widgets */
	view = webkit_web_view_new();
	widget = gtk_scrolled_window_new(NULL, NULL);
	g_object_set_data(G_OBJECT(widget), "surfer", surfer);
	g_object_set_data(G_OBJECT(widget), "view", view);
	/* view */
	g_signal_connect(G_OBJECT(view), "hovering-over-link", G_CALLBACK(
				_on_hovering_over_link), widget);
	g_signal_connect(G_OBJECT(view), "load-finished", G_CALLBACK(
				_on_load_finished), widget);
	g_signal_connect(G_OBJECT(view), "load-progress-changed", G_CALLBACK(
				_on_load_progress_changed), widget);
	g_signal_connect(G_OBJECT(view), "load-started", G_CALLBACK(
				_on_load_started), widget);
	g_signal_connect(G_OBJECT(view), "title-changed", G_CALLBACK(
				_on_title_changed), widget);
	/* scrolled window */
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(widget), view);
	return widget;
}


/* accessors */
/* ghtml_can_go_back */
gboolean ghtml_can_go_back(GtkWidget * ghtml)
{
	GtkWidget * view;

	view = g_object_get_data(G_OBJECT(ghtml), "view");
	return webkit_web_view_can_go_back(WEBKIT_WEB_VIEW(view));
}


gboolean ghtml_can_go_forward(GtkWidget * ghtml)
{
	GtkWidget * view;

	view = g_object_get_data(G_OBJECT(ghtml), "view");
	return webkit_web_view_can_go_forward(WEBKIT_WEB_VIEW(view));
}


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


int ghtml_set_base(GtkWidget * ghtml, char const * url)
{
	/* FIXME implement */
	return 1;
}


/* useful */
gboolean ghtml_go_back(GtkWidget * ghtml)
{
	GtkWidget * view;

	if(ghtml_can_go_back(ghtml) == FALSE)
		return FALSE;
	view = g_object_get_data(G_OBJECT(ghtml), "view");
	webkit_web_view_go_back(WEBKIT_WEB_VIEW(view));
	return TRUE;
}


gboolean ghtml_go_forward(GtkWidget * ghtml)
{
	GtkWidget * view;

	if(ghtml_can_go_forward(ghtml) == FALSE)
		return FALSE;
	view = g_object_get_data(G_OBJECT(ghtml), "view");
	webkit_web_view_go_forward(WEBKIT_WEB_VIEW(view));
	return TRUE;
}


void ghtml_load_url(GtkWidget * ghtml, char const * url)
{
	GtkWidget * view;

	view = g_object_get_data(G_OBJECT(ghtml), "view");
	webkit_web_view_open(WEBKIT_WEB_VIEW(view), url);
}


void ghtml_refresh(GtkWidget * ghtml)
{
	GtkWidget * view;

	view = g_object_get_data(G_OBJECT(ghtml), "view");
	webkit_web_view_reload(WEBKIT_WEB_VIEW(view));
}


void ghtml_reload(GtkWidget * ghtml)
{
	return ghtml_refresh(ghtml);
}


void ghtml_stop(GtkWidget * ghtml)
{
	GtkWidget * view;

	view = g_object_get_data(G_OBJECT(ghtml), "view");
	webkit_web_view_stop_loading(WEBKIT_WEB_VIEW(view));
}


void ghtml_select_all(GtkWidget * ghtml)
{
	GtkWidget * view;

	view = g_object_get_data(G_OBJECT(ghtml), "view");
	webkit_web_view_select_all(WEBKIT_WEB_VIEW(view));
}


void ghtml_unselect_all(GtkWidget * ghtml)
{
	/* FIXME implement */
}


void ghtml_zoom_in(GtkWidget * ghtml)
{
	GtkWidget * view;

	view = g_object_get_data(G_OBJECT(ghtml), "view");
	webkit_web_view_zoom_in(WEBKIT_WEB_VIEW(view));
}


void ghtml_zoom_out(GtkWidget * ghtml)
{
	GtkWidget * view;

	view = g_object_get_data(G_OBJECT(ghtml), "view");
	webkit_web_view_zoom_out(WEBKIT_WEB_VIEW(view));
}


void ghtml_zoom_reset(GtkWidget * ghtml)
{
	GtkWidget * view;

	view = g_object_get_data(G_OBJECT(ghtml), "view");
	webkit_web_view_set_zoom_level(WEBKIT_WEB_VIEW(view), 1.0);
}


/* private */
/* functions */
static void _on_hovering_over_link(WebKitWebView * view, const gchar * title,
		const gchar * url, gpointer data)
{
	Surfer * surfer;

	surfer = g_object_get_data(G_OBJECT(data), "surfer");
	surfer_set_status(surfer, url);
}


static void _on_load_finished(WebKitWebView * view, WebKitWebFrame * arg1,
			gpointer data)
{
	Surfer * surfer;

	surfer = g_object_get_data(G_OBJECT(data), "surfer");
	surfer_set_progress(surfer, -1.0);
	surfer_set_status(surfer, NULL);
}


static void _on_load_progress_changed(WebKitWebView * view, gint progress,
		gpointer data)
{
	Surfer * surfer;
	gdouble fraction = progress;

	surfer = g_object_get_data(G_OBJECT(data), "surfer");
	surfer_set_progress(surfer, fraction / 100);
}


static void _on_load_started(WebKitWebView * view, WebKitWebFrame * frame,
		gpointer data)
{
	Surfer * surfer;

	surfer = g_object_get_data(G_OBJECT(data), "surfer");
	surfer_set_progress(surfer, 0.0);
	surfer_set_status(surfer, "Downloading...");
}


static void _on_title_changed(WebKitWebView * view, WebKitWebFrame * frame,
		const gchar * title, gpointer data)
{
	Surfer * surfer;

	surfer = g_object_get_data(G_OBJECT(data), "surfer");
	surfer_set_title(surfer, title);
}
