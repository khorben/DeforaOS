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



#include <unistd.h>
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
};


/* protected */
/* prototypes */
/* plug-in */
static OpenSSL * _openssl_init(AppTransportPluginHelper * helper,
		AppTransportMode mode, char const * name);
static void _openssl_destroy(OpenSSL * openssl);


/* private */
/* prototypes */
static void _openssl_error(void);


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
static void _init_client(OpenSSL * openssl, char const * name);
static void _init_server(OpenSSL * openssl, char const * name);

static OpenSSL * _openssl_init(AppTransportPluginHelper * helper,
		AppTransportMode mode, char const * name)
{
	OpenSSL * openssl;

	if((openssl = object_new(sizeof(*openssl))) == NULL)
		return NULL;
	openssl->helper = helper;
	openssl->fd = -1;
	openssl->ssl_ctx = NULL;
	switch(mode)
	{
		case ATM_CLIENT:
			_init_client(openssl, name);
			break;
		case ATM_SERVER:
			_init_server(openssl, name);
			break;
			/* FIXME implement the rest */
	}
	/* check for errors */
	if(openssl->ssl_ctx == NULL)
	{
		_openssl_destroy(openssl);
		return NULL;
	}
	return openssl;
}

static void _init_client(OpenSSL * openssl, char const * name)
{
	if((openssl->ssl_ctx = SSL_CTX_new(SSLv3_client_method())) == NULL
			|| SSL_CTX_set_cipher_list(openssl->ssl_ctx,
				SSL_DEFAULT_CIPHER_LIST) != 1)
	{
		_openssl_error();
		if(openssl->ssl_ctx != NULL)
			SSL_CTX_free(openssl->ssl_ctx);
		openssl->ssl_ctx = NULL;
		return;
	}
	/* FIXME implement the rest */
}

static void _init_server(OpenSSL * openssl, char const * name)
{
	String * crt;

	if((crt = string_new_append(SYSCONFDIR, "/AppServer/", name, ".crt"))
			== NULL)
		return;
	if((openssl->ssl_ctx = SSL_CTX_new(SSLv3_server_method())) == NULL
			|| SSL_CTX_set_cipher_list(openssl->ssl_ctx,
				SSL_DEFAULT_CIPHER_LIST) != 1
			|| SSL_CTX_use_certificate_file(openssl->ssl_ctx, crt,
				SSL_FILETYPE_PEM) == 0
			|| SSL_CTX_use_PrivateKey_file(openssl->ssl_ctx, crt,
				SSL_FILETYPE_PEM) == 0)
	{
		_openssl_error();
		string_delete(crt);
		if(openssl->ssl_ctx != NULL)
			SSL_CTX_free(openssl->ssl_ctx);
		openssl->ssl_ctx = NULL;
		return;
	}
	string_delete(crt);
	/* FIXME implement the rest */
}

/* openssl_destroy */
static void _openssl_destroy(OpenSSL * openssl)
{
	if(openssl->fd != -1)
		close(openssl->fd);
	if(openssl->ssl_ctx != NULL)
		SSL_CTX_free(openssl->ssl_ctx);
	object_delete(openssl);
}


/* private */
/* functions */
/* openssl_error */
static void _openssl_error(void)
{
	error_set("%s", ERR_error_string(ERR_get_error(), NULL));
}
