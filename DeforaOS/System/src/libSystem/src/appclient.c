/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System libSystem */
/* libSystem is not free software; you can redistribute it and/or modify it
 * under the terms of the Creative Commons Attribution-NonCommercial-ShareAlike
 * 3.0 Unported as published by the Creative Commons organization.
 *
 * libSystem is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with libSystem; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#ifdef WITH_SSL
# include <openssl/ssl.h>
#endif

#include "System.h"
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
	int buf_read_cnt;
	char buf_write[ASC_BUFSIZE];
	int buf_write_cnt;
	char const * lastfunc;
	void ** lastargs;
	int32_t * lastret;
#ifdef WITH_SSL
	SSL_CTX * ssl_ctx;
	SSL * ssl;
#endif
};


/* private */
static int _appclient_timeout(AppClient * appclient)
{
#ifdef DEBUG
	fprintf(stderr, "%s%d%s", "appclient_timeout(", appclient->fd, ")\n");
#endif
	errno = ETIMEDOUT;
	event_unregister_io_read(appclient->event, appclient->fd);
	event_unregister_io_write(appclient->event, appclient->fd);
	return 1;
}


/* appclient_read */
#ifdef WITH_SSL
# define READ(fd, ac, len) SSL_read(ac->ssl, &ac->buf_read[ac->buf_read_cnt], \
		len)
#else
# define READ(fd, ac, len) read(fd, &ac->buf_read[ac->buf_read_cnt], len)
#endif
static int _read_error();

static int _appclient_read(int fd, AppClient * ac)
{
	ssize_t len;

	if((len = (sizeof(ac->buf_read) - ac->buf_read_cnt)) < 0
			|| (len = READ(fd, ac, len)) <= 0)
		return _read_error(fd, ac);
	ac->buf_read_cnt += len;
#ifdef DEBUG
	fprintf(stderr, "%s%d%s%zd%s", "appclient_read(", fd, ") ", len,
			" bytes\n");
#endif
	len = appinterface_call_receive(ac->interface, ac->lastret,
			ac->buf_read, ac->buf_read_cnt, ac->lastfunc,
			ac->lastargs);
	if(len < 0 || len > ac->buf_read_cnt)
		return _read_error(fd, ac);
	if(len == 0) /* try again */
		return 0;
	ac->buf_read_cnt -= len;
	event_unregister_timeout(ac->event,
			(EventTimeoutFunc)_appclient_timeout);
	return 1;
}

static int _read_error(int fd, AppClient * ac)
{
	/* FIXME catch error */
#ifdef WITH_SSL
	SSL_shutdown(ac->ssl);
#endif
	close(fd);
	return 1;
}


/* appclient_write */
#ifdef WITH_SSL
# define WRITE(fd, ac, len) SSL_write(ac->ssl, ac->buf_write, len)
#else
# define WRITE(fd, ac, len) write(fd, ac->buf_write, len)
#endif
static int _appclient_write(int fd, AppClient * ac)
{
	ssize_t len;

	len = ac->buf_write_cnt;
#ifdef DEBUG
	fprintf(stderr, "%s%d%s%zd%s", "appclient_write(", fd, ") ", len,
			" bytes\n");
#endif
	if((len = WRITE(fd, ac, len)) <= 0)
	{
#ifdef WITH_SSL
		SSL_shutdown(ac->ssl);
#endif
		return 1;
	}
	memmove(ac->buf_write, &ac->buf_write[len], len);
	ac->buf_write_cnt-=len;
	if(ac->buf_write_cnt > 0)
		return 0; /* there is more to write */
	event_register_io_read(ac->event, fd, /* read the answer */
			(EventIOFunc)_appclient_read, ac);
	return 1;
}


/* public */
/* functions */
/* appclient_new */
AppClient * appclient_new(char * app)
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
static int _new_connect(AppClient * appclient, char * app);

AppClient * appclient_new_event(char * app, Event * event)
{
	AppClient * appclient;

#ifdef DEBUG
	fprintf(stderr, "%s%s%s", "appclient_new(\"", app, "\")\n");
#endif
	if((appclient = malloc(sizeof(AppClient))) == NULL)
		return NULL;
	if((appclient->interface = appinterface_new("Session")) == NULL)
	{
		free(appclient);
		return NULL;
	}
	appclient->event = event;
	appclient->buf_read_cnt = 0;
	appclient->buf_write_cnt = 0;
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
		return NULL;
	}
	return appclient;
}

static int _connect_addr(char * service, uint32_t * addr);
static int _new_connect(AppClient * appclient, char * app)
{
	struct sockaddr_in sa;
	int32_t port = -1;

	if((appclient->fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return 1;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(appinterface_get_port(appclient->interface));
	if(_connect_addr("Session", &sa.sin_addr.s_addr) != 0)
		return 1;
	if(connect(appclient->fd, (struct sockaddr *)&sa, sizeof(sa)) != 0
#ifdef WITH_SSL
			|| (appclient->ssl = SSL_new(appclient->ssl_ctx))
			== NULL
			|| SSL_set_fd(appclient->ssl, appclient->fd) != 1
#endif
			)
		return 1;
#ifdef WITH_SSL
	SSL_set_connect_state(appclient->ssl);
#endif
	if(appclient_call(appclient, &port, "port", app) != 0
			|| port < 0)
		return 1;
	if(port == 0)
		return 0;
#ifdef WITH_SSL
	SSL_shutdown(appclient->ssl);
	SSL_free(appclient->ssl);
	appclient->ssl = NULL;
#endif
	close(appclient->fd);
	appclient->fd = -1;
#ifdef DEBUG
	fprintf(stderr, "%s%d%s", "AppClient bouncing to port ", port, "\n");
#endif
	appinterface_delete(appclient->interface);
	if((appclient->interface = appinterface_new(app)) == NULL)
		return 1;
	if((appclient->fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return 1;
	if(_connect_addr(app, &sa.sin_addr.s_addr) != 0)
		return 1;
	sa.sin_port = htons(port);
	if(connect(appclient->fd, (struct sockaddr *)&sa, sizeof(sa)) != 0
#ifdef WITH_SSL
			|| (appclient->ssl = SSL_new(appclient->ssl_ctx))
			== NULL
			|| SSL_set_fd(appclient->ssl, appclient->fd) != 1
#endif
			)
		return 1;
#ifdef WITH_SSL
	SSL_set_connect_state(appclient->ssl);
#endif
	return 0;
}

static int _connect_addr(char * service, uint32_t * addr)
{
	char prefix[] = "APPSERVER_";
	int len = sizeof(prefix);
	char * env;
	char * server;
	struct hostent * he;

	if((env = malloc(len + string_length(service) + 1)) == NULL)
		return 1;
	sprintf(env, "%s%s", prefix, service);
	server = getenv(env);
	free(env);
	if(server == NULL)
	{
		*addr = htonl(INADDR_LOOPBACK);
		return 0;
	}
	if((he = gethostbyname(server)) == NULL)
		return 1;
	*addr = *((uint32_t*)he->h_addr);
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
	free(appclient);
}


/* useful */
/* appclient_call */
static int _call_event(AppClient * ac);

int appclient_call(AppClient * ac, int32_t * ret, char const * function, ...)
{
	void ** args = NULL;
	va_list arg;
	size_t left = sizeof(ac->buf_write) - ac->buf_write_cnt;
	int i;

#ifdef DEBUG
	fprintf(stderr, "%s%p%s", "appclient_call(), interface ", ac->interface,
			"\n");
#endif
	if((i = appinterface_get_args_count(ac->interface, function)) < 0)
		return -1;
	if(i > 0 && (args = calloc(sizeof(*args), i)) == NULL)
		return -1;
	va_start(arg, function);
	i = appinterface_call(ac->interface, &ac->buf_write[ac->buf_write_cnt],
			left, function, args, arg);
	va_end(arg);
	if(i <= 0 || i > left)
	{
		free(args);
		return -1;
	}
	ac->lastfunc = function; /* XXX safe for now because synchronous only */
	ac->lastargs = args;
	ac->lastret = ret;
	ac->buf_write_cnt += i;
	if(_call_event(ac) != 0)
	{
		free(ac->lastargs);
		return -1;
	}
	free(ac->lastargs);
	return 0;
}

static int _call_event(AppClient * ac)
	/* FIXME don't block processing of other events */
{
	Event * eventtmp;
	struct timeval tv = { 10, 0 };

	eventtmp = ac->event;
	ac->event = event_new();
	event_register_timeout(ac->event, tv,
			(EventTimeoutFunc)_appclient_timeout, ac);
	event_register_io_write(ac->event, ac->fd,
			(EventIOFunc)_appclient_write, ac);
#ifdef DEBUG
	fprintf(stderr, "%s", "AppClient looping in wait for answer\n");
#endif
	event_loop(ac->event);
	event_delete(ac->event);
	ac->event = eventtmp;
	return 0; /* FIXME catch errors */
}
