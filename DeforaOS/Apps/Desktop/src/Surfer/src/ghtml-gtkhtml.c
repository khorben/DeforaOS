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
#include <errno.h>
#include <libgtkhtml/gtkhtml.h>
#include <libgtkhtml/view/htmlselection.h>
#define GNET_EXPERIMENTAL
#include <gnet.h>
#include <System.h>
#include "ghtml.h"
#include "../config.h"


/* GHtml */
/* private */
typedef struct _GHtmlConn GHtmlConn;

typedef struct _GHtml
{
	Surfer * surfer;

	gchar * html_base;
	gchar * html_url;
	/* FIXME implement history */

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

static int _ghtml_set_base(GHtml * ghtml, char const * url);
static int _ghtml_document_load(GHtml * ghtml, gchar const * url,
		gchar const * post);
static gchar * _ghtml_make_url(gchar const * base, gchar const * url);
static void _ghtml_stop(GHtml * ghtml);
static GHtmlConn * _ghtml_stream_load(GHtml * ghtml, HtmlStream * stream,
		gchar const * url, gchar const * post);

/* callbacks */
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
	ghtml->html_base = NULL;
	ghtml->html_url = NULL;
	ghtml->conns = NULL;
	ghtml->conns_cnt = 0;
	ghtml->html_view = html_view_new();
	g_object_set_data(G_OBJECT(ghtml->html_view), "ghtml", ghtml);
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
gboolean ghtml_can_go_back(GtkWidget * ghtml)
{
	/* FIXME implement */
	return FALSE;
}


/* ghtml_can_go_forward */
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


/* ghtml_get_location */
char const * ghtml_get_location(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	return ghtml->html_url;
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
/* ghtml_go_back */
gboolean ghtml_go_back(GtkWidget * ghtml)
{
	/* FIXME implement */
	return FALSE;
}


/* ghtml_go_forward */
gboolean ghtml_go_forward(GtkWidget * ghtml)
{
	/* FIXME implement */
	return FALSE;
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
	if(_ghtml_document_load(ghtml, url, NULL) != 0)
	{
		g_free(link);
		return;
	}
	g_free(ghtml->html_base);
	ghtml->html_base = (link != NULL) ? link : g_strdup(url);
	g_free(ghtml->html_url);
	ghtml->html_url = g_strdup(url);
}


/* ghtml_refresh */
void ghtml_refresh(GtkWidget * widget)
{
	GHtml * ghtml;

	ghtml = g_object_get_data(G_OBJECT(widget), "ghtml");
	if(ghtml->html_url == NULL)
		return;
	/* FIXME keep and warn about POST data */
	_ghtml_document_load(ghtml, ghtml->html_url, NULL);
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


/* ghtml_set_base */
static int _ghtml_set_base(GHtml * ghtml, char const * url)
{
	g_free(ghtml->html_base);
	if(url != NULL)
	{
		if((ghtml->html_base = g_strdup(url)) == NULL)
			return 1;
	}
	else
		ghtml->html_base = NULL;
	return 0;
}


/* ghtml_document_load */
static int _ghtml_document_load(GHtml * ghtml, gchar const * url,
		gchar const * post)
{
	GHtmlConn * gc;

	_ghtml_stop(ghtml);
	surfer_set_location(ghtml->surfer, url);
	surfer_set_title(ghtml->surfer, NULL);
	html_document_open_stream(ghtml->html_document, "text/html");
	if((gc = _ghtml_stream_load(ghtml, ghtml->html_document->current_stream,
			url, post)) != NULL)
		gc->direct = 1;
	return gc != NULL ? 0 : 1;
}


/* ghtml_make_url */
static gchar * _ghtml_make_url(gchar const * base, gchar const * url)
{
	char * b;
	char * p;

	if(url == NULL)
		return NULL;
	/* XXX use a more generic protocol finder (strchr(':')) */
	if(strncmp("http://", url, 7) == 0)
		return g_strdup(url);
	if(strncmp("ftp://", url, 6) == 0)
		return g_strdup(url);
	if(base != NULL)
	{
		if(url[0] == '/')
			/* FIXME construct from / of base */
			return g_strdup_printf("%s%s", base, url);
		/* construct from basename */
		if((b = strdup(base)) == NULL)
			return NULL;
		if((p = strrchr(b, '/')) != NULL)
			*p = '\0';
		p = g_strdup_printf("%s/%s", b, url);
		free(b);
		return p;
	}
	/* base is NULL, url is not NULL */
	if(url[0] == '/')
		return g_strdup(url);
	/* guess protocol */
	if(strncmp("ftp", url, 3) == 0)
		return g_strdup_printf("%s%s", "ftp://", url);
	/* FIXME guess http only for "www.*"? we're already in GNet...? */
	return g_strdup_printf("%s%s", "http://", url);
}


/* ghtml_stop */
static void _ghtml_stop(GHtml * ghtml)
{
	size_t i;

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
	surfer_error(conn->ghtml->surfer, "Unknown protocol", 0);
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
		surfer_set_status(conn->ghtml->surfer, "Reading file...");
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
	surfer_set_status(conn->ghtml->surfer, "Resolving...");
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
			return _http_connected(conn);
		case GNET_CONN_HTTP_DATA_COMPLETE:
			return _http_data_complete((GConnHttpEventData*)event,
					conn);
		case GNET_CONN_HTTP_DATA_PARTIAL:
			return _http_data_partial((GConnHttpEventData*)event,
					conn);
		case GNET_CONN_HTTP_ERROR:
			return _http_error((GConnHttpEventError*)event, conn);
		case GNET_CONN_HTTP_REDIRECT:
			return _http_redirect((GConnHttpEventRedirect*)event,
					conn);
		case GNET_CONN_HTTP_RESOLVED:
			return _http_resolved((GConnHttpEventResolved*)event,
					conn);
		case GNET_CONN_HTTP_RESPONSE:
			return _http_response((GConnHttpEventResponse*)event,
					conn);
		case GNET_CONN_HTTP_TIMEOUT:
			return _http_timeout(conn);
	}
}

static void _http_connected(GHtmlConn * conn)
{
	surfer_set_status(conn->ghtml->surfer, "Connected");
}

static void _http_data_complete(GConnHttpEventData * event, GHtmlConn * conn)
{
	gchar * buf;
	gsize size;

	if(gnet_conn_http_steal_buffer(conn->http, &buf, &size) != TRUE)
	{
		/* FIXME report error */
		surfer_set_progress(conn->ghtml->surfer, -1.0);
	}
	else
	{
		if(size > 0)
			html_stream_write(conn->stream, buf, size);
		surfer_set_progress(conn->ghtml->surfer, 1.0);
	}
	surfer_set_status(conn->ghtml->surfer, NULL);
	_ghtmlconn_delete(conn);
}

static void _http_data_partial(GConnHttpEventData * event, GHtmlConn * conn)
{
	gchar * buf;
	gsize size;
	gdouble fraction;

	surfer_set_status(conn->ghtml->surfer, "Downloading...");
	if(gnet_conn_http_steal_buffer(conn->http, &buf, &size) != TRUE)
	{
		/* FIXME report error */
		_ghtmlconn_delete(conn);
		return;
	}
	html_stream_write(conn->stream, buf, size);
	if(event->content_length > 0)
	{
		conn->data_received = event->data_received;
		conn->content_length = event->content_length;
		fraction = conn->data_received;
		surfer_set_progress(conn->ghtml->surfer,
				fraction / conn->content_length);
	}
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
			msg = "Unsupported protocol";
			break;
		case GNET_CONN_HTTP_ERROR_HOSTNAME_RESOLUTION:
			msg = "Unknown host";
			break;
		case GNET_CONN_HTTP_ERROR_UNSPECIFIED:
		default:
			msg = "Unspecified error";
			break;
	}
	surfer_set_progress(conn->ghtml->surfer, -1.0);
	surfer_set_status(conn->ghtml->surfer, NULL);
	surfer_error(conn->ghtml->surfer, msg, 0);
	_ghtmlconn_delete(conn);
}

static void _http_redirect(GConnHttpEventRedirect * event, GHtmlConn * conn)
{
	char buf[256] = "Redirecting...";
	char * url = event->new_location;

	if(url == NULL)
	{
		surfer_set_status(conn->ghtml->surfer, buf);
		return;
	}
	if((url = _ghtml_make_url(conn->ghtml->html_base, url)) != NULL)
	{
		snprintf(buf, sizeof(buf), "%s %s", "Redirecting to ", url);
		g_free(conn->url);
		conn->url = url;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() URL: %s, %d (%u/%u)\n", __func__, url,
			event->auto_redirect, event->num_redirects,
			event->max_redirects);
#endif
	_ghtml_set_base(conn->ghtml, url);
	surfer_set_location(conn->ghtml->surfer, url);
	surfer_set_status(conn->ghtml->surfer, buf);
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
#if 0
		surfer_set_progress(conn->ghtml->surfer, -1.0);
		surfer_set_status(conn->ghtml->surfer, NULL);
		surfer_error(conn->ghtml->surfer, "Unknown host", 0);
		_ghtmlconn_delete(conn);
#endif
	}
	else if((name = gnet_inetaddr_get_name_nonblock(event->ia)) == NULL)
		surfer_set_status(conn->ghtml->surfer, "Connecting...");
	else
	{
		snprintf(buf, sizeof(buf), "%s%s%s%d", "Connecting to ", name,
				":", gnet_inetaddr_get_port(event->ia));
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
	fprintf(stderr, "DEBUG: image found\n");
	snprintf(buf, sizeof(buf), "<img src=\"%s\">\n", conn->url);
	html_stream_write(conn->stream, buf, strlen(buf));
	_ghtmlconn_delete(conn);
}

static void _http_timeout(GHtmlConn * conn)
{
	surfer_error(conn->ghtml->surfer, "Timed out", 0);
	surfer_set_progress(conn->ghtml->surfer, -1.0);
	surfer_set_status(conn->ghtml->surfer, NULL);
	_ghtmlconn_delete(conn);
}


/* callbacks */
static void _on_link_clicked(HtmlDocument * document, const gchar * url)
{
	GHtml * ghtml;
	gchar * link;

	ghtml = g_object_get_data(G_OBJECT(document), "ghtml");
	if((link = _ghtml_make_url(ghtml->html_base, url)) == NULL)
		return; /* FIXME report error */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\") base=\"%s\" => \"%s\"\n", __func__,
			url, ghtml->html_base, link);
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
	if((link = _ghtml_make_url(ghtml->html_base, url)) == NULL)
		return; /* FIXME report error */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\") base=\"%s\" => \"%s\"\n", __func__,
			url, ghtml->html_base, link);
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
	u = _ghtml_make_url(ghtml->html_base, url);
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
	if((u = _ghtml_make_url(ghtml->html_base, url)) == NULL)
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
		surfer_error(ghtml->surfer, "Unsupported method", 0);
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
	if((link = _ghtml_make_url(ghtml->html_base, url)) != NULL)
	{
		surfer_set_status(ghtml->surfer, link);
		g_free(link);
	}
}
