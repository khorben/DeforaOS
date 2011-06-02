/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System libApp */
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



#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#ifdef __WIN32__
# include <Winsock2.h>
#else
# include <netinet/in.h>
# include <netdb.h>
# include <arpa/inet.h>
#endif
#ifdef WITH_SSL
# include <openssl/ssl.h>
# include <openssl/err.h>
#endif
#include <System.h>
#include "System/App.h"
#include "appinterface.h"


/* AppClient */
/* private */
/* types */
struct _AppClient
{
	AppInterface * interface;
	Event * event;
	int fd;
#define ASC_BUFSIZE 65536 /* FIXME */
	char buf_read[ASC_BUFSIZE];
	size_t buf_read_cnt;
	char buf_write[ASC_BUFSIZE];
	size_t buf_write_cnt;

	/* current call */
	char const * lastfunc;
	void ** lastargs;
	int32_t * lastret;

	/* callbacks */
	int (*error)(AppClient * appclient);
	ssize_t (*read)(AppClient * appclient, char * buffer, size_t count);
	ssize_t (*write)(AppClient * appclient, char const * buffer,
			size_t count);
#ifdef WITH_SSL

	/* ssl-specific */
	SSL_CTX * ssl_ctx;
	SSL * ssl;
#endif
};


/* private */
/* prototypes */
static int _appclient_timeout(AppClient * appclient);
static int _appclient_read(int fd, AppClient * ac);
static int _appclient_write(int fd, AppClient * ac);

/* callbacks */
static ssize_t _callback_read(AppClient * appclient, char * buffer,
		size_t count);
static ssize_t _callback_write(AppClient * appclient, char const * buffer,
		size_t count);
#ifdef WITH_SSL
static int _callback_error_ssl(AppClient * appclient);
static ssize_t _callback_read_ssl(AppClient * appclient, char * buffer,
		size_t count);
static ssize_t _callback_write_ssl(AppClient * appclient, char const * buffer,
		size_t count);
#endif


/* functions */
/* appclient_timeout */
static int _appclient_timeout(AppClient * appclient)
{
#ifdef DEBUG
	fprintf(stderr, "%s%d%s", "DEBUG: timeout(", appclient->fd, ")\n");
#endif
	event_unregister_io_read(appclient->event, appclient->fd);
	event_unregister_io_write(appclient->event, appclient->fd);
	errno = ETIMEDOUT;
	return error_set_code(1, "%s", strerror(errno));
}


/* appclient_read */
static int _read_error(AppClient * ac);
static int _read_unregister(AppClient * appclient);

static int _appclient_read(int fd, AppClient * ac)
{
	ssize_t len = sizeof(ac->buf_read) - ac->buf_read_cnt;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d, %p)\n", __func__, fd, (void*)ac);
#endif
	assert(len >= 0 && ac->fd == fd);
	if((len = ac->read(ac, &ac->buf_read[ac->buf_read_cnt], len)) <= 0)
		return _read_error(ac);
#ifdef DEBUG
	fprintf(stderr, "%s%d%s%zd%s", "DEBUG: read(", fd, ") => ", len, "\n");
#endif
	ac->buf_read_cnt += len;
	len = appinterface_call_receive(ac->interface, ac->lastret,
			ac->buf_read, ac->buf_read_cnt, ac->lastfunc,
			ac->lastargs);
	if(len < 0)
	{
#ifdef WITH_SSL
		SSL_shutdown(ac->ssl);
#endif
		close(ac->fd);
		ac->fd = -1;
		return _read_unregister(ac);
	}
	else if(len == 0) /* FIXME the buffer may actually be too small */
		return 0;
	assert((size_t)len <= ac->buf_read_cnt);
	ac->buf_read_cnt -= len;
	return _read_unregister(ac);
}

static int _read_error(AppClient * ac)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p)\n", __func__, (void*)ac);
#endif
	ac->error(ac);
#ifdef DEBUG
	error_print("DEBUG");
#endif
	close(ac->fd);
	/* FIXME properly shutdown connection
	 * (eg avoid failed assertion on clear connect to SSL */
	ac->fd = -1;
	return _read_unregister(ac);
}

static int _read_unregister(AppClient * appclient)
{
	event_unregister_timeout(appclient->event,
			(EventTimeoutFunc)_appclient_timeout);
	return 1;
}


/* appclient_write */
static int _write_error(AppClient * ac);

static int _appclient_write(int fd, AppClient * ac)
{
	ssize_t len;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d, %p)\n", __func__, fd, (void*)ac);
#endif
	/* FIXME is EOF an error? */
	if((len = ac->write(ac, ac->buf_write, ac->buf_write_cnt)) <= 0)
		return _write_error(ac);
#ifdef DEBUG
	fprintf(stderr, "%s%d%s%zu%s%zd%s", "DEBUG: write(", fd, ", ",
			ac->buf_write_cnt, ") ", len, "\n");
#endif
	memmove(ac->buf_write, &ac->buf_write[len], len);
	ac->buf_write_cnt -= len;
	if(ac->buf_write_cnt > 0)
		return 0; /* there is more to write */
	/* read the answer */
	event_register_io_read(ac->event, fd,
			(EventIOFunc)_appclient_read, ac);
	return 1;
}

static int _write_error(AppClient * ac)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p)\n", __func__, (void*)ac);
#endif
	ac->error(ac);
#ifdef DEBUG
	error_print("DEBUG");
#endif
	close(ac->fd);
	ac->fd = -1;
	return 1;
}


/* callbacks */
/* callback_error */
static int _callback_error(AppClient * appclient)
{
	return error_set_code(1, "%s", strerror(errno));
}


/* callback_read */
static ssize_t _callback_read(AppClient * appclient, char * buffer,
		size_t count)
{
	return read(appclient->fd, buffer, count);
}


/* callback_write */
static ssize_t _callback_write(AppClient * appclient, char const * buffer,
		size_t count)
{
	return write(appclient->fd, buffer, count);
}


#ifdef WITH_SSL
/* callback_error_ssl */
static int _callback_error_ssl(AppClient * appclient)
{
	error_set_code(1, "%s", ERR_error_string(ERR_get_error(), NULL));
	SSL_shutdown(appclient->ssl);
	return 1;
}


/* callback_read_ssl */
static ssize_t _callback_read_ssl(AppClient * appclient, char * buffer,
		size_t count)
{
	return SSL_read(appclient->ssl, buffer, count);
}


/* callback_write_ssl */
static ssize_t _callback_write_ssl(AppClient * appclient, char const * buffer,
		size_t count)
{
	return SSL_write(appclient->ssl, buffer, count);
}
#endif


/* public */
/* functions */
/* appclient_new */
AppClient * appclient_new(char const * app)
{
	AppClient * appclient;
	Event * event;

	if((event = event_new()) == NULL)
		return NULL;
	if((appclient = appclient_new_event(app, event)) == NULL)
	{
		event_delete(event);
		return NULL;
	}
	return appclient;
}


/* appclient_new_event */
static int _new_connect(AppClient * appclient, char const * app);
static int _connect_addr(String const * service, uint32_t * addr);

AppClient * appclient_new_event(char const * app, Event * event)
{
	AppClient * appclient;

#ifdef DEBUG
	fprintf(stderr, "%s%s%s%s", __func__, "(\"", app, "\")\n");
#endif
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", %p)\n", __func__, app, (void*)event);
#endif
	if((appclient = object_new(sizeof(AppClient))) == NULL)
		return NULL;
	if((appclient->interface = appinterface_new("Init")) == NULL)
	{
		object_delete(appclient);
		return NULL;
	}
	appclient->event = event;
	appclient->buf_read_cnt = 0;
	appclient->buf_write_cnt = 0;
	appclient->error = _callback_error;
	appclient->read = _callback_read;
	appclient->write = _callback_write;
#ifdef WITH_SSL
	appclient->ssl = NULL;
	if((appclient->ssl_ctx = SSL_CTX_new(SSLv3_client_method())) == NULL
			|| SSL_CTX_set_cipher_list(appclient->ssl_ctx,
				SSL_DEFAULT_CIPHER_LIST) != 1
			|| _new_connect(appclient, app) != 0)
#else
	if(_new_connect(appclient, app) != 0)
#endif
	{
		appclient_delete(appclient);
		appclient = NULL;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", %p) => %p\n", __func__, app,
			(void*)event, (void*)appclient);
#endif
	return appclient;
}

static int _new_connect(AppClient * appclient, char const * app)
{
	const char init[] = "Init";
	struct sockaddr_in sa;
	int32_t port = -1;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, \"%s\")\n", __func__, (void*)appclient,
			app);
#endif
	if((appclient->fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return 1;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(appinterface_get_port(appclient->interface));
	if(_connect_addr(init, &sa.sin_addr.s_addr) != 0)
		return 1;
	if(connect(appclient->fd, (struct sockaddr *)&sa, sizeof(sa)) != 0)
		return error_set_code(1, "%s%s%s", init, ": ", strerror(errno));
#ifdef DEBUG
	fprintf(stderr, "DEBUG: connect(%d, %s:%d) => 0\n", appclient->fd,
			inet_ntoa(sa.sin_addr), ntohs(sa.sin_port));
#endif
#ifdef WITH_SSL
	if(sa.sin_addr.s_addr != INADDR_LOOPBACK)
	{
		if((appclient->ssl = SSL_new(appclient->ssl_ctx)) == NULL
				|| SSL_set_fd(appclient->ssl, appclient->fd)
				!= 1)
			return error_set_code(1, "%s", ERR_error_string(
						ERR_get_error(), NULL));
		appclient->error = _callback_error_ssl;
		appclient->read = _callback_read_ssl;
		appclient->write = _callback_write_ssl;
		SSL_set_connect_state(appclient->ssl);
	}
#endif
	if(appclient_call(appclient, &port, "get_session", app) != 0)
		return 1;
	if(port < 0)
		return error_set_code(1, "%s", "Could not obtain remote port");
	if(port == 0) /* the connection is good already or being forwarded */
		return 0;
#ifdef WITH_SSL
	if(appclient->ssl != NULL)
	{
		SSL_shutdown(appclient->ssl);
		SSL_free(appclient->ssl);
		appclient->ssl = NULL;
	}
#endif
#ifdef DEBUG
	fprintf(stderr, "%s%d%s", "DEBUG: Bouncing to port ", port, "\n");
#endif
	appinterface_delete(appclient->interface);
	if((appclient->interface = appinterface_new(app)) == NULL)
		return 1;
	close(appclient->fd);
	if((appclient->fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return error_set_code(1, "%s%s", "socket: ", strerror(errno));
	if(_connect_addr(app, &sa.sin_addr.s_addr) != 0)
		return 1;
	sa.sin_port = htons(port);
	if(connect(appclient->fd, (struct sockaddr *)&sa, sizeof(sa)) != 0)
		return error_set_code(1, "%s%s%s", app, ": ", strerror(errno));
#ifdef DEBUG
	fprintf(stderr, "DEBUG: connect(%d, %s:%d) => 0\n", appclient->fd,
			inet_ntoa(sa.sin_addr), ntohs(sa.sin_port));
#endif
#ifdef WITH_SSL
	appclient->read = _callback_read;
	appclient->write = _callback_write;
	if(sa.sin_addr.s_addr != INADDR_LOOPBACK)
	{
		if((appclient->ssl = SSL_new(appclient->ssl_ctx)) == NULL
				|| SSL_set_fd(appclient->ssl, appclient->fd)
				!= 1)
			return error_set_code(1, "%s", ERR_error_string(
						ERR_get_error(), NULL));
		appclient->read = _callback_read_ssl;
		appclient->write = _callback_write_ssl;
		SSL_set_connect_state(appclient->ssl);
	}
#endif
	return 0;
}

static int _connect_addr(String const * service, uint32_t * addr)
{
	const char prefix[] = "APPSERVER_";
	size_t len;
	char * env;
	char const * server;
	struct hostent * he;

	len = sizeof(prefix) + string_length(service) + 1;
	if((env = malloc(len)) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	snprintf(env, len, "%s%s", prefix, service);
	server = getenv(env);
	free(env);
	if(server == NULL)
	{
		*addr = htonl(INADDR_LOOPBACK);
		return 0;
	}
	if((he = gethostbyname(server)) == NULL)
		return error_set_code(1, "%s", hstrerror(h_errno));
	/* FIXME use memcpy()? check he->addrtype and he->h_length? */
	*addr = *((uint32_t*)he->h_addr_list[0]);
	return 0;
}


/* appclient_delete */
void appclient_delete(AppClient * appclient)
{
	appinterface_delete(appclient->interface);
	if(appclient->fd != -1)
		close(appclient->fd);
#ifdef WITH_SSL
	if(appclient->ssl != NULL)
		SSL_free(appclient->ssl);
	if(appclient->ssl_ctx != NULL)
		SSL_CTX_free(appclient->ssl_ctx);
#endif
	object_delete(appclient);
}


/* useful */
/* appclient_call */
static int _call_event(AppClient * ac);

int appclient_call(AppClient * ac, int32_t * ret, char const * function, ...)
{
	void ** args = NULL;
	va_list arg;
	size_t left;
	size_t cnt;
	ssize_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, %p, \"%s\", ...)\n", __func__, (void*)ac,
			(void*)ret, function);
#endif
	if(appinterface_get_args_count(ac->interface, &cnt, function) != 0)
		return 1;
	if((args = calloc(sizeof(*args), cnt)) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	assert(sizeof(ac->buf_write) >= ac->buf_write_cnt);
	left = sizeof(ac->buf_write) - ac->buf_write_cnt;
	va_start(arg, function);
	i = appinterface_call(ac->interface, &ac->buf_write[ac->buf_write_cnt],
			left, function, args, arg);
	va_end(arg);
	if(i <= 0)
	{
		free(args);
		return 1;
	}
	assert((size_t)i <= left);
	ac->lastfunc = function; /* XXX safe for now because synchronous only */
	ac->lastargs = args;
	ac->lastret = ret;
	ac->buf_write_cnt += i;
	i = _call_event(ac);
	free(args);
	return (i == 0) ? 0 : 1;
}

static int _call_event(AppClient * ac)
	/* FIXME don't block processing of other events */
{
	Event * eventtmp;
	struct timeval tv = { 10, 0 };

	eventtmp = ac->event;
	ac->event = event_new();
	event_register_timeout(ac->event, &tv,
			(EventTimeoutFunc)_appclient_timeout, ac);
	event_register_io_write(ac->event, ac->fd,
			(EventIOFunc)_appclient_write, ac);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() %s\n", __func__, "Waiting for answer");
#endif
	event_loop(ac->event);
	event_delete(ac->event); /* FIXME may already be free'd */
	ac->event = eventtmp;
	return (ac->fd >= 0) ? 0 : 1;
}
