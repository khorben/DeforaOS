/* $Id$ */
/* Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org> */
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
#include <unistd.h>
#include <stdlib.h>
#ifdef DEBUG
# include <stdio.h>
#endif
#include <string.h>
#include <errno.h>
#ifdef __WIN32__
# include <Winsock2.h>
#else
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
#endif
#ifdef WITH_SSL
# include <openssl/ssl.h>
# include <openssl/err.h>
#endif
#include <System.h>
#include "System/App/appserver.h"
#include "appinterface.h"
#include "common.h"
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

	/* callbacks */
	ssize_t (*read)(struct _AppServerClient * asc, char * buffer,
			size_t count);
	ssize_t (*write)(struct _AppServerClient * asc, char const * buffer,
			size_t count);

#ifdef WITH_SSL

	/* ssl-specific */
	SSL * ssl;
#endif
} AppServerClient;


/* prototypes */
static AppServerClient * _appserverclient_new(int fd, uint32_t addr,
		uint16_t port
#ifdef WITH_SSL
		, SSL_CTX * ssl_ctx
#endif
		);
static void _appserverclient_delete(AppServerClient * appserverclient);


/* AppServer */
/* private */
/* types */
ARRAY(AppServerClient *, AppServerClient)
struct _AppServer
{
	AppInterface * interface;
	Event * event;
	int event_free;
#ifdef WITH_SSL
	SSL_CTX * ssl_ctx;
#endif
	AppServerClientArray * clients;
	AppServerClient * current;
};


/* prototypes */
static int _appserver_client_add(AppServer * appserver, AppServerClient * asc);
static int _appserver_client_remove(AppServer * appserver,
		AppServerClient * asc);

static int _appserver_accept(int fd, AppServer * appserver);
static int _appserver_read(int fd, AppServer * appserver);
static int _appserver_write(int fd, AppServer * appserver);
#ifdef WITH_SSL
static char const * _appserver_error_ssl(void);
#endif

/* callbacks */
static ssize_t _callback_read(AppServerClient * asc, char * buffer,
		size_t count);
static ssize_t _callback_write(AppServerClient * asc, char const * buffer,
		size_t count);
#ifdef WITH_SSL
static ssize_t _callback_read_ssl(AppServerClient * asc, char * buffer,
		size_t count);
static ssize_t _callback_write_ssl(AppServerClient * asc, char const * buffer,
		size_t count);
#endif


/* AppServerClient */
/* private */
/* functions */
/* appserverclient_new */
static AppServerClient * _appserverclient_new(int fd, uint32_t addr,
		uint16_t port
#ifdef WITH_SSL
		, SSL_CTX * ssl_ctx
#endif
		)
{
	AppServerClient * asc;

	if((asc = object_new(sizeof(*asc))) == NULL)
		return NULL;
	asc->state = ASCS_NEW;
	asc->addr = addr;
	asc->port = port;
	asc->buf_read_cnt = 0;
	asc->buf_write_cnt = 0;
	asc->read = _callback_read;
	asc->write = _callback_write;
#ifdef WITH_SSL
	if(addr != INADDR_LOOPBACK)
	{
		if((asc->ssl = SSL_new(ssl_ctx)) == NULL
				|| SSL_set_fd(asc->ssl, fd) != 1)
		{
			error_set_code(1, "%s", _appserver_error_ssl());
			_appserverclient_delete(asc);
			return NULL;
		}
		asc->read = _callback_read_ssl;
		asc->write = _callback_write_ssl;
		SSL_set_accept_state(asc->ssl);
	}
#endif
	asc->fd = fd;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d, %x, %u) => %p\n", __func__, fd, addr,
			port, (void *)asc);
#endif
	common_socket_set_nodelay(fd, 1);
	return asc;
}


/* appserverclient_delete */
static void _appserverclient_delete(AppServerClient * appserverclient)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p)\n", __func__, (void *)appserverclient);
#endif
#ifdef WITH_SSL
	if(appserverclient->ssl != NULL)
		SSL_free(appserverclient->ssl);
#endif
	if(appserverclient->fd != -1)
		close(appserverclient->fd);
	object_delete(appserverclient);
}


/* AppServer */
/* private */
/* functions */
/* appserver_client_add */
static int _appserver_client_add(AppServer * appserver, AppServerClient * asc)
	/* FIXME check for errors */
{
	array_append(appserver->clients, &asc);
	event_register_io_read(appserver->event, asc->fd,
			(EventIOFunc)_appserver_read, appserver);
	return 0;
}


/* appserver_client_remove */
static int _appserver_client_remove(AppServer * appserver,
		AppServerClient * asc)
	/* FIXME check for errors */
{
	AppServerClient * p;
	unsigned int i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, %p)\n", __func__, (void *)appserver,
			(void *)asc);
#endif
	for(i = 0; i < array_count(appserver->clients); i++)
	{
		if(array_get_copy(appserver->clients, i, &p) != 0)
			break;
		if(p == asc)
			break;
	}
	if(asc->buf_write_cnt > 0)
		event_unregister_io_write(appserver->event, asc->fd);
	event_unregister_io_read(appserver->event, asc->fd);
	_appserverclient_delete(asc);
	array_remove_pos(appserver->clients, i);
	return 0;
}


/* appserver_accept */
static int _appserver_accept(int fd, AppServer * appserver)
{
	struct sockaddr_in sa;
	socklen_t sa_size = sizeof(sa);
	int newfd;
	AppServerClient * asc;

	if((newfd = accept(fd, (struct sockaddr *)&sa, &sa_size)) == -1)
		return error_set_code(1, "%s%s", "accept: ", strerror(errno));
#ifdef DEBUG
	fprintf(stderr, "%s%d%s%d %s:%u\n", "DEBUG: accept(", fd, ") => ",
			newfd, inet_ntoa(sa.sin_addr), sa.sin_port);
#endif
	if((asc = _appserverclient_new(newfd, htonl(sa.sin_addr.s_addr),
					htons(sa.sin_port)
#ifdef WITH_SSL
					, appserver->ssl_ctx
#endif
					)) == NULL)
	{
#ifdef DEBUG
		error_print("DEBUG");
#endif
		close(newfd);
		return 0;
	}
	return _appserver_client_add(appserver, asc);
}


/* appserver_read */
static int _read_error(AppServer * appserver, AppServerClient * asc);
static int _read_eof(AppServer * appserver, AppServerClient * asc);
static int _read_process(AppServer * appserver, AppServerClient * asc);
static int _read_logged(AppServer * appserver, AppServerClient * asc);

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
	len = sizeof(asc->buf_read) - asc->buf_read_cnt;
	assert(len > 0 && asc->fd == fd);
	if((len = asc->read(asc, &asc->buf_read[asc->buf_read_cnt], len)) < 0)
		return _read_error(appserver, asc);
	else if(len == 0)
		return _read_eof(appserver, asc);
	asc->buf_read_cnt += len;
#ifdef DEBUG
	fprintf(stderr, "%s%d%s%ld%s", "DEBUG: read(", fd, ") => ", len, "\n");
#endif
	if(_read_process(appserver, asc) != 0)
	{
		_appserver_client_remove(appserver, asc);
		return 1;
	}
	return 0;
}

static int _read_error(AppServer * appserver, AppServerClient * asc)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, %p)\n", __func__, (void *)appserver,
			(void *)asc);
#endif
#ifdef WITH_SSL
	error_set_code(1, "%s", _appserver_error_ssl());
	SSL_shutdown(asc->ssl);
#else
	error_set_code(1, "%s", strerror(errno));
#endif
#ifdef DEBUG
	error_print("DEBUG");
#endif
	_appserver_client_remove(appserver, asc);
	return 1;
}

static int _read_eof(AppServer * appserver, AppServerClient * asc)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, %p)\n", __func__, (void *)appserver,
			(void *)asc);
#endif
	_appserver_client_remove(appserver, asc);
	return 1;
}

static int _read_process(AppServer * appserver, AppServerClient * asc)
{
	switch(asc->state)
	{
		case ASCS_NEW:
			/* FIXME authenticate */
			asc->state = ASCS_LOGGED;
		case ASCS_LOGGED:
			return _read_logged(appserver, asc);
	}
	return 1;
}

static int _read_logged(AppServer * appserver, AppServerClient * asc)
{
	ssize_t i;
	int32_t ret;

	appserver->current = asc;
	i = appinterface_receive(appserver->interface, &ret, asc->buf_read,
			asc->buf_read_cnt, asc->buf_write,
			sizeof(asc->buf_write), &asc->buf_write_cnt);
	appserver->current = NULL;
	if(i <= 0 || (size_t)i > asc->buf_read_cnt)
		return 1;
	asc->buf_read_cnt -= i;
	memmove(asc->buf_read, &asc->buf_read[i], asc->buf_read_cnt);
	/* FIXME should it be done in AppInterface? */
	if(asc->buf_write_cnt + sizeof(ret) > sizeof(asc->buf_write))
		return error_set_code(1, "%s", strerror(ENOBUFS));
	ret = htonl(ret);
	memcpy(&(asc->buf_write[asc->buf_write_cnt]), &ret, sizeof(ret));
	asc->buf_write_cnt += sizeof(ret);
	event_register_io_write(appserver->event, asc->fd,
			(EventIOFunc)_appserver_write, appserver);
	return (asc->fd != -1) ? 0 : 1;
}


/* appserver_write */
static int _write_error(AppServerClient * asc);

static int _appserver_write(int fd, AppServer * appserver)
{
	AppServerClient * asc;
	ssize_t len;
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
	if((len = asc->write(asc, asc->buf_write, asc->buf_write_cnt)) <= 0)
		return _write_error(asc);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: write(%d, %lu) => %ld\n", fd,
			asc->buf_write_cnt, len);
#endif
	memmove(asc->buf_write, &asc->buf_write[len], len);
	asc->buf_write_cnt-=len;
	return (asc->buf_write_cnt == 0) ? 1 : 0;
}

static int _write_error(AppServerClient * asc)
{
#ifdef WITH_SSL
	error_set_code(1, "%s", _appserver_error_ssl());
#else
	error_set_code(1, "%s", strerror(errno));
#endif
	close(asc->fd);
	asc->fd = -1;
	return 1;
}


#ifdef WITH_SSL
/* appserver_error_ssl */
static char const * _appserver_error_ssl(void)
{
	return ERR_error_string(ERR_get_error(), NULL);
}
#endif


/* callbacks */
/* callback_read */
static ssize_t _callback_read(AppServerClient * asc, char * buffer,
		size_t count)
{
	return read(asc->fd, buffer, count);
}


/* callback_write */
static ssize_t _callback_write(AppServerClient * asc, char const * buffer,
		size_t count)
{
	return write(asc->fd, buffer, count);
}


#ifdef WITH_SSL
/* callback_read_ssl */
static ssize_t _callback_read_ssl(AppServerClient * asc, char * buffer,
		size_t count)
{
	return SSL_read(asc->ssl, buffer, count);
}


/* callback_write_ssl */
static ssize_t _callback_write_ssl(AppServerClient * asc, char const * buffer,
		size_t count)
{
	return SSL_write(asc->ssl, buffer, count);
}
#endif


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
static int _new_server(AppServer * appserver, char const * app, int options);

AppServer * appserver_new_event(char const * app, int options, Event * event)
{
	AppServer * appserver;
#ifdef WITH_SSL
	char crt[256];

	if(snprintf(crt, sizeof(crt), "%s%s%s", PREFIX "/etc/AppServer/", app,
				".crt") >= (int)sizeof(crt))
		return NULL;
#endif
	if((appserver = object_new(sizeof(*appserver))) == NULL)
		return NULL;
	appserver->interface = NULL;
	appserver->event = event;
	appserver->event_free = 0;
#ifdef WITH_SSL
	appserver->ssl_ctx = NULL;
#endif
	if((appserver->clients = AppServerClientarray_new()) == NULL
			|| _new_server(appserver, app, options) != 0)
	{
		appserver_delete(appserver);
		return NULL;
	}
#ifdef WITH_SSL
# ifdef DEBUG
	fprintf(stderr, "DEBUG: Using certificate \"%s\"\n", crt);
# endif
	if(!(options & ASO_LOCAL))
		if((appserver->ssl_ctx = SSL_CTX_new(SSLv3_server_method())) == NULL
				|| SSL_CTX_set_cipher_list(appserver->ssl_ctx,
					SSL_DEFAULT_CIPHER_LIST) != 1
				|| SSL_CTX_use_certificate_file(
					appserver->ssl_ctx, crt,
					SSL_FILETYPE_PEM) == 0
				|| SSL_CTX_use_PrivateKey_file(
					appserver->ssl_ctx, crt,
					SSL_FILETYPE_PEM) == 0)
		{
			error_set_code(1, "%s", _appserver_error_ssl());
			appserver_delete(appserver);
			return NULL;
		}
#endif
	appserver->current = NULL;
	return appserver;
}

static int _new_server(AppServer * appserver, char const * app, int options)
{
	int fd;
	struct sockaddr_in sa;

	if((appserver->interface = appinterface_new_server(app)) == NULL)
		return -1;
	if((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return -error_set_code(1, "%s%s", "socket: ", strerror(errno));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(appinterface_get_port(appserver->interface));
	sa.sin_addr.s_addr = htonl(options & ASO_LOCAL ? INADDR_LOOPBACK
			: INADDR_ANY);
	if(bind(fd, (struct sockaddr *)&sa, sizeof(sa)) == 0)
	{
		if(listen(fd, 5) == 0)
		{
			event_register_io_read(appserver->event, fd,
					(EventIOFunc)_appserver_accept,
					appserver);
			return 0;
		}
		error_set_code(1, "%s%s", "listen: ", strerror(errno));
	}
	else
		error_set_code(1, "%s%s", "bind: ", strerror(errno));
	close(fd);
	return -1;
}


/* appserver_delete */
void appserver_delete(AppServer * appserver)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(appserver->interface != NULL)
		appinterface_delete(appserver->interface);
	if(appserver->event_free != 0)
		event_delete(appserver->event);
	array_delete(appserver->clients);
#ifdef WITH_SSL
	if(appserver->ssl_ctx != NULL)
		SSL_CTX_free(appserver->ssl_ctx);
#endif
	object_delete(appserver);
}


/* accessors */
/* appserver_get_client_id */
void * appserver_get_client_id(AppServer * appserver)
{
	return appserver->current;
}


/* useful */
/* appserver_loop */
int appserver_loop(AppServer * appserver)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	return event_loop(appserver->event);
}
