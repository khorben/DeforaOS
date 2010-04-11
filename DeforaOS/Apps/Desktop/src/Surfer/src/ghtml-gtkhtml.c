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
/* TODO:
 * - fix URL generation for relative path
 * - implement selection
 * - more meaningful status updates
 * - implement cookies (beware same-origin policy)
 * - implement referer
 * - implement history
 * - implement anchors
 * - probably need to implement html_stream_cancel
 * - need to take care of CSRF? eg remotely load local files */



#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <libintl.h>
#include <libgtkhtml/gtkhtml.h>
#include <libgtkhtml/view/htmlselection.h>
#define GNET_EXPERIMENTAL
#include <gnet.h>
#include <System.h>
#include "ghtml.h"
#include "callbacks.h"
#include "../config.h"
#include "common/history.c"
#include "common/url.c"
#define _(string) gettext(string)
#define max(a, b) ((a) > (b) ? (a) : (b))


/* GHtml */
/* private */
typedef struct _GHtmlConn GHtmlConn;

typedef struct _GHtml
{
	Surfer * surfer;
	char * source; /* XXX keep the size */

	/* history */
	GList * history;
	GList * current;

	/* connections */
	struct _GHtmlConn ** conns;
	size_t conns_cnt;

	/* html widget */
	HtmlDocument * html_document;
	gchar * html_title;
	GtkWidget * html_view;
} GHtml;

struct _GHtmlConn
{
	GHtml * ghtml;

	char * url;
	char const * anchor;
	guint64 content_length;
	guint64 data_received;
	HtmlStream * stream;

	int direct;
	int image;

	/* file */
	GIOChannel * file;
	guint64 file_size;
	guint64 file_read;

	/* http */
	GConnHttp * http;
	GConnHttpMethod http_method;
	gchar * http_post;
};


/* prototypes */
static GHtmlConn * _ghtmlconn_new(GHtml * ghtml, HtmlStream * stream,
		gchar const * url, gchar const * post);
static void _ghtmlconn_delete(GHtmlConn * ghtmlconn);

static char const * _ghtml_get_base(GHtml * ghtml);
static int _ghtml_set_base(GHtml * ghtml, char const * url);
static int _ghtml_document_load(GHtml * ghtml, gchar const * url,
		gchar const * post);
static int _ghtml_document_reload(GHtml * ghtml);
static gchar * _ghtml_make_url(gchar const * base, gchar const * url);
static int _ghtml_source_append(GHtml * ghtml, char const * buf, size_t size);
static void _ghtml_stop(GHtml * ghtml);
static GHtmlConn * _ghtml_stream_load(GHtml * ghtml, HtmlStream * stream,
		gchar const * url, gchar const * post);

/* callbacks */
static gboolean _on_button_press_event(GtkWidget* widget,
		GdkEventButton * event, gpointer data);
static void _on_link_clicked(HtmlDocument * document, const gchar * url);
static void _on_request_url(HtmlDocument * document, const gchar * url,
		HtmlStream * stream);
static void _on_set_base(HtmlDocument * document, const gchar * url);
static void _on_submit(HtmlDocument * document, const gchar * url,
		const gchar * method, const gchar * encoding);
static void _on_title_changed(HtmlDocument * document, const gchar * title);
static void _on_url(HtmlView * view, const gchar * url);


/* public */
/* functions */
GtkWidget * ghtml_new(Surfer * surfer)
{
	GHtml * ghtml;
	GtkWidget * widget;

	if((ghtml = malloc(sizeof(*ghtml))) == NULL)
		return NULL;
	ghtml->surfer = surfer;
	ghtml->source = NULL;
	ghtml->history = NULL;
	ghtml->current = NULL;
	ghtml->conns = NULL;
	ghtml->conns_cnt = 0;
	ghtml->html_view = html_view_new();
	g_object_set_data(G_OBJECT(ghtml->html_view), "ghtml", ghtml);
	g_signal_connect(G_OBJECT(ghtml->html_view), "button-press-event",
			G_CALLBACK(_on_button_press_event), ghtml);
	g_signal_connect(G_OBJECT(ghtml->html_view), "on-url", G_CALLBACK(
				_on_url), NULL);
	ghtml->html_document = html_document_new();
	ghtml->html_title = NULL;
	g_object_set_data(G_OBJECT(ghtml->html_document), "ghtml", ghtml);
	g_signal_connect(G_OBJECT(ghtml->html_document), "link-clicked",
			G_CALLBACK(_on_link_clicked), NULL);
	g_signal_connect(G_OBJECT(ghtml->html_document), "request-url",
			G_CALLBACK(_on_request_url), NULL);
	g_signal_connect(G_OBJECT(ghtml->html_document), "set-base", G_CALLBACK(
				_on_set_base), NULL);
	g_signal_connect(G_OBJECT(ghtml->html_document), "submit", G_CALLBACK(
				_on_submit), NULL);
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
/* ghtml_can_go_back */
gboolean ghtml_can_go_back(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	return _history_can_go_back(ghtml->current);
}


/* ghtml_can_go_forward */
gboolean ghtml_can_go_forward(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	return _history_can_go_forward(ghtml->current);
}


/* ghtml_get_link_message */
char const * ghtml_get_link_message(GtkWidget * ghtml)
{
	/* FIXME implement */
	return NULL;
}


/* ghtml_get_location */
char const * ghtml_get_location(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	return _history_get_location(ghtml->current);
}


/* ghtml_get_source */
char const * ghtml_get_source(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	return ghtml->source;
}


/* ghtml_get_title */
char const * ghtml_get_title(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	return ghtml->html_title;
}


/* ghtml_set_base */
int ghtml_set_base(GtkWidget * widget, char const * url)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	return _ghtml_set_base(ghtml, url);
}


/* useful */
/* ghtml_find */
gboolean ghtml_find(GtkWidget * ghtml, char const * text, gboolean sensitive,
		gboolean wrap)
{
	/* FIXME implement */
	return FALSE;
}


/* ghtml_go_back */
gboolean ghtml_go_back(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	if(ghtml->current == NULL || ghtml->current->prev == NULL)
		return FALSE;
	ghtml->current = ghtml->current->prev;
	ghtml_refresh(widget);
	return (ghtml->current->prev != NULL) ? TRUE : FALSE;
}


/* ghtml_go_forward */
gboolean ghtml_go_forward(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	if(ghtml->current == NULL || ghtml->current->next == NULL)
		return FALSE;
	ghtml->current = ghtml->current->next;
	ghtml_refresh(widget);
	return (ghtml->current->next != NULL) ? TRUE : FALSE;
}


/* ghtml_load_url */
void ghtml_load_url(GtkWidget * widget, char const * url)
{
	GHtml * ghtml;
	gchar * link;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, url);
#endif
	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	if((link = _ghtml_make_url(NULL, url)) != NULL)
		url = link;
	_ghtml_document_load(ghtml, url, NULL);
	g_free(link);
}


/* ghtml_print */
void ghtml_print(GtkWidget * ghtml)
{
	/* FIXME implement */
}


/* ghtml_refresh */
void ghtml_refresh(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	_ghtml_document_reload(ghtml);
}


/* ghtml_reload */
void ghtml_reload(GtkWidget * ghtml)
{
	ghtml_refresh(ghtml);
}


/* ghtml_select_all */
void ghtml_select_all(GtkWidget * widget)
{
#if 0 /* FIXME does not work */
	GHtml * ghtml;
	DomNode * node;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	if((node = html_document_find_anchor(ghtml->html_document, "")) != NULL)
		html_selection_set(HTML_VIEW(ghtml->html_view), node, 0, -1);
#endif
}


/* ghtml_stop */
void ghtml_stop(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	_ghtml_stop(ghtml);
}


/* ghtml_unselect_all */
void ghtml_unselect_all(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	html_selection_clear(HTML_VIEW(ghtml->html_view));
}


/* ghtml_zoom_in */
void ghtml_zoom_in(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	html_view_zoom_in(HTML_VIEW(ghtml->html_view));
}


/* ghtml_zoom_out */
void ghtml_zoom_out(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	html_view_zoom_out(HTML_VIEW(ghtml->html_view));
}


/* ghtml_zoom_reset */
void ghtml_zoom_reset(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	html_view_zoom_reset(HTML_VIEW(ghtml->html_view));
}


/* private */
/* functions */
static GHtmlConn * _ghtmlconn_new(GHtml * ghtml, HtmlStream * stream,
		gchar const * url, gchar const * post)
{
	GHtmlConn ** p;
	GHtmlConn * c;
	char * q = NULL;

	/* FIXME leaks memory: records are not re-used */
	if((p = realloc(ghtml->conns, sizeof(*p) * (ghtml->conns_cnt + 1)))
			== NULL)
		return NULL;
	ghtml->conns = p;
	if((c = malloc(sizeof(*c))) == NULL)
		return NULL;
	ghtml->conns[ghtml->conns_cnt] = c;
	c->ghtml = ghtml;
	c->url = strdup(url);
	/* look for an anchor */
	/* XXX should it really be done here? */
	if(c->url != NULL && (q = strrchr(c->url, '#')) != NULL)
		*q = '\0';
	c->anchor = (q != NULL) ? ++q : NULL;
#ifdef DEBUG
	if(c->anchor != NULL)
		fprintf(stderr, "DEBUG: %s() anchor=\"%s\"\n", __func__,
				c->anchor);
#endif
	c->content_length = 0;
	c->data_received = 0;
	c->stream = stream;
	c->direct = 0;
	c->image = 0;
	c->file = NULL;
	c->file_size = 0;
	c->file_read = 0;
	c->http = NULL;
	c->http_method = (post == NULL) ? GNET_CONN_HTTP_METHOD_GET
		: GNET_CONN_HTTP_METHOD_POST;
	c->http_post = (post != NULL) ? g_strdup(post) : NULL;
	if(c->url == NULL || (post != NULL && c->http_post == NULL))
	{
		_ghtmlconn_delete(c);
		return NULL;
	}
	ghtml->conns_cnt++;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, %p, \"%s\") => %p\n", __func__, ghtml,
			stream, url, c);
#endif
	return c;
}


/* ghtmlconn_delete */
static void _ghtmlconn_delete_file(GHtmlConn * ghtmlconn);
static void _ghtmlconn_delete_http(GHtmlConn * ghtmlconn);

static void _ghtmlconn_delete(GHtmlConn * ghtmlconn)
{
	GHtml * ghtml = ghtmlconn->ghtml;
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p)\n", __func__, ghtmlconn);
#endif
	for(i = 0; i < ghtml->conns_cnt; i++)
		if(ghtml->conns[i] == ghtmlconn)
		{
			ghtml->conns[i] = NULL; /* don't double free later */
			break;
		}
	if(ghtmlconn->file != NULL)
		_ghtmlconn_delete_file(ghtmlconn);
	else if(ghtmlconn->http != NULL)
		_ghtmlconn_delete_http(ghtmlconn);
	free(ghtmlconn->url);
	if(ghtmlconn->stream != NULL)
		html_stream_close(ghtmlconn->stream);
	free(ghtmlconn);
	/* free the connections array if possible */
	for(i = 0; i < ghtml->conns_cnt; i++)
		if(ghtml->conns[i] != NULL)
			return;
	free(ghtml->conns);
	ghtml->conns = NULL;
	ghtml->conns_cnt = 0;
}

static void _ghtmlconn_delete_file(GHtmlConn * ghtmlconn)
{
	if(ghtmlconn->file != NULL)
		g_io_channel_unref(ghtmlconn->file);
}

static void _ghtmlconn_delete_http(GHtmlConn * ghtmlconn)
{
	gnet_conn_http_delete(ghtmlconn->http);
	g_free(ghtmlconn->http_post);
}


/* ghtml_get_base */
static char const * _ghtml_get_base(GHtml * ghtml)
{
	History * h;

	if(ghtml->current == NULL || (h = ghtml->current->data) == NULL)
		return NULL;
	return (h->base != NULL) ? h->base : h->url;
}


/* ghtml_set_base */
static int _ghtml_set_base(GHtml * ghtml, char const * url)
{
	History * h;

	if(ghtml->current == NULL || (h = ghtml->current->data) == NULL)
		return 1;
	free(h->base);
	if(url == NULL)
		h->base = NULL;
	else if((h->base = strdup(url)) == NULL)
		return 1;
	return 0;
}


/* ghtml_document_load */
static int _ghtml_document_load(GHtml * ghtml, gchar const * url,
		gchar const * post)
{
	GHtmlConn * gc;
	History * h;

	_ghtml_stop(ghtml);
	if((h = _history_new(url, post)) == NULL)
		return 1;
	ghtml->history = g_list_append(ghtml->history, h);
	ghtml->current = g_list_last(ghtml->history);
	surfer_set_location(ghtml->surfer, url);
	surfer_set_title(ghtml->surfer, NULL);
	html_document_open_stream(ghtml->html_document, "text/html");
	if((gc = _ghtml_stream_load(ghtml, ghtml->html_document->current_stream,
			url, post)) != NULL)
		gc->direct = 1;
	return gc != NULL ? 0 : 1;
}


static int _ghtml_document_reload(GHtml * ghtml)
{
	GHtmlConn * gc;
	History * h;

	_ghtml_stop(ghtml);
	if(ghtml->current == NULL || (h = ghtml->current->data) == NULL)
		return 0;
	surfer_set_location(ghtml->surfer, h->url);
	surfer_set_title(ghtml->surfer, NULL);
	html_document_open_stream(ghtml->html_document, "text/html");
	/* FIXME warn if h->post is set */
	if((gc = _ghtml_stream_load(ghtml, ghtml->html_document->current_stream,
			h->url, h->post)) != NULL)
		gc->direct = 1;
	return gc != NULL ? 0 : 1;
}


/* ghtml_source_append */
static int _ghtml_source_append(GHtml * ghtml, char const * buf, size_t size)
{
	size_t len = (ghtml->source != NULL) ? strlen(ghtml->source) : 0;
	char * p;

	/* FIXME this may lose data (eg if it contains NULL bytes) */
	if((p = realloc(ghtml->source, len + size + 1)) == NULL)
		return 1; /* XXX report error */
	ghtml->source = p;
	memcpy(p + len, buf, size);
	p[len + size] = '\0';
	return 0;
}


/* ghtml_stop */
static void _ghtml_stop(GHtml * ghtml)
{
	size_t i;

	free(ghtml->source);
	ghtml->source = NULL;
	for(i = 0; i < ghtml->conns_cnt; i++)
		if(ghtml->conns[i] != NULL)
			_ghtmlconn_delete(ghtml->conns[i]);
	free(ghtml->conns);
	ghtml->conns = NULL;
	ghtml->conns_cnt = 0;
}


/* ghtml_stream_load */
static gboolean _stream_load_idle(gpointer data);
static gboolean _stream_load_idle_directory(GHtmlConn * conn);
static gboolean _stream_load_idle_file(GHtmlConn * conn);
static gboolean _stream_load_watch_file(GIOChannel * source,
		GIOCondition condition, gpointer data);
static gboolean _stream_load_idle_http(GHtmlConn * conn);
static void _stream_load_watch_http(GConnHttp * connhttp,
		GConnHttpEvent * event, gpointer data);
static void _http_connected(GHtmlConn * conn);
static void _http_data_complete(GConnHttpEventData * event, GHtmlConn * conn);
static void _http_data_partial(GConnHttpEventData * event, GHtmlConn * conn);
static void _http_data_progress(GConnHttpEventData * event, GHtmlConn * conn);
static void _http_error(GConnHttpEventError * event, GHtmlConn * conn);
static void _http_redirect(GConnHttpEventRedirect * event, GHtmlConn * conn);
static void _http_resolved(GConnHttpEventResolved * event, GHtmlConn * conn);
static void _http_response(GConnHttpEventResponse * event, GHtmlConn * conn);
static void _http_timeout(GHtmlConn * conn);

static GHtmlConn * _ghtml_stream_load(GHtml * ghtml, HtmlStream * stream,
		gchar const * url, gchar const * post)
{
	GHtmlConn * conn;

	if((conn = _ghtmlconn_new(ghtml, stream, url, post)) == NULL)
		return NULL;
	g_idle_add(_stream_load_idle, conn);
	return conn;
}

static gboolean _stream_load_idle(gpointer data)
{
	GHtmlConn * conn = data;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p) \"%s\"\n", __func__, data, conn->url);
#endif
	html_view_jump_to_anchor(HTML_VIEW(conn->ghtml->html_view),
			(conn->anchor != NULL) ? conn->anchor : "");
	if(conn->url[0] == '/')
		return _stream_load_idle_file(conn);
	if(strncmp(conn->url, "file:", 5) == 0)
	{
		strcpy(conn->url, &conn->url[5]); /* XXX no corruption? */
		return _stream_load_idle_file(conn);
	}
	if(strncmp(conn->url, "http:", 5) == 0)
		return _stream_load_idle_http(conn);
	surfer_error(conn->ghtml->surfer, _("Unknown protocol"), 0);
	_ghtmlconn_delete(conn);
	return FALSE;
}

static gboolean _stream_load_idle_directory(GHtmlConn * conn)
{
	const char tail[] = "</ul>\n<hr>\n</body></html>\n";
	char buf[1024];
	DIR * dir;
	struct dirent * de;

	if((dir = opendir(conn->url)) == NULL)
	{
		surfer_error(conn->ghtml->surfer, strerror(errno), 0);
		_ghtmlconn_delete(conn);
		return FALSE;
	}
	if(snprintf(buf, sizeof(buf), "%s/", conn->url) < (int)sizeof(buf))
		_ghtml_set_base(conn->ghtml, buf); /* XXX what otherwise? */
	snprintf(buf, sizeof(buf), "%s%s%s%s%s", "<html><head><title>Index of ",
			conn->url, "</title><body>\n<h1>Index of ", conn->url,
			"</h1>\n<hr>\n<ul>\n");
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s", buf);
#endif
	html_stream_write(conn->stream, buf, strlen(buf));
	while((de = readdir(dir)) != NULL)
	{
		if(strcmp(de->d_name, ".") == 0)
			continue;
		snprintf(buf, sizeof(buf), "%s%s%s%s%s", "<li><a href=\"",
				de->d_name, "\">", de->d_name, "</a></li>\n");
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s", buf);
#endif
		html_stream_write(conn->stream, buf, strlen(buf));
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s", tail);
#endif
	html_stream_write(conn->stream, tail, sizeof(tail) - 1);
	closedir(dir);
	surfer_set_progress(conn->ghtml->surfer, -1.0);
	surfer_set_status(conn->ghtml->surfer, NULL);
	_ghtmlconn_delete(conn);
	return FALSE;
}

static gboolean _stream_load_idle_file(GHtmlConn * conn)
{
	int fd;
	struct stat st;
	GIOChannel * channel;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__, conn->url);
#endif
	if((fd = open(conn->url, O_RDONLY)) < 0)
	{
		surfer_error(conn->ghtml->surfer, strerror(errno), 0);
		_ghtmlconn_delete(conn);
	}
	else
	{
		surfer_set_progress(conn->ghtml->surfer, 0.0);
		surfer_set_status(conn->ghtml->surfer, _("Reading file..."));
		if(fstat(fd, &st) == 0)
		{
			if(S_ISDIR(st.st_mode))
			{
				close(fd);
				return _stream_load_idle_directory(conn);
			}
			conn->file_size = st.st_size;
		}
		channel = g_io_channel_unix_new(fd);
		g_io_add_watch(channel, G_IO_IN, _stream_load_watch_file, conn);
	}
	return FALSE;
}

static gboolean _stream_load_watch_file(GIOChannel * source,
		GIOCondition condition, gpointer data)
{
	GHtmlConn * conn = data;
	gsize len;
	char buf[BUFSIZ];
	gdouble fraction;

	if(condition != G_IO_IN)
	{
		_ghtmlconn_delete(conn);
		return FALSE;
	}
	if(g_io_channel_read(source, buf, sizeof(buf), &len)
			!= G_IO_ERROR_NONE)
	{
		/* FIXME report error */
		_ghtmlconn_delete(conn);
		return FALSE;
	}
	if(len == 0) /* no more data */
	{
		surfer_set_progress(conn->ghtml->surfer, 1.0);
		surfer_set_status(conn->ghtml->surfer, NULL);
		_ghtmlconn_delete(conn);
		return FALSE;
	}
	_ghtml_source_append(conn->ghtml, buf, len);
	html_stream_write(conn->stream, buf, len);
	conn->file_read+=len;
	if(conn->file_size > 0)
	{
		fraction = conn->file_read;
		surfer_set_progress(conn->ghtml->surfer,
				fraction / conn->file_size);
	}
	return TRUE;
}

static gboolean _stream_load_idle_http(GHtmlConn * conn)
{
	surfer_set_progress(conn->ghtml->surfer, -1.0);
	surfer_set_status(conn->ghtml->surfer, _("Resolving..."));
	conn->http = gnet_conn_http_new();
	gnet_conn_http_set_uri(conn->http, conn->url);
	gnet_conn_http_set_user_agent(conn->http, "DeforaOS " PACKAGE);
	gnet_conn_http_set_method(conn->http, conn->http_method,
			conn->http_post, (conn->http_post != NULL)
			? strlen(conn->http_post) : 0);
	gnet_conn_http_run_async(conn->http, _stream_load_watch_http, conn);
	return FALSE;
}

static void _stream_load_watch_http(GConnHttp * connhttp,
		GConnHttpEvent * event, gpointer data)
	/* FIXME handle error cases */
{
	GHtmlConn * conn = data;

	if(conn->http != connhttp)
		return; /* FIXME report error */
	switch(event->type)
	{
		case GNET_CONN_HTTP_CONNECTED:
			_http_connected(conn);
			return;
		case GNET_CONN_HTTP_DATA_COMPLETE:
			_http_data_complete((GConnHttpEventData*)event, conn);
			return;
		case GNET_CONN_HTTP_DATA_PARTIAL:
			_http_data_partial((GConnHttpEventData*)event, conn);
			return;
		case GNET_CONN_HTTP_ERROR:
			_http_error((GConnHttpEventError*)event, conn);
			return;
		case GNET_CONN_HTTP_REDIRECT:
			_http_redirect((GConnHttpEventRedirect*)event, conn);
			return;
		case GNET_CONN_HTTP_RESOLVED:
			_http_resolved((GConnHttpEventResolved*)event, conn);
			return;
		case GNET_CONN_HTTP_RESPONSE:
			_http_response((GConnHttpEventResponse*)event, conn);
			return;
		case GNET_CONN_HTTP_TIMEOUT:
			_http_timeout(conn);
			return;
	}
}

static void _http_connected(GHtmlConn * conn)
{
	surfer_set_status(conn->ghtml->surfer, _("Connected"));
}

static void _http_data_complete(GConnHttpEventData * event, GHtmlConn * conn)
{
	gchar * buf;
	gsize size;
	GHtml * ghtml;

	if(gnet_conn_http_steal_buffer(conn->http, &buf, &size) != TRUE)
	{
		/* FIXME report error */
		surfer_set_progress(conn->ghtml->surfer, -1.0);
	}
	else
	{
		if(size > 0)
		{
			if(conn == conn->ghtml->conns[0]) /* XXX ugly */
				_ghtml_source_append(conn->ghtml, buf, size);
			html_stream_write(conn->stream, buf, size);
		}
		_http_data_progress(event, conn);
	}
	ghtml = conn->ghtml;
	_ghtmlconn_delete(conn);
	if(ghtml->conns_cnt == 0)
		surfer_set_status(ghtml->surfer, NULL);
}

static void _http_data_partial(GConnHttpEventData * event, GHtmlConn * conn)
{
	gchar * buf;
	gsize size;

	surfer_set_status(conn->ghtml->surfer, _("Downloading..."));
	if(gnet_conn_http_steal_buffer(conn->http, &buf, &size) != TRUE)
	{
		/* FIXME report error */
		_ghtmlconn_delete(conn);
		return;
	}
	if(conn == conn->ghtml->conns[0]) /* XXX ugly */
		_ghtml_source_append(conn->ghtml, buf, size);
	html_stream_write(conn->stream, buf, size);
	_http_data_progress(event, conn);
}

static void _http_data_progress(GConnHttpEventData * event, GHtmlConn * conn)
{
	size_t i;
	GHtmlConn * p;
	guint64 len = 1; /* don't divide by zero */
	guint64 rec = 1;
	gdouble fraction;

	conn->data_received = event->data_received;
	conn->content_length = event->content_length;
	for(i = 0; i < conn->ghtml->conns_cnt; i++)
	{
		if((p = conn->ghtml->conns[i]) == NULL
				|| p->content_length == 0)
			continue;
		len += (p->content_length != 0) ? p->content_length
			: p->data_received + 1;
		rec += p->data_received;
	}
	fraction = rec;
	surfer_set_progress(conn->ghtml->surfer, fraction / len);
}

static void _http_error(GConnHttpEventError * event, GHtmlConn * conn)
{
	char * msg;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	switch(event->code)
	{
		case GNET_CONN_HTTP_ERROR_PROTOCOL_UNSUPPORTED:
			msg = _("Unsupported protocol");
			break;
#if GNET_CHECK_VERSION(2, 0, 8) /* XXX unsure about the exact version */
		case GNET_CONN_HTTP_ERROR_HOSTNAME_RESOLUTION:
			msg = _("Unknown host");
			break;
#endif
		case GNET_CONN_HTTP_ERROR_UNSPECIFIED:
		default:
			msg = _("Unspecified error");
			break;
	}
	surfer_set_progress(conn->ghtml->surfer, -1.0);
	surfer_set_status(conn->ghtml->surfer, NULL);
	surfer_error(conn->ghtml->surfer, msg, 0);
	_ghtmlconn_delete(conn);
}

static void _http_redirect(GConnHttpEventRedirect * event, GHtmlConn * conn)
{
	GHtml * ghtml = conn->ghtml;
	char buf[256];
	char * url = event->new_location;

	snprintf(buf, sizeof(buf), "%s", _("Redirecting..."));
	if(conn == conn->ghtml->conns[0] && url == NULL) /* XXX ugly */
	{
		surfer_set_status(ghtml->surfer, buf);
		return;
	}
	if((url = _ghtml_make_url(_ghtml_get_base(ghtml), url)) != NULL)
	{
		snprintf(buf, sizeof(buf), "%s %s", _("Redirecting to "), url);
		g_free(conn->url);
		conn->url = url;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() URL: %s, %d (%u/%u)\n", __func__, url,
			event->auto_redirect, event->num_redirects,
			event->max_redirects);
#endif
	if(conn != conn->ghtml->conns[0]) /* XXX ugly */
		return;
	_ghtml_set_base(ghtml, url);
	surfer_set_location(ghtml->surfer, url);
	surfer_set_status(ghtml->surfer, buf);
}

static void _http_resolved(GConnHttpEventResolved * event, GHtmlConn * conn)
{
	char buf[256];
	char * name;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(event->ia == NULL)
	{
#if 0 /* XXX check again if this is really an error case */
		surfer_set_progress(conn->ghtml->surfer, -1.0);
		surfer_set_status(conn->ghtml->surfer, NULL);
		surfer_error(conn->ghtml->surfer, "Unknown host", 0);
		_ghtmlconn_delete(conn);
#endif
	}
	else if((name = gnet_inetaddr_get_name_nonblock(event->ia)) == NULL)
		surfer_set_status(conn->ghtml->surfer, _("Connecting..."));
	else
	{
		snprintf(buf, sizeof(buf), "%s%s%s%d", _("Connecting to "),
				name, ":", gnet_inetaddr_get_port(event->ia));
		surfer_set_status(conn->ghtml->surfer, buf);
	}
}

static void _http_response(GConnHttpEventResponse * event, GHtmlConn * conn)
{
	size_t i;
	char buf[256];

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() %u %s\n", __func__, event->response_code,
			conn->url);
#endif
	/* embed images in HTML */
	/* only continue if successful direct query and we have headers */
	if(conn->direct == 0 || event->response_code != 200
			|| event->header_fields == NULL)
		return;
	for(i = 0; event->header_fields[i] != NULL; i++)
		if(strcasecmp(event->header_fields[i], "Content-type") == 0)
			break;
	if(event->header_fields[i] == NULL)
		return;
	/* check if it's an image */
	if(strncmp(event->header_values[i], "image/", 6) != 0)
		return;
	conn->image = 1;
	snprintf(buf, sizeof(buf), "<img src=\"%s\">\n", conn->url);
	html_stream_write(conn->stream, buf, strlen(buf));
	_ghtmlconn_delete(conn);
}

static void _http_timeout(GHtmlConn * conn)
{
	surfer_error(conn->ghtml->surfer, _("Timeout"), 0);
	surfer_set_progress(conn->ghtml->surfer, -1.0);
	surfer_set_status(conn->ghtml->surfer, NULL);
	_ghtmlconn_delete(conn);
}


/* callbacks */
static gboolean _on_button_press_event(GtkWidget* widget,
		GdkEventButton * event, gpointer data)
{
	GHtml * ghtml = data;
	Surfer * surfer;
	GtkWidget * menu;
	GtkWidget * menuitem;

	if(event->type == GDK_BUTTON_PRESS
			&& event->button == 3)
	{
		surfer = ghtml->surfer;
		menu = gtk_menu_new();
		menuitem = gtk_image_menu_item_new_from_stock(
				GTK_STOCK_GO_BACK, NULL);
		g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(
					on_back), surfer);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		menuitem = gtk_image_menu_item_new_from_stock(
				GTK_STOCK_GO_FORWARD, NULL);
		g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(
					on_forward), surfer);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		menuitem = gtk_image_menu_item_new_from_stock(
				GTK_STOCK_REFRESH, NULL);
		g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(
					on_refresh), surfer);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		gtk_widget_show_all(menu);
		gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
				event->button, event->time);
	}
	return FALSE;
}


static void _on_link_clicked(HtmlDocument * document, const gchar * url)
{
	GHtml * ghtml;
	gchar * link;

	ghtml = g_object_get_data(G_OBJECT(document), "ghtml");
	if((link = _ghtml_make_url(_ghtml_get_base(ghtml), url)) == NULL)
		return; /* FIXME report error */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\") base=\"%s\" => \"%s\"\n", __func__,
			url, _ghtml_get_base(ghtml), link);
#endif
	_ghtml_document_load(ghtml, link, NULL);
	g_free(link);
}


static void _on_request_url(HtmlDocument * document, const gchar * url,
		HtmlStream * stream)
{
	GHtml * ghtml;
	gchar * link;

	ghtml = g_object_get_data(G_OBJECT(document), "ghtml");
	if((link = _ghtml_make_url(_ghtml_get_base(ghtml), url)) == NULL)
		return; /* FIXME report error */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\") base=\"%s\" => \"%s\"\n", __func__,
			url, _ghtml_get_base(ghtml), link);
#endif
	_ghtml_stream_load(ghtml, stream, link, NULL);
	g_free(link);
}


static void _on_set_base(HtmlDocument * document, const gchar * url)
{
	GHtml * ghtml;
	gchar * u;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, url);
#endif
	ghtml = g_object_get_data(G_OBJECT(document), "ghtml");
	/* FIXME may fail */
	u = _ghtml_make_url(_ghtml_get_base(ghtml), url);
	_ghtml_set_base(ghtml, u);
	g_free(u);
}


static void _on_submit(HtmlDocument * document, const gchar * url,
		const gchar * method, const gchar * encoding)
{
	GHtml * ghtml;
	gchar * u;
	gchar * v;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", \"%s\", \"%s\")\n", __func__,
			url, method, encoding);
#endif
	ghtml = g_object_get_data(G_OBJECT(document), "ghtml");
	if((u = _ghtml_make_url(_ghtml_get_base(ghtml), url)) == NULL)
		return; /* FIXME report error */
	if(method == NULL || strcasecmp(method, "GET") == 0)
	{
		v = (encoding != NULL) ? g_strdup_printf("%s?%s", u, encoding)
			: g_strdup(u);
		if(v == NULL)
			; /* FIXME report error */
		else
			_ghtml_document_load(ghtml, v, NULL);
		g_free(v);
	}
	else if(strcasecmp(method, "POST") == 0)
		_ghtml_document_load(ghtml, u, encoding);
	else
		surfer_error(ghtml->surfer, _("Unsupported method"), 0);
	g_free(u);
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
	surfer_set_title(ghtml->surfer, title);
}


static void _on_url(HtmlView * view, const gchar * url)
{
	GHtml * ghtml;
	gchar * link;

	ghtml = g_object_get_data(G_OBJECT(view), "ghtml");
	if(url == NULL)
	{
		surfer_set_status(ghtml->surfer, NULL);
		return;
	}
	if((link = _ghtml_make_url(_ghtml_get_base(ghtml), url)) != NULL)
	{
		surfer_set_status(ghtml->surfer, link);
		g_free(link);
	}
}
