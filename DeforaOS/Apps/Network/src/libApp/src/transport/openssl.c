/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
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



#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <System.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "App/apptransport.h"
#include "../../config.h"

#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif
#ifndef SYSCONFDIR
# define SYSCONFDIR	PREFIX "/etc"
#endif


/* OpenSSL */
/* private */
/* types */
typedef struct _AppTransportPlugin OpenSSL;

struct _AppTransportPlugin
{
	AppTransportPluginHelper * helper;
	int fd;
	SSL_CTX * ssl_ctx;
	/* client */
	SSL * ssl;
};


/* protected */
/* prototypes */
/* plug-in */
static OpenSSL * _openssl_init(AppTransportPluginHelper * helper,
		AppTransportMode mode, char const * name);
static void _openssl_destroy(OpenSSL * openssl);


/* private */
/* prototypes */
static int _openssl_error(char const * message, int code);
static int _openssl_error_ssl(int code);

/* callbacks */
static int _openssl_callback_accept(int fd, OpenSSL * openssl);


/* public */
/* constants */
/* plug-in */
AppTransportPluginDefinition definition =
{
	"OpenSSL",
	NULL,
	_openssl_init,
	_openssl_destroy,
	NULL
};


/* protected */
/* functions */
/* plug-in */
/* openssl_init */
static int _init_client(OpenSSL * openssl, char const * name);
static int _init_server(OpenSSL * openssl, char const * name);

static OpenSSL * _openssl_init(AppTransportPluginHelper * helper,
		AppTransportMode mode, char const * name)
{
	OpenSSL * openssl;
	int res = -1;

	if((openssl = object_new(sizeof(*openssl))) == NULL)
		return NULL;
	openssl->helper = helper;
	openssl->fd = -1;
	openssl->ssl_ctx = NULL;
	openssl->ssl = NULL;
	switch(mode)
	{
		case ATM_CLIENT:
			res = _init_client(openssl, name);
			break;
		case ATM_SERVER:
			res = _init_server(openssl, name);
			break;
	}
	/* check for errors */
	if(res != 0)
	{
		_openssl_destroy(openssl);
		return NULL;
	}
	return openssl;
}

static int _init_client(OpenSSL * openssl, char const * name)
{
	if((openssl->ssl_ctx = SSL_CTX_new(SSLv3_client_method())) == NULL
			|| SSL_CTX_set_cipher_list(openssl->ssl_ctx,
				SSL_DEFAULT_CIPHER_LIST) != 1
			|| (openssl->ssl = SSL_new(openssl->ssl_ctx)) == NULL)
		return -_openssl_error_ssl(1);
	if((openssl->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -_openssl_error("socket", 1);
	/* FIXME implement the rest */
	return 0;
}

static int _init_server(OpenSSL * openssl, char const * name)
{
	String * crt;
	struct sockaddr_in sa;

	if((crt = string_new_append(SYSCONFDIR, "/AppServer/", name, ".crt"))
			== NULL)
		return -1;
	if((openssl->ssl_ctx = SSL_CTX_new(SSLv3_server_method())) == NULL
			|| SSL_CTX_set_cipher_list(openssl->ssl_ctx,
				SSL_DEFAULT_CIPHER_LIST) != 1
			|| SSL_CTX_use_certificate_file(openssl->ssl_ctx, crt,
				SSL_FILETYPE_PEM) == 0
			|| SSL_CTX_use_PrivateKey_file(openssl->ssl_ctx, crt,
				SSL_FILETYPE_PEM) == 0)
	{
		string_delete(crt);
		return -_openssl_error_ssl(1);
	}
	string_delete(crt);
	if((openssl->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -_openssl_error("socket", 1);
	sa.sin_family = AF_INET;
	sa.sin_port = htons(4242); /* XXX hard-coded */
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(openssl->fd, (struct sockaddr *)&sa, sizeof(sa)) != 0)
		return -_openssl_error("bind", 1);
	if(listen(openssl->fd, 5) != 0)
		return -_openssl_error("listen", 1);
	event_register_io_read(openssl->helper->event, openssl->fd,
			(EventIOFunc)_openssl_callback_accept, openssl);
	return 0;
}

/* openssl_destroy */
static void _openssl_destroy(OpenSSL * openssl)
{
	if(openssl->ssl != NULL)
		SSL_free(openssl->ssl);
	if(openssl->fd != -1)
		close(openssl->fd);
	if(openssl->ssl_ctx != NULL)
		SSL_CTX_free(openssl->ssl_ctx);
	object_delete(openssl);
}


/* private */
/* functions */
/* openssl_error */
static int _openssl_error(char const * message, int code)
{
	return error_set_code(code, "%s%s%s", (message != NULL) ? message : "",
			(message != NULL) ? ": " : "", strerror(errno));
}


/* openssl_error_ssl */
static int _openssl_error_ssl(int code)
{
	return error_set_code(code, "%s", ERR_error_string(ERR_get_error(),
				NULL));
}


/* callbacks */
static int _openssl_callback_accept(int fd, OpenSSL * openssl)
{
	struct sockaddr_in sa;
	socklen_t sa_size = sizeof(sa);
	int newfd;

	if((newfd = accept(fd, (struct sockaddr *)&sa, &sa_size)) < 0)
		return error_set_code(1, "%s%s", "accept: ", strerror(errno));
	/* FIXME really implement */
	close(newfd);
	return 0;
}
