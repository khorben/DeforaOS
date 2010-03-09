/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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
#include "common.h" /* XXX should not be needed */
#include "common/url.c"


/* private */
/* prototypes */
/* functions */
/* callbacks */
static WebKitWebView * _on_create_web_view(WebKitWebView * view,
		WebKitWebFrame * frame, gpointer data);
static gboolean _on_download_requested(WebKitWebView * view,
		WebKitDownload * download, gpointer data);
static void _on_hovering_over_link(WebKitWebView * view, const gchar * title,
		const gchar * url, gpointer data);
static void _on_load_committed(WebKitWebView * view, WebKitWebFrame * frame,
		gpointer data);
static gboolean _on_load_error(WebKitWebView * view, WebKitWebFrame * frame,
		const gchar * uri, GError * error, gpointer data);
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
	g_signal_connect(G_OBJECT(view), "download-requested", G_CALLBACK(
				_on_download_requested), widget);
	g_signal_connect(G_OBJECT(view), "hovering-over-link", G_CALLBACK(
				_on_hovering_over_link), widget);
	g_signal_connect(G_OBJECT(view), "load-committed", G_CALLBACK(
				_on_load_committed), widget);
	g_signal_connect(G_OBJECT(view), "load-error", G_CALLBACK(
				_on_load_error), widget);
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


/* ghtml_get_source */
char const * ghtml_get_source(GtkWidget * ghtml)
{
	GtkWidget * view;
	WebKitWebFrame * frame;
	WebKitWebDataSource * source;
	GString * str;

	view = g_object_get_data(G_OBJECT(ghtml), "view");
	frame = webkit_web_view_get_main_frame(WEBKIT_WEB_VIEW(view));
	source = webkit_web_frame_get_data_source(frame);
	if((str = webkit_web_data_source_get_data(source)) == NULL)
		return NULL;
	return str->str;
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
	gchar * p;

	if((p = _ghtml_make_url(NULL, url)) != NULL)
		url = p;
	view = g_object_get_data(G_OBJECT(ghtml), "view");
	webkit_web_view_open(WEBKIT_WEB_VIEW(view), url);
	g_free(p);
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
#if WEBKIT_CHECK_VERSION(1, 0, 3)
	webkit_web_view_reload_bypass_cache(WEBKIT_WEB_VIEW(view));
#else
	webkit_web_view_reload(WEBKIT_WEB_VIEW(view));
#endif
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
/* on_create_web_view */
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
	surfer_show_window(copy, FALSE);
	ret = g_object_get_data(G_OBJECT(copy->view), "view");
	g_signal_connect(G_OBJECT(ret), "web-view-ready", G_CALLBACK(
				_on_web_view_ready), copy->view);
	return ret;
}


/* on_download_requested */
static gboolean _on_download_requested(WebKitWebView * view,
		WebKitDownload * download, gpointer data)
{
	Surfer * surfer;
	char const * url;
	char const * suggested;

	surfer = g_object_get_data(G_OBJECT(data), "surfer");
	url = webkit_download_get_uri(download);
	suggested = webkit_download_get_suggested_filename(download);
	surfer_download(surfer, url, suggested);
	webkit_download_cancel(download);
	return FALSE;
}


/* on_hovering_over_link */
static void _on_hovering_over_link(WebKitWebView * view, const gchar * title,
		const gchar * url, gpointer data)
{
	Surfer * surfer;

	surfer = g_object_get_data(G_OBJECT(data), "surfer");
	surfer_set_status(surfer, url);
}


/* on_load_committed */
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


/* on_load_error */
static gboolean _on_load_error(WebKitWebView * view, WebKitWebFrame * frame,
		const gchar * uri, GError * error, gpointer data)
{
	Surfer * surfer;

	surfer = g_object_get_data(G_OBJECT(data), "surfer");
	if(error != NULL && error->message != NULL)
		surfer_error(surfer, error->message, 0);
	return TRUE;
}


/* on_load_finished */
static void _on_load_finished(WebKitWebView * view, WebKitWebFrame * arg1,
			gpointer data)
{
	Surfer * surfer;

	surfer = g_object_get_data(G_OBJECT(data), "surfer");
	surfer_set_progress(surfer, -1.0);
	surfer_set_status(surfer, NULL);
}


/* on_load_progress_changed */
static void _on_load_progress_changed(WebKitWebView * view, gint progress,
		gpointer data)
{
	Surfer * surfer;
	gdouble fraction = progress;

	surfer = g_object_get_data(G_OBJECT(data), "surfer");
	surfer_set_progress(surfer, fraction / 100);
}


/* on_load_started */
static void _on_load_started(WebKitWebView * view, WebKitWebFrame * frame,
		gpointer data)
{
	Surfer * surfer;

	surfer = g_object_get_data(G_OBJECT(data), "surfer");
	surfer_set_progress(surfer, 0.0);
	surfer_set_status(surfer, "Downloading...");
}


/* on_script_alert */
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
		surfer_resize(surfer, w, h);
	g_object_get(G_OBJECT(features), "fullscreen", &b, NULL);
	if(b == TRUE)
		surfer_set_fullscreen(surfer, TRUE);
# ifndef EMBEDDED
	g_object_get(G_OBJECT(features), "menubar-visible", &b, NULL);
	surfer_show_menubar(surfer, b);
# endif
	g_object_get(G_OBJECT(features), "toolbar-visible", &b, NULL);
	surfer_show_toolbar(surfer, b);
	g_object_get(G_OBJECT(features), "statusbar-visible", &b, NULL);
	surfer_show_statusbar(surfer, b);
	surfer_show_window(surfer, TRUE);
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
