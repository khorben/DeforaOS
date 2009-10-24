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
static WebKitWebView * _on_create_web_view(WebKitWebView * view,
		WebKitWebFrame * frame, gpointer data);
static void _on_hovering_over_link(WebKitWebView * view, const gchar * title,
		const gchar * url, gpointer data);
static void _on_load_committed(WebKitWebView * view, WebKitWebFrame * frame,
		gpointer data);
static void _on_load_finished(WebKitWebView * view, WebKitWebFrame * frame,
		gpointer data);
static void _on_load_progress_changed(WebKitWebView * view, gint progress,
		gpointer data);
static void _on_load_started(WebKitWebView * view, WebKitWebFrame * frame,
		gpointer data);
static void _on_script_alert(WebKitWebView * view, WebKitWebFrame * frame,
		gchar * message, gpointer data);
static gboolean _on_script_confirm(WebKitWebView * view, WebKitWebFrame * frame,
		gchar * message, gboolean confirmed, gpointer data);
static void _on_status_bar_text_changed(WebKitWebView * view, gchar * arg1,
		gpointer data);
static void _on_title_changed(WebKitWebView * view, WebKitWebFrame * frame,
		const gchar * title, gpointer data);
static gboolean _on_web_view_ready(WebKitWebView * view, gpointer data);


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
	g_signal_connect(G_OBJECT(view), "create-web-view", G_CALLBACK(
				_on_create_web_view), widget);
	g_signal_connect(G_OBJECT(view), "hovering-over-link", G_CALLBACK(
				_on_hovering_over_link), widget);
	g_signal_connect(G_OBJECT(view), "load-committed", G_CALLBACK(
				_on_load_committed), widget);
	g_signal_connect(G_OBJECT(view), "load-finished", G_CALLBACK(
				_on_load_finished), widget);
	g_signal_connect(G_OBJECT(view), "load-progress-changed", G_CALLBACK(
				_on_load_progress_changed), widget);
	g_signal_connect(G_OBJECT(view), "load-started", G_CALLBACK(
				_on_load_started), widget);
	g_signal_connect(G_OBJECT(view), "script-alert", G_CALLBACK(
				_on_script_alert), widget);
	g_signal_connect(G_OBJECT(view), "script-confirm", G_CALLBACK(
				_on_script_confirm), widget);
	g_signal_connect(G_OBJECT(view), "status-bar-text-changed", G_CALLBACK(
				_on_status_bar_text_changed), widget);
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
	GtkWidget * view;
	WebKitWebFrame * frame;

	view = g_object_get_data(G_OBJECT(ghtml), "view");
	frame = webkit_web_view_get_main_frame(WEBKIT_WEB_VIEW(view));
	return webkit_web_frame_get_uri(frame);
}


char const * ghtml_get_title(GtkWidget * ghtml)
{
	GtkWidget * view;
	WebKitWebFrame * frame;

	view = g_object_get_data(G_OBJECT(ghtml), "view");
	frame = webkit_web_view_get_main_frame(WEBKIT_WEB_VIEW(view));
	return webkit_web_frame_get_title(frame);
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
	GtkWidget * view;

	view = g_object_get_data(G_OBJECT(ghtml), "view");
	webkit_web_view_reload_bypass_cache(WEBKIT_WEB_VIEW(view));
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
static WebKitWebView * _on_create_web_view(WebKitWebView * view,
		WebKitWebFrame * frame, gpointer data)
{
	WebKitWebView * ret;
	Surfer * surfer;
	Surfer * copy;

	surfer = g_object_get_data(G_OBJECT(data), "surfer");
	if((copy = surfer_new_copy(surfer)) == NULL)
		return NULL;
	/* FIXME many things:
	 * - this is a bit ugly (showing and hiding)
	 * - we may not want history etc to be copied
	 * - it loads the current URL first */
	gtk_widget_hide(copy->window);
	ret = g_object_get_data(G_OBJECT(copy->view), "view");
	g_signal_connect(G_OBJECT(ret), "web-view-ready", G_CALLBACK(
				_on_web_view_ready), copy->view);
	return ret;
}


static void _on_hovering_over_link(WebKitWebView * view, const gchar * title,
		const gchar * url, gpointer data)
{
	Surfer * surfer;

	surfer = g_object_get_data(G_OBJECT(data), "surfer");
	surfer_set_status(surfer, url);
}


static void _on_load_committed(WebKitWebView * view, WebKitWebFrame * frame,
		gpointer data)
{
	Surfer * surfer;
	char const * location;

	surfer = g_object_get_data(G_OBJECT(data), "surfer");
	if(frame == webkit_web_view_get_main_frame(view)
			&& (location = webkit_web_frame_get_uri(frame)) != NULL)
		surfer_set_location(surfer, location);
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


static void _on_script_alert(WebKitWebView * view, WebKitWebFrame * frame,
		gchar * message, gpointer data)
{
	Surfer * surfer;

	surfer = g_object_get_data(G_OBJECT(data), "surfer");
	surfer_warning(surfer, message);
}


static gboolean _on_script_confirm(WebKitWebView * view, WebKitWebFrame * frame,
		gchar * message, gboolean confirmed, gpointer data)
{
	Surfer * surfer;

	surfer = g_object_get_data(G_OBJECT(data), "surfer");
	return surfer_confirm(surfer, message);
}


static void _on_status_bar_text_changed(WebKitWebView * view, gchar * arg1,
		gpointer data)
{
	Surfer * surfer;

	surfer = g_object_get_data(G_OBJECT(data), "surfer");
	surfer_set_status(surfer, arg1);
}


static void _on_title_changed(WebKitWebView * view, WebKitWebFrame * frame,
		const gchar * title, gpointer data)
{
	Surfer * surfer;

	surfer = g_object_get_data(G_OBJECT(data), "surfer");
	surfer_set_title(surfer, title);
}


#if WEBKIT_CHECK_VERSION(1, 0, 3)
static gboolean _on_web_view_ready(WebKitWebView * view, gpointer data)
{
	Surfer * surfer;
	WebKitWebWindowFeatures * features;
	gboolean b;
	gint w;
	gint h;

	surfer = g_object_get_data(G_OBJECT(data), "surfer");
	features = webkit_web_view_get_window_features(WEBKIT_WEB_VIEW(view));
	/* FIXME track properties with notify:: instead */
	g_object_get(G_OBJECT(features), "width", &w, "height", &h, NULL);
	if(w > 0 && h > 0)
		gtk_window_resize(GTK_WINDOW(surfer->window), w, h);
	g_object_get(G_OBJECT(features), "fullscreen", &b, NULL);
	if(b == TRUE)
		surfer_set_fullscreen(surfer, TRUE);
# ifndef EMBEDDED
	g_object_get(G_OBJECT(features), "menubar-visible", &b, NULL);
	if(b == FALSE)
		gtk_widget_hide(surfer->menubar);
# endif
	g_object_get(G_OBJECT(features), "toolbar-visible", &b, NULL);
	if(b == FALSE)
		gtk_widget_hide(surfer->toolbar);
	g_object_get(G_OBJECT(features), "statusbar-visible", &b, NULL);
	if(b == FALSE)
		gtk_widget_hide(surfer->statusbox);
	gtk_widget_show(surfer->window);
	return FALSE;
}
#else /* WebKitWebWindowFeatures is not available */
static gboolean _on_web_view_ready(WebKitWebView * view, gpointer data)
{
	Surfer * surfer;
	gboolean b;
	gint w;
	gint h;

	surfer = g_object_get_data(G_OBJECT(data), "surfer");
	gtk_widget_show(surfer->window);
	return FALSE;
}
#endif
