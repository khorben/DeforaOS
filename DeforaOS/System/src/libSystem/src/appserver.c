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
#include <stdlib.h>
#ifdef DEBUG
# include <stdio.h>
#endif
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#ifdef WITH_SSL
# include <openssl/ssl.h>
#endif

#include "System.h"
#include "appinterface.h"
#include "../config.h"


/* AppServerClient */
/* private */
/* types */
typedef enum _AppServerClientState
{
	ASCS_NEW,
	ASCS_LOGGED
} AppServerClientState;

typedef struct _AppServerClient
{
	AppServerClientState state;
	int fd;
	uint32_t addr; /* FIXME uint8_t[4] instead? */
	uint16_t port;
#define ASC_BUFSIZE 65536 /* FIXME */
	char buf_read[ASC_BUFSIZE];
	size_t buf_read_cnt;
	char buf_write[ASC_BUFSIZE];
	size_t buf_write_cnt;
#ifdef WITH_SSL
	SSL * ssl;
#endif
} AppServerClient;


/* functions */
/* _appserverclient_new */
static void _appserverclient_delete(AppServerClient * appserverclient);

static AppServerClient * _appserverclient_new(int fd, uint32_t addr,
		uint16_t port
#ifdef WITH_SSL
		, SSL_CTX * ssl_ctx
#endif
		)
{
	AppServerClient * asc;

	if((asc = malloc(sizeof(AppServerClient))) == NULL)
	{
		error_set_code(1, "%s", strerror(errno));
		return NULL;
	}
	asc->state = ASCS_NEW;
	asc->fd = -1;
	asc->addr = addr;
	asc->port = port;
	asc->buf_read_cnt = 0;
	asc->buf_write_cnt = 0;
#ifdef WITH_SSL
	if((asc->ssl = SSL_new(ssl_ctx)) == NULL
			|| SSL_set_fd(asc->ssl, fd) != 1)
	{
		return error_set_code(1, "%s", ERR_error_string(ERR_get_error(),
					NULL));
		_appserverclient_delete(asc);
		return NULL;
	}
	SSL_set_accept_state(asc->ssl);
#endif
	asc->fd = fd;
	return asc;
}


/* _appserverclient_delete */
static void _appserverclient_delete(AppServerClient * appserverclient)
{
#ifdef WITH_SSL
	if(appserverclient->ssl != NULL)
		SSL_free(appserverclient->ssl);
#endif
	if(appserverclient->fd != -1)
		close(appserverclient->fd);
	free(appserverclient);
}


/* AppServer */
/* private */
/* types */
ARRAY(AppServerClient *, AppServerClient);
struct _AppServer
{
	AppInterface * interface;
	Event * event;
	int event_free;
#ifdef WITH_SSL
	SSL_CTX * ssl_ctx;
#endif
	AppServerClientArray * clients;
};


/* functions */
static int _appserver_accept(int fd, AppServer * appserver);
static int _appserver_read(int fd, AppServer * appserver);
static int _appserver_write(int fd, AppServer * appserver);

/* appserver_accept */
static int _appserver_accept(int fd, AppServer * appserver)
{
	struct sockaddr_in sa;
	socklen_t sa_size = sizeof(struct sockaddr_in);
	int newfd;
	AppServerClient * asc;

#ifdef DEBUG
	fprintf(stderr, "%s%d%s%p%s", "_appserver_accept(", fd, ", ", appserver,
			")\n");
#endif
	if((newfd = accept(fd, (struct sockaddr *)&sa, &sa_size)) == -1)
		return error_set_code(1, "%s%s", "accept: ", strerror(errno));
	if((asc = _appserverclient_new(newfd, sa.sin_addr.s_addr, sa.sin_port
#ifdef WITH_SSL
					, appserver->ssl_ctx
#endif
					)) == NULL)
	{
		close(newfd);
		return 0;
	}
	array_append(appserver->clients, &asc);
	event_register_io_read(appserver->event, newfd,
			(EventIOFunc)_appserver_read, appserver);
	return 0;
}


/* appserver_read */
#ifdef WITH_SSL
# define READ(fd, asc, len) SSL_read(asc->ssl, \
		&asc->buf_read[asc->buf_read_cnt], len)
#else
# define READ(fd, asc, len) read(fd, &asc->buf_read[asc->buf_read_cnt], len)
#endif
static int _read_process(AppServer * appserver, AppServerClient * asc);

static int _appserver_read(int fd, AppServer * appserver)
{
	AppServerClient * asc = NULL;
	unsigned int i;
	ssize_t len;

	for(i = 0; i < array_count(appserver->clients); i++)
	{
		if(array_get_copy(appserver->clients, i, &asc))
			break;
		if(fd == asc->fd)
			break;
		asc = NULL;
	}
	if(asc == NULL)
		return 1;
	if((len = sizeof(asc->buf_read) - asc->buf_read_cnt) <= 0
			|| (len = READ(fd, asc, len)) <= 0)
		/* FIXME the error is not necessarily because of READ */
	{
#ifdef WITH_SSL
		error_set_code(1, "%s", ERR_error_string(ERR_get_error(),
					NULL));
		SSL_shutdown(asc->ssl);
#else
		error_set_code(1, "%s%s", "read: ", strerror(errno));
#endif
		/* FIXME do all this in appserverclient_delete() or something
		 * like appserver_remove_client() */
		if(asc->buf_write_cnt > 0)
			event_unregister_io_write(appserver->event, fd);
		event_unregister_io_read(appserver->event, fd);
		_appserverclient_delete(asc);
		array_remove_pos(appserver->clients, i);
		return 1;
	}
	asc->buf_read_cnt+=len;
#ifdef DEBUG
	fprintf(stderr, "%s%d%s%zd%s", "_appserver_read(", fd,
			", appserver): ", len, " characters read\n");
#endif
	return _read_process(appserver, asc);
}

static int _read_logged(AppServer * appserver, AppServerClient * asc);
static int _read_process(AppServer * appserver, AppServerClient * asc)
{
	switch(asc->state)
	{
		case ASCS_NEW:
			/* FIXME authenticate */
		case ASCS_LOGGED:
			return _read_logged(appserver, asc);
	}
	return 1;
}

static int _appserver_receive(AppServer * appserver, AppServerClient * asc);

static int _read_logged(AppServer * appserver, AppServerClient * asc)
{
	if(_appserver_receive(appserver, asc) != 0)
	{
		close(asc->fd);
		return 1;
	}
	return 0;
}

static int _appserver_receive(AppServer * appserver, AppServerClient * asc)
{
	int i;
	int32_t ret;

	if((i = appinterface_receive(appserver->interface, &ret, asc->buf_read,
			asc->buf_read_cnt, asc->buf_write,
			sizeof(asc->buf_write), &asc->buf_write_cnt)) == -1)
		return -1;
	if(i <= 0 || i > asc->buf_read_cnt)
		return -1;
	memmove(asc->buf_read, &asc->buf_read[i], asc->buf_read_cnt-i);
	asc->buf_read_cnt-=i;
	/* FIXME should be done in AppInterface? */
	if(asc->buf_write_cnt + sizeof(ret) > sizeof(asc->buf_write))
		return -1;
	ret = htonl(ret);
	memcpy(&(asc->buf_write[asc->buf_write_cnt]), &ret, sizeof(ret));
	asc->buf_write_cnt += sizeof(ret);
	event_register_io_write(appserver->event, asc->fd,
			(EventIOFunc)_appserver_write, appserver);
	return 0;
}


/* appserver_write */
#ifdef WITH_SSL
# define WRITE(fd, asc) SSL_write(asc->ssl, asc->buf_write, asc->buf_write_cnt)
#else
# define WRITE(fd, asc) write(fd, asc->buf_write, asc->buf_write_cnt)
#endif

static int _appserver_write(int fd, AppServer * appserver)
{
	AppServerClient * asc;
	size_t len;
	unsigned int i;

	/* FIXME factorize this code */
	for(i = 0; i < array_count(appserver->clients); i++)
	{
		if(array_get_copy(appserver->clients, i, &asc))
			break;
		if(fd == asc->fd)
			break;
		asc = NULL;
	}
	if(asc == NULL)
		return 1;
#ifdef DEBUG
	fprintf(stderr, "sending result: %zu long\n", asc->buf_write_cnt);
#endif
	if(asc->buf_write_cnt == 0 || (len = WRITE(fd, asc)) <= 0)
		return 1; /* FIXME catch and handle error */
	memmove(asc->buf_write, &asc->buf_write[len], len);
	asc->buf_write_cnt-=len;
#ifdef DEBUG
	fprintf(stderr, "%s%d%s%zu%s", "_appserver_write_int(", fd,
			", appserver): ", len, " characters written\n");
#endif
	return asc->buf_write_cnt == 0 ? 1 : 0;
}


/* public */
/* functions */
/* appserver_new */
AppServer * appserver_new(const char * app, int options)
{
	AppServer * appserver;
	Event * event;

	if((event = event_new()) == NULL)
		return NULL;
	if((appserver = appserver_new_event(app, options, event)) == NULL)
	{
		event_delete(event);
		return NULL;
	}
	appserver->event_free = 1;
	return appserver;
}


/* appserver_new_event */
static int _new_server(AppServer * appserver, int options);

AppServer * appserver_new_event(char const * app, int options, Event * event)
{
	AppServer * appserver;
#ifdef WITH_SSL
	char crt[256];

	if(snprintf(crt, sizeof(crt), "%s%s%s", PREFIX "/etc/AppServer/", app,
				".crt") >= sizeof(crt))
		return NULL;
#endif
	if((appserver = malloc(sizeof(AppServer))) == NULL)
	{
		error_set_code(1, "%s", strerror(errno));
		return NULL;
	}
	appserver->interface = NULL;
	appserver->event = event;
	appserver->event_free = 0;
#ifdef WITH_SSL
	appserver->ssl_ctx = NULL;
#endif
	if((appserver->clients = AppServerClientarray_new()) == NULL
			|| (appserver->interface = appinterface_new_server(app))
			== NULL
			|| _new_server(appserver, options) != 0)
	{
		appserver_delete(appserver);
		return NULL;
	}
#ifdef WITH_SSL
	if((appserver->ssl_ctx = SSL_CTX_new(SSLv3_server_method())) == NULL
			|| SSL_CTX_set_cipher_list(appserver->ssl_ctx,
				SSL_DEFAULT_CIPHER_LIST) != 1
			|| SSL_CTX_use_certificate_file(appserver->ssl_ctx, crt,
				SSL_FILETYPE_PEM) == 0
			|| SSL_CTX_use_PrivateKey_file(appserver->ssl_ctx, crt,
				SSL_FILETYPE_PEM) == 0)
	{
		error_set_code(1, "%s", ERR_error_string(ERR_get_error(),
					NULL));
		appserver_delete(appserver);
		return NULL;
	}
#endif
	return appserver;
}

static int _new_server(AppServer * appserver, int options)
{
	int fd;
	struct sockaddr_in sa;

	if((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return error_set_code(1, "%s%s", "socket: ", strerror(errno));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(appinterface_get_port(appserver->interface));
	sa.sin_addr.s_addr = htonl(options & ASO_LOCAL ? INADDR_LOOPBACK
			: INADDR_ANY);
	if(bind(fd, (struct sockaddr *)&sa, sizeof(sa)) != 0
			|| listen(fd, 5) != 0)
	{
		error_set_code(1, "%s%s", "bind: ", strerror(errno));
		close(fd);
		return 1;
	}
	event_register_io_read(appserver->event, fd,
			(EventIOFunc)_appserver_accept, appserver);
	return 0;
}


/* appserver_delete */
void appserver_delete(AppServer * appserver)
{
	if(appserver->interface != NULL)
		appinterface_delete(appserver->interface);
	if(appserver->event_free != 0)
		event_delete(appserver->event);
	array_delete(appserver->clients);
#ifdef WITH_SSL
	if(appserver->ssl_ctx != NULL)
		SSL_CTX_free(appserver->ssl_ctx);
#endif
	free(appserver);
}


/* useful */
int appserver_loop(AppServer * appserver)
{
	return event_loop(appserver->event);
}
