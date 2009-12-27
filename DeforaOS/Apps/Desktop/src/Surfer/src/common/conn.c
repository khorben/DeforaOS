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



#include <string.h>
#define GNET_EXPERIMENTAL
#include <gnet.h>


/* Conn */
/* private */
/* types */
typedef struct _Conn Conn;

struct _Conn
{
	Surfer * surfer;

	char * url;
	char const * anchor;
	guint64 content_length;
	guint64 data_received;

	int direct;
	int image;

	/* callback */
	ssize_t (*callback_write)(Conn *, char const *, ssize_t, gpointer);
	gpointer callback_write_data;

	/* http */
	GConnHttp * http;
	GConnHttpMethod http_method;
	gchar * http_post;
};


/* prototypes */
static Conn * _conn_new(Surfer * surfer, char const * url,
		char const * post);
static void _conn_delete(Conn * conn);

/* accessors */
static void _conn_set_callback_write(Conn * conn,
		ssize_t (*callback)(Conn *, char const *, ssize_t, gpointer),
		gpointer data);

/* useful */
/* conn_load */
static int _conn_load(Conn * conn);


/* functions */
/* conn_new */
static ssize_t _new_callback_write(Conn * conn, char const * buf, ssize_t size,
		gpointer data);

static Conn * _conn_new(Surfer * surfer, char const * url, char const * post)
{
	Conn * conn;
	char * p;

	if((conn = malloc(sizeof(*conn))) == NULL)
		return NULL;
	conn->surfer = surfer;
	conn->url = strdup(url);
	conn->http_post = (post != NULL) ? strdup(post) : NULL;
	if(conn->url == NULL || (post != NULL && conn->http_post == NULL))
	{
		_conn_delete(conn);
		return NULL;
	}
	if((p = strrchr(conn->url, '#')) != NULL)
	{
		*(p++) = '\0';
		conn->anchor = p;
	}
	conn->content_length = 0;
	conn->data_received = 0;
	conn->direct = 0;
	conn->image = 0;
	conn->callback_write = _new_callback_write;
	conn->callback_write_data = stdout;
	conn->http = NULL;
	conn->http_method = (post == NULL) ? GNET_CONN_HTTP_METHOD_GET
		: GNET_CONN_HTTP_METHOD_POST;
	return conn;
}

static ssize_t _new_callback_write(Conn * conn, char const * buf, ssize_t size,
		gpointer data)
{
	ssize_t ret;

	if(size < 0)
		return -1;
	if((ret = fwrite(buf, sizeof(*buf), size, data)) != size)
		return -1;
	return ret;
}


/* conn_delete */
static void _conn_delete(Conn * conn)
{
	free(conn->url);
	free(conn->http_post);
	free(conn);
}


/* accessors */
static void _conn_set_callback_write(Conn * conn,
		ssize_t (*callback)(Conn *, char const *, ssize_t, gpointer),
		gpointer data)
{
	conn->callback_write = callback;
	conn->callback_write_data = data;
}


/* useful */
/* conn_load */
static void _load_watch_http(GConnHttp * connhttp, GConnHttpEvent * event,
		gpointer data);
static void _http_connected(Conn * conn);
static void _http_data_complete(GConnHttpEventData * event, Conn * conn);
static void _http_data_partial(GConnHttpEventData * event, Conn * conn);
static void _http_data_progress(GConnHttpEventData * event, Conn * conn);
static void _http_error(GConnHttpEventError * event, Conn * conn);
static void _http_redirect(GConnHttpEventRedirect * event, Conn * conn);
static void _http_resolved(GConnHttpEventResolved * event, Conn * conn);
static void _http_response(GConnHttpEventResponse * event, Conn * conn);
static void _http_timeout(Conn * conn);

static int _conn_load(Conn * conn)
{
	static const char http[] = "http:";

	if(strncmp(conn->url, http, sizeof(http) - 1) != 0)
	{
		/* FIXME support "file:", report error */
		return 1;
	}
	surfer_set_progress(conn->surfer, -1.0);
	surfer_set_status(conn->surfer, "Resolving...");
	conn->http = gnet_conn_http_new();
	gnet_conn_http_set_uri(conn->http, conn->url);
	gnet_conn_http_set_user_agent(conn->http, "DeforaOS " PACKAGE);
	gnet_conn_http_set_method(conn->http, conn->http_method,
			conn->http_post, (conn->http_post != NULL)
			? strlen(conn->http_post) : 0);
	gnet_conn_http_run_async(conn->http, _load_watch_http, conn);
	return 0;
}

static void _load_watch_http(GConnHttp * connhttp, GConnHttpEvent * event,
		gpointer data)
{
	Conn * conn = data;

	if(conn->http != connhttp)
		return; /* FIXME shouldn't happen, but report error */
	switch(event->type)
	{
		case GNET_CONN_HTTP_CONNECTED:
			_http_connected(conn);
			break;
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

static void _http_connected(Conn * conn)
{
	surfer_set_status(conn->surfer, "Connected");
}

static void _http_data_complete(GConnHttpEventData * event, Conn * conn)
{
	gchar * buf;
	gsize size;

	if(gnet_conn_http_steal_buffer(conn->http, &buf, &size) != TRUE)
	{
		/* FIXME report error */
		surfer_set_progress(conn->surfer, -1.0);
		surfer_set_status(conn->surfer, NULL);
		return;
	}
	conn->callback_write(conn, buf, size, conn->callback_write_data);
	_http_data_progress(event, conn);
	surfer_set_status(conn->surfer, NULL);
}

static void _http_data_partial(GConnHttpEventData * event, Conn * conn)
{
	gchar * buf;
	gsize size;

	surfer_set_status(conn->surfer, "Downloading...");
	if(gnet_conn_http_steal_buffer(conn->http, &buf, &size) != TRUE)
		/* FIXME report error */
		return;
	conn->callback_write(conn, buf, size, conn->callback_write_data);
	_http_data_progress(event, conn);
}

static void _http_data_progress(GConnHttpEventData * event, Conn * conn)
{
	gdouble fraction;

	conn->data_received = event->data_received;
	conn->content_length = event->content_length;
	fraction = conn->data_received;
	surfer_set_progress(conn->surfer, fraction / conn->content_length);
}

static void _http_error(GConnHttpEventError * event, Conn * conn)
{
	char * msg;

	switch(event->code)
	{
		case GNET_CONN_HTTP_ERROR_PROTOCOL_UNSUPPORTED:
			msg = "Unsupported protocol";
			break;
#if GNET_CHECK_VERSION(2, 0, 8) /* XXX unsure about the exact version */
		case GNET_CONN_HTTP_ERROR_HOSTNAME_RESOLUTION:
			msg = "Unknown host";
			break;
#endif
		case GNET_CONN_HTTP_ERROR_UNSPECIFIED:
		default:
			msg = "Unspecified error";
			break;
	}
	surfer_set_progress(conn->surfer, -1.0);
	surfer_set_status(conn->surfer, NULL);
	surfer_error(conn->surfer, msg, 0);
}

static void _http_redirect(GConnHttpEventRedirect * event, Conn * conn)
{
	char buf[256] = "Redirecting...";
	char * url = event->new_location;

	if(url == NULL)
	{
		surfer_set_status(conn->surfer, buf);
		return;
	}
	/* FIXME implement */
	surfer_set_status(conn->surfer, buf);
}

static void _http_resolved(GConnHttpEventResolved * event, Conn * conn)
{
	/* FIXME implement */
}

static void _http_response(GConnHttpEventResponse * event, Conn * conn)
{
	/* FIXME implement */
}

static void _http_timeout(Conn * conn)
{
	surfer_error(conn->surfer, "Timed out", 0);
	surfer_set_progress(conn->surfer, -1.0);
	surfer_set_status(conn->surfer, NULL);
}
