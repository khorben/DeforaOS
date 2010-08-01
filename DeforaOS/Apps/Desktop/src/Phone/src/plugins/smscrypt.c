/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Phone */
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
/* TODO:
 * - set the secret global or per-number
 * - apply XOR on the result of the previous buffer
 * - XOR against a hash of the secret
 * - settings window */



#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <openssl/sha.h>
#include <System.h>
#include "Phone.h"


/* SMSCrypt */
/* private */
/* types */
typedef struct _SMSCrypt
{
	unsigned char buf[20];
	size_t len;
} SMSCrypt;


/* prototypes */
static int _smscrypt_init(PhonePlugin * plugin);
static int _smscrypt_destroy(PhonePlugin * plugin);
static int _smscrypt_event(PhonePlugin * plugin, PhoneEvent event, ...);
static int _smscrypt_secret(PhonePlugin * plugin, char const * number);
static void _smscrypt_settings(PhonePlugin * plugin);


/* public */
/* variables */
PhonePlugin plugin =
{
	NULL,
	"SMS encryption",
	NULL,
	_smscrypt_init,
	_smscrypt_destroy,
	_smscrypt_event,
	_smscrypt_settings,
	NULL
};


/* private */
/* functions */
/* smscrypt_init */
static int _smscrypt_init(PhonePlugin * plugin)
{
	SMSCrypt * smscrypt;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() secret=\"%s\"\n", __func__, secret);
#endif
	if((smscrypt = malloc(sizeof(*smscrypt))) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	plugin->priv = smscrypt;
	smscrypt->len = sizeof(smscrypt->buf);
	return 0;
}


/* smscrypt_destroy */
static int _smscrypt_destroy(PhonePlugin * plugin)
{
	SMSCrypt * smscrypt = plugin->priv;

	free(smscrypt);
	return 0;
}


/* smscrypt_event */
static int _smscrypt_event_sms_receiving(PhonePlugin * plugin,
		char const * number, PhoneEncoding * encoding, char ** buf,
		size_t * len);
static int _smscrypt_event_sms_sending(PhonePlugin * plugin,
		char const * number, PhoneEncoding * encoding, char ** buf,
		size_t * len);

static int _smscrypt_event(PhonePlugin * plugin, PhoneEvent event, ...)
{
	int ret = 0;
	va_list ap;
	char const * number;
	PhoneEncoding * encoding;
	char ** buf;
	size_t * len;

	va_start(ap, event);
	switch(event)
	{
		/* our deal */
		case PHONE_EVENT_SMS_RECEIVING:
			number = va_arg(ap, char const *);
			encoding = va_arg(ap, PhoneEncoding *);
			buf = va_arg(ap, char **);
			len = va_arg(ap, size_t *);
			ret = _smscrypt_event_sms_receiving(plugin, number,
					encoding, buf, len);
			break;
		case PHONE_EVENT_SMS_SENDING:
			number = va_arg(ap, char const *);
			encoding = va_arg(ap, PhoneEncoding *);
			buf = va_arg(ap, char **);
			len = va_arg(ap, size_t *);
			ret = _smscrypt_event_sms_sending(plugin, number,
					encoding, buf, len);
			break;
		/* ignore the rest */
		default:
			break;
	}
	va_end(ap);
	return ret;
}

static int _smscrypt_event_sms_receiving(PhonePlugin * plugin,
		char const * number, PhoneEncoding * encoding, char ** buf,
		size_t * len)
{
	SMSCrypt * smscrypt = plugin->priv;
	size_t i;
	size_t j = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u, buf, %lu)\n", __func__, *encoding,
			*len);
#endif
	if(*encoding != PHONE_ENCODING_DATA)
		return 0; /* not for us */
	if(_smscrypt_secret(plugin, number) != 0)
		return 0; /* XXX warn */
	for(i = 0; i < *len; i++)
	{
		(*buf)[i] ^= smscrypt->buf[j++];
		j %= smscrypt->len;
	}
	*encoding = PHONE_ENCODING_UTF8;
	return 0;
}

static int _smscrypt_event_sms_sending(PhonePlugin * plugin,
		char const * number, PhoneEncoding * encoding, char ** buf,
		size_t * len)
{
	SMSCrypt * smscrypt = plugin->priv;
	size_t i;
	size_t j = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", %u, buf, %lu)\n", __func__, number,
			*encoding, *len);
#endif
	if(*encoding != PHONE_ENCODING_UTF8)
		return 0; /* not for us */
	if(_smscrypt_secret(plugin, number) != 0)
		return 0; /* XXX warn */
	*encoding = PHONE_ENCODING_DATA;
	for(i = 0; i < *len; i++)
	{
		(*buf)[i] ^= smscrypt->buf[j++];
		j %= smscrypt->len;
	}
	*encoding = PHONE_ENCODING_DATA;
	return 0;
}


/* smscrypt_secret */
static int _smscrypt_secret(PhonePlugin * plugin, char const * number)
{
	SMSCrypt * smscrypt = plugin->priv;
	char const * secret = NULL;
	SHA_CTX sha1;

	if(number != NULL)
		secret = plugin->helper->config_get(plugin->helper->phone,
				"smscrypt", number);
	if(secret == NULL)
		secret = plugin->helper->config_get(plugin->helper->phone,
				"smscrypt", "secret");
	if(secret == NULL)
		return 1;
	SHA1_Init(&sha1);
	SHA1_Update(&sha1, (unsigned char const *)secret, strlen(secret));
	SHA1_Final(smscrypt->buf, &sha1);
	return 0;
}


/* smscrypt_settings */
static void _smscrypt_settings(PhonePlugin * plugin)
{
	/* FIXME implement */
}
