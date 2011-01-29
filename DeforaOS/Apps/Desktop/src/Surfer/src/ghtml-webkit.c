/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Surfer */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <libintl.h>
#include <webkit/webkit.h>
#include "ghtml.h"
#include "common/url.c"
#define _(string) gettext(string)


/* private */
/* types */
typedef struct _GHtml
{
	Surfer * surfer;
	GtkWidget * widget;
	GtkWidget * view;
	char * status;
	gboolean ssl;
} GHtml;


/* prototypes */
/* functions */
static void _ghtml_set_status(GtkWidget * widget, char const * status);

/* callbacks */
static gboolean _on_console_message(WebKitWebView * view, const gchar * message,
		guint line, const gchar * source, gpointer data);
static WebKitWebView * _on_create_web_view(WebKitWebView * view,
		WebKitWebFrame * frame, gpointer data);
#ifdef WEBKIT_TYPE_DOWNLOAD
static gboolean _on_download_requested(WebKitWebView * view,
		WebKitDownload * download, gpointer data);
#endif
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
static gboolean _on_script_alert(WebKitWebView * view, WebKitWebFrame * frame,
		const gchar * message, gpointer data);
static gboolean _on_script_confirm(WebKitWebView * view, WebKitWebFrame * frame,
		const gchar * message, gboolean * confirmed, gpointer data);
static gboolean _on_script_prompt(WebKitWebView * view, WebKitWebFrame * frame,
		const gchar * message, const gchar * default_value,
		gchar ** value, gpointer data);
static void _on_status_bar_text_changed(WebKitWebView * view, gchar * arg1,
		gpointer data);
static void _on_title_changed(WebKitWebView * view, WebKitWebFrame * frame,
		const gchar * title, gpointer data);
static gboolean _on_web_view_ready(WebKitWebView * view, gpointer data);


/* public */
/* functions */
/* ghtml_new */
static void _new_init(GHtml * ghtml);

GtkWidget * ghtml_new(Surfer * surfer)
{
	GHtml * ghtml;
	GtkWidget * widget;

	if((ghtml = object_new(sizeof(*ghtml))) == NULL)
		return NULL;
	ghtml->surfer = surfer;
	ghtml->status = NULL;
	ghtml->ssl = FALSE;
	/* widgets */
	widget = gtk_scrolled_window_new(NULL, NULL);
	ghtml->widget = widget;
	ghtml->view = webkit_web_view_new();
	g_object_set_data(G_OBJECT(widget), "ghtml", ghtml);
	/* view */
	g_signal_connect(G_OBJECT(ghtml->view), "console-message", G_CALLBACK(
				_on_console_message), widget);
	g_signal_connect(G_OBJECT(ghtml->view), "create-web-view", G_CALLBACK(
				_on_create_web_view), widget);
#ifdef WEBKIT_TYPE_DOWNLOAD
	g_signal_connect(G_OBJECT(ghtml->view), "download-requested",
			G_CALLBACK(_on_download_requested), widget);
#endif
	g_signal_connect(G_OBJECT(ghtml->view), "hovering-over-link",
			G_CALLBACK(_on_hovering_over_link), widget);
	g_signal_connect(G_OBJECT(ghtml->view), "load-committed", G_CALLBACK(
				_on_load_committed), widget);
	g_signal_connect(G_OBJECT(ghtml->view), "load-error", G_CALLBACK(
				_on_load_error), widget);
	g_signal_connect(G_OBJECT(ghtml->view), "load-finished", G_CALLBACK(
				_on_load_finished), widget);
	g_signal_connect(G_OBJECT(ghtml->view), "load-progress-changed",
			G_CALLBACK(_on_load_progress_changed), widget);
	g_signal_connect(G_OBJECT(ghtml->view), "load-started", G_CALLBACK(
				_on_load_started), widget);
	g_signal_connect(G_OBJECT(ghtml->view), "script-alert", G_CALLBACK(
				_on_script_alert), widget);
	g_signal_connect(G_OBJECT(ghtml->view), "script-confirm", G_CALLBACK(
				_on_script_confirm), widget);
	g_signal_connect(G_OBJECT(ghtml->view), "script-prompt", G_CALLBACK(
				_on_script_prompt), widget);
	g_signal_connect(G_OBJECT(ghtml->view), "status-bar-text-changed",
			G_CALLBACK(_on_status_bar_text_changed), widget);
	g_signal_connect(G_OBJECT(ghtml->view), "title-changed", G_CALLBACK(
				_on_title_changed), widget);
	/* scrolled window */
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(widget), ghtml->view);
	_new_init(ghtml);
	return widget;
}

static void _new_init(GHtml * ghtml)
{
	static int initialized = 0;
#if WEBKIT_CHECK_VERSION(1, 1, 0)
	SoupSession * session;
	char const * cacerts[] =
	{
		"/etc/pki/tls/certs/ca-bundle.crt",
		"/etc/ssl/certs/ca-certificates.crt",
		"/etc/openssl/certs/ca-certificates.crt"
	};
	size_t i;
#endif

	if(initialized++ == 1)
	{
		ghtml->ssl = TRUE;
		initialized = 1;
		return;
	}
	else if(initialized != 1)
		return;
#if WEBKIT_CHECK_VERSION(1, 1, 0)
	session = webkit_get_default_session();
	for(i = 0; i < sizeof(cacerts) / sizeof(*cacerts); i++)
		if(access(cacerts[i], R_OK) == 0)
		{
			g_object_set(session, "ssl-ca-file", cacerts[i],
					"ssl-strict", FALSE, NULL);
			ghtml->ssl = TRUE;
			return;
		}
#endif
	surfer_warning(ghtml->surfer, "Could not load certificate bundle:\n"
			"SSL certificates will not be verified.");
	initialized++;
}


/* ghtml_delete */
void ghtml_delete(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	free(ghtml->status);
	object_delete(ghtml);
}


/* accessors */
/* ghtml_can_go_back */
gboolean ghtml_can_go_back(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	return webkit_web_view_can_go_back(WEBKIT_WEB_VIEW(ghtml->view));
}


gboolean ghtml_can_go_forward(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	return webkit_web_view_can_go_forward(WEBKIT_WEB_VIEW(ghtml->view));
}


char const * ghtml_get_link_message(GtkWidget * widget)
{
	/* FIXME implement */
	return NULL;
}


/* ghtml_get_location */
char const * ghtml_get_location(GtkWidget * widget)
{
	GHtml * ghtml;
	WebKitWebFrame * frame;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	frame = webkit_web_view_get_main_frame(WEBKIT_WEB_VIEW(ghtml->view));
	return webkit_web_frame_get_uri(frame);
}


/* ghtml_get_progress */
gdouble ghtml_get_progress(GtkWidget * widget)
{
	gdouble ret = -1.0;
#if WEBKIT_CHECK_VERSION(1, 1, 0) /* XXX may not be accurate */
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	ret = webkit_web_view_get_progress(WEBKIT_WEB_VIEW(ghtml->view));
	if(ret == 0.0)
		ret = -1.0;
#endif
	return ret;
}


/* ghtml_get_security */
SurferSecurity ghtml_get_security(GtkWidget * widget)
{
	SurferSecurity security = SS_NONE;
#if WEBKIT_CHECK_VERSION(1, 1, 0)
	GHtml * ghtml;
	WebKitWebFrame * frame;
	char const * location;
	WebKitWebDataSource *source;
	WebKitNetworkRequest *request;
	SoupMessage * message;
#endif

#if WEBKIT_CHECK_VERSION(1, 1, 0)
	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	frame = webkit_web_view_get_main_frame(WEBKIT_WEB_VIEW(ghtml->view));
	if((location = webkit_web_frame_get_uri(frame)) != NULL
			&& strncmp(location, "https://", 8) == 0)
	{
		security = SS_UNTRUSTED;
		source = webkit_web_frame_get_data_source(frame);
		request = webkit_web_data_source_get_request(source);
		message = webkit_network_request_get_message(request);
		if(ghtml->ssl == TRUE && message != NULL
				&& soup_message_get_flags(message)
				& SOUP_MESSAGE_CERTIFICATE_TRUSTED)
			security = SS_TRUSTED;
	}
#endif
	return security;
}


/* ghtml_get_source */
char const * ghtml_get_source(GtkWidget * widget)
{
#if WEBKIT_CHECK_VERSION(1, 1, 0)
	GHtml * ghtml;
	WebKitWebFrame * frame;
	WebKitWebDataSource * source;
	GString * str;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	frame = webkit_web_view_get_main_frame(WEBKIT_WEB_VIEW(ghtml->view));
	source = webkit_web_frame_get_data_source(frame);
	if((str = webkit_web_data_source_get_data(source)) == NULL)
		return NULL;
	return str->str;
#else
	return NULL;
#endif
}


/* ghtml_get_status */
char const * ghtml_get_status(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	return ghtml->status;
}


/* ghtml_get_title */
char const * ghtml_get_title(GtkWidget * widget)
{
	GHtml * ghtml;
	WebKitWebFrame * frame;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	frame = webkit_web_view_get_main_frame(WEBKIT_WEB_VIEW(ghtml->view));
	return webkit_web_frame_get_title(frame);
}


/* ghtml_set_proxy */
int ghtml_set_proxy(GtkWidget * widget, SurferProxyType type, char const * http,
		unsigned int http_port)
{
#if WEBKIT_CHECK_VERSION(1, 1, 0)
	SoupSession * session;
	char buf[32];
	struct hostent * he;
	struct in_addr addr;
	SoupURI * uri = NULL;

	session = webkit_get_default_session();
	if(type == SPT_HTTP && http != NULL && strlen(http) > 0)
	{
		if((he = gethostbyname(http)) == NULL)
			return -error_set_code(1, "%s", hstrerror(h_errno));
		memcpy(&addr.s_addr, he->h_addr, sizeof(addr.s_addr));
		snprintf(buf, sizeof(buf), "http://%s:%u/", inet_ntoa(addr),
				http_port);
		uri = soup_uri_new(buf);
	}
	g_object_set(session, "proxy-uri", uri, NULL);
	return 0;
#else
	/* FIXME really implement */
	return -error_set_code(1, "%s", strerror(ENOSYS));
#endif
}


/* useful */
/* ghtml_copy */
void ghtml_copy(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	webkit_web_view_copy_clipboard(WEBKIT_WEB_VIEW(ghtml->view));
}


/* ghtml_cut */
void ghtml_cut(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	webkit_web_view_cut_clipboard(WEBKIT_WEB_VIEW(ghtml->view));
}


/* ghtml_execute */
void ghtml_execute(GtkWidget * widget, char const * code)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	webkit_web_view_execute_script(WEBKIT_WEB_VIEW(ghtml->view), code);
}


/* ghtml_find */
gboolean ghtml_find(GtkWidget * widget, char const * text, gboolean sensitive,
		gboolean wrap)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	return webkit_web_view_search_text(WEBKIT_WEB_VIEW(ghtml->view), text,
			sensitive, TRUE, wrap);
}


gboolean ghtml_go_back(GtkWidget * widget)
{
	GHtml * ghtml;

	if(ghtml_can_go_back(widget) == FALSE)
		return FALSE;
	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	webkit_web_view_go_back(WEBKIT_WEB_VIEW(ghtml->view));
	return TRUE;
}


gboolean ghtml_go_forward(GtkWidget * widget)
{
	GHtml * ghtml;

	if(ghtml_can_go_forward(widget) == FALSE)
		return FALSE;
	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	webkit_web_view_go_forward(WEBKIT_WEB_VIEW(ghtml->view));
	return TRUE;
}


void ghtml_load_url(GtkWidget * widget, char const * url)
{
	GHtml * ghtml;
	gchar * p;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	if((p = _ghtml_make_url(NULL, url)) != NULL)
		url = p;
	webkit_web_view_open(WEBKIT_WEB_VIEW(ghtml->view), url);
	g_free(p);
	surfer_set_progress(ghtml->surfer, 0.0);
	surfer_set_security(ghtml->surfer, SS_NONE);
	_ghtml_set_status(widget, _("Connecting..."));
}


/* ghtml_paste */
void ghtml_paste(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	webkit_web_view_paste_clipboard(WEBKIT_WEB_VIEW(ghtml->view));
}


/* ghtml_print */
void ghtml_print(GtkWidget * widget)
{
#if WEBKIT_CHECK_VERSION(1, 1, 0) /* XXX may not be accurate */
	GHtml * ghtml;
	WebKitWebFrame * frame;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	frame = webkit_web_view_get_main_frame(WEBKIT_WEB_VIEW(ghtml->view));
	webkit_web_frame_print(frame);
#endif
}


/* ghtml_redo */
void ghtml_redo(GtkWidget * widget)
{
#if WEBKIT_CHECK_VERSION(1, 1, 14)
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	webkit_web_view_redo(WEBKIT_WEB_VIEW(ghtml->view));
#endif
}


void ghtml_refresh(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	webkit_web_view_reload(WEBKIT_WEB_VIEW(ghtml->view));
}


void ghtml_reload(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
#if WEBKIT_CHECK_VERSION(1, 0, 3)
	webkit_web_view_reload_bypass_cache(WEBKIT_WEB_VIEW(ghtml->view));
#else
	webkit_web_view_reload(WEBKIT_WEB_VIEW(ghtml->view));
#endif
}


void ghtml_stop(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	webkit_web_view_stop_loading(WEBKIT_WEB_VIEW(ghtml->view));
}


void ghtml_select_all(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	webkit_web_view_select_all(WEBKIT_WEB_VIEW(ghtml->view));
}


/* ghtml_undo */
void ghtml_undo(GtkWidget * widget)
{
#if WEBKIT_CHECK_VERSION(1, 1, 14)
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	webkit_web_view_undo(WEBKIT_WEB_VIEW(ghtml->view));
#endif
}


/* ghtml_unselect_all */
void ghtml_unselect_all(GtkWidget * widget)
{
	/* FIXME implement */
}


/* ghtml_zoom_in */
void ghtml_zoom_in(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	webkit_web_view_zoom_in(WEBKIT_WEB_VIEW(ghtml->view));
}


/* ghtml_zoom_out */
void ghtml_zoom_out(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	webkit_web_view_zoom_out(WEBKIT_WEB_VIEW(ghtml->view));
}


/* ghtml_zoom_reset */
void ghtml_zoom_reset(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	webkit_web_view_set_zoom_level(WEBKIT_WEB_VIEW(ghtml->view), 1.0);
}


/* private */
/* functions */
static void _ghtml_set_status(GtkWidget * widget, char const * status)
{
	GHtml * ghtml;
	gdouble progress;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	free(ghtml->status);
	if(status == NULL)
	{
		if((progress = ghtml_get_progress(widget)) == 0.0)
			status = _("Connecting...");
		else if(progress > 0.0)
			status = _("Downloading...");
	}
	/* XXX may fail */
	ghtml->status = (status != NULL) ? strdup(status) : NULL;
	surfer_set_status(ghtml->surfer, status);
}


/* callbacks */
/* on_console_message */
static gboolean _on_console_message(WebKitWebView * view, const gchar * message,
		guint line, const gchar * source, gpointer data)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(data), "ghtml");
	surfer_console_message(ghtml->surfer, message, source, line);
	return TRUE;
}


/* on_create_web_view */
static WebKitWebView * _on_create_web_view(WebKitWebView * view,
		WebKitWebFrame * frame, gpointer data)
{
	GHtml * ghtml;
	Surfer * surfer;
	GtkWidget * widget;

	if((surfer = surfer_new(NULL)) == NULL)
		return NULL;
	/* FIXME we may want the history to be copied (and then more) */
	if((widget = surfer_get_view(surfer)) == NULL)
	{
		surfer_delete(surfer);
		return NULL;
	}
	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	g_signal_connect(G_OBJECT(ghtml->view), "web-view-ready", G_CALLBACK(
				_on_web_view_ready), widget);
	return WEBKIT_WEB_VIEW(ghtml->view);
}


#ifdef WEBKIT_TYPE_DOWNLOAD
/* on_download_requested */
static gboolean _on_download_requested(WebKitWebView * view,
		WebKitDownload * download, gpointer data)
{
	GHtml * ghtml;
	char const * url;
	char const * suggested;

	ghtml = g_object_get_data(G_OBJECT(data), "ghtml");
	url = webkit_download_get_uri(download);
	suggested = webkit_download_get_suggested_filename(download);
	surfer_download(ghtml->surfer, url, suggested);
	webkit_download_cancel(download);
	return FALSE;
}
#endif


/* on_hovering_over_link */
static void _on_hovering_over_link(WebKitWebView * view, const gchar * title,
		const gchar * url, gpointer data)
{
	GtkWidget * widget = data;

	_ghtml_set_status(widget, url);
}


/* on_load_committed */
static void _on_load_committed(WebKitWebView * view, WebKitWebFrame * frame,
		gpointer data)
{
	GHtml * ghtml;
	char const * location;

	ghtml = g_object_get_data(G_OBJECT(data), "ghtml");
	if(frame == webkit_web_view_get_main_frame(view)
			&& (location = webkit_web_frame_get_uri(frame)) != NULL)
		surfer_set_location(ghtml->surfer, location);
	surfer_set_security(ghtml->surfer, ghtml_get_security(ghtml->widget));
}


/* on_load_error */
static gboolean _on_load_error(WebKitWebView * view, WebKitWebFrame * frame,
		const gchar * uri, GError * error, gpointer data)
{
	GHtml * ghtml;
#ifdef WEBKIT_POLICY_ERROR
	char const * suggested;
#endif

	ghtml = g_object_get_data(G_OBJECT(data), "ghtml");
	if(error == NULL)
		return surfer_error(ghtml->surfer, _("Unknown error"), TRUE);
#ifdef WEBKIT_NETWORK_ERROR
	if(error->domain == WEBKIT_NETWORK_ERROR
			&& error->code == WEBKIT_NETWORK_ERROR_CANCELLED)
		return TRUE; /* ignored if the user cancelled it */
#endif
#ifdef WEBKIT_POLICY_ERROR
	if(error->domain == WEBKIT_POLICY_ERROR
			&& error->code == WEBKIT_POLICY_ERROR_FRAME_LOAD_INTERRUPTED_BY_POLICY_CHANGE)
	{
		/* FIXME propose to download or cancel instead */
		if((suggested = strrchr(uri, '/')) != NULL)
			suggested++;
		surfer_download(ghtml->surfer, uri, suggested);
		return TRUE;
	}
#endif
	return surfer_error(ghtml->surfer, error->message, TRUE);
}


/* on_load_finished */
static void _on_load_finished(WebKitWebView * view, WebKitWebFrame * arg1,
			gpointer data)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(data), "ghtml");
	surfer_set_progress(ghtml->surfer, -1.0);
	_ghtml_set_status(ghtml->widget, NULL);
}


/* on_load_progress_changed */
static void _on_load_progress_changed(WebKitWebView * view, gint progress,
		gpointer data)
{
	GHtml * ghtml;
	gdouble fraction = progress;

	ghtml = g_object_get_data(G_OBJECT(data), "ghtml");
	surfer_set_progress(ghtml->surfer, fraction / 100);
	_ghtml_set_status(ghtml->widget, _("Downloading..."));
}


/* on_load_started */
static void _on_load_started(WebKitWebView * view, WebKitWebFrame * frame,
		gpointer data)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(data), "ghtml");
	surfer_set_progress(ghtml->surfer, 0.00);
	_ghtml_set_status(ghtml->widget, _("Downloading..."));
}


/* on_script_alert */
static gboolean _on_script_alert(WebKitWebView * view, WebKitWebFrame * frame,
		const gchar * message, gpointer data)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(data), "ghtml");
	surfer_warning(ghtml->surfer, message);
	return TRUE;
}


static gboolean _on_script_confirm(WebKitWebView * view, WebKitWebFrame * frame,
		const gchar * message, gboolean * confirmed, gpointer data)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(data), "ghtml");
	if(surfer_confirm(ghtml->surfer, message, confirmed) != 0)
		*confirmed = FALSE;
	return TRUE;
}

static gboolean _on_script_prompt(WebKitWebView * view, WebKitWebFrame * frame,
		const gchar * message, const gchar * default_value,
		gchar ** value, gpointer data)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(data), "ghtml");
	if(surfer_prompt(ghtml->surfer, message, default_value, value) == 0)
		return TRUE;
	*value = NULL;
	return TRUE;
}


static void _on_status_bar_text_changed(WebKitWebView * view, gchar * arg1,
		gpointer data)
{
	GtkWidget * widget = data;

	if(strlen(arg1) == 0)
		return;
	_ghtml_set_status(widget, arg1);
}


static void _on_title_changed(WebKitWebView * view, WebKitWebFrame * frame,
		const gchar * title, gpointer data)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(data), "ghtml");
	surfer_set_title(ghtml->surfer, title);
}


#if WEBKIT_CHECK_VERSION(1, 0, 3)
static gboolean _on_web_view_ready(WebKitWebView * view, gpointer data)
{
	GHtml * ghtml;
	WebKitWebWindowFeatures * features;
	gboolean b;
	gint w;
	gint h;

	ghtml = g_object_get_data(G_OBJECT(data), "ghtml");
	features = webkit_web_view_get_window_features(WEBKIT_WEB_VIEW(view));
	/* FIXME track properties with notify:: instead */
	g_object_get(G_OBJECT(features), "width", &w, "height", &h, NULL);
	if(w > 0 && h > 0)
		surfer_resize(ghtml->surfer, w, h);
	g_object_get(G_OBJECT(features), "fullscreen", &b, NULL);
	if(b == TRUE)
		surfer_set_fullscreen(ghtml->surfer, TRUE);
# ifndef EMBEDDED
	g_object_get(G_OBJECT(features), "menubar-visible", &b, NULL);
	surfer_show_menubar(ghtml->surfer, b);
# endif
	g_object_get(G_OBJECT(features), "toolbar-visible", &b, NULL);
	surfer_show_toolbar(ghtml->surfer, b);
	g_object_get(G_OBJECT(features), "statusbar-visible", &b, NULL);
	surfer_show_statusbar(ghtml->surfer, b);
	surfer_show_window(ghtml->surfer, TRUE);
	return FALSE;
}
#else /* WebKitWebWindowFeatures is not available */
static gboolean _on_web_view_ready(WebKitWebView * view, gpointer data)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(data), "ghtml");
	surfer_show_window(ghtml->surfer, TRUE);
	return FALSE;
}
#endif
