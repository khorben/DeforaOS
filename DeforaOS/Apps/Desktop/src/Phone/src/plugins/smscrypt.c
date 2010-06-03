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



#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <System.h>
#include "Phone.h"


/* SMSCrypt */
/* private */
/* types */
typedef struct _SMSCrypt
{
	char * secret;
	size_t secret_len;
} SMSCrypt;


/* prototypes */
static int _smscrypt_init(PhonePlugin * plugin);
static int _smscrypt_destroy(PhonePlugin * plugin);
static int _smscrypt_event(PhonePlugin * plugin, PhoneEvent event, ...);
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
	char const * secret;

	if((secret = plugin->helper->config_get(plugin->helper->phone,
					"smscrypt", "secret")) == NULL)
		return 1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() secret=\"%s\"\n", __func__, secret);
#endif
	if((smscrypt = malloc(sizeof(*smscrypt))) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	plugin->priv = smscrypt;
	smscrypt->secret = strdup(secret);
	smscrypt->secret_len = strlen(secret);
	if(smscrypt->secret == NULL || smscrypt->secret_len == 0)
	{
		_smscrypt_destroy(plugin);
		return error_set_code(1, "%s", strerror(errno));
	}
	return 0;
}


/* smscrypt_destroy */
static int _smscrypt_destroy(PhonePlugin * plugin)
{
	SMSCrypt * smscrypt = plugin->priv;

	if(smscrypt->secret != NULL)
		memset(smscrypt->secret, 0, smscrypt->secret_len);
	free(smscrypt->secret);
	free(smscrypt);
	return 0;
}


/* smscrypt_event */
static void _smscrypt_event_sms_receiving(SMSCrypt * smscrypt,
		PhoneEncoding * encoding, char ** buf, size_t * len);
static void _smscrypt_event_sms_sending(SMSCrypt * smscrypt,
		PhoneEncoding * encoding, char ** buf, size_t * len);

static int _smscrypt_event(PhonePlugin * plugin, PhoneEvent event, ...)
{
	SMSCrypt * smscrypt = plugin->priv;
	va_list ap;
	PhoneEncoding * encoding;
	char ** buf;
	size_t * len;

	va_start(ap, event);
	switch(event)
	{
		/* our deal */
		case PHONE_EVENT_SMS_RECEIVING:
			encoding = va_arg(ap, PhoneEncoding *);
			buf = va_arg(ap, char **);
			len = va_arg(ap, size_t *);
			_smscrypt_event_sms_receiving(smscrypt, encoding, buf,
					len);
			break;
		case PHONE_EVENT_SMS_SENDING:
			encoding = va_arg(ap, PhoneEncoding *);
			buf = va_arg(ap, char **);
			len = va_arg(ap, size_t *);
			_smscrypt_event_sms_sending(smscrypt, encoding, buf,
					len);
			break;
		/* ignore the rest */
		default:
			break;
	}
	va_end(ap);
	return 0;
}

static void _smscrypt_event_sms_receiving(SMSCrypt * smscrypt,
		PhoneEncoding * encoding, char ** buf, size_t * len)
{
	size_t i;
	size_t j = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u, buf, %lu)\n", __func__, *encoding,
			*len);
#endif
	if(*encoding != PHONE_ENCODING_DATA)
		return; /* not for us */
	for(i = 0; i < *len; i++)
	{
		(*buf)[i] ^= smscrypt->secret[j++];
		j %= smscrypt->secret_len;
	}
	*encoding = PHONE_ENCODING_UTF8;
}

static void _smscrypt_event_sms_sending(SMSCrypt * smscrypt,
		PhoneEncoding * encoding, char ** buf, size_t * len)
{
	size_t i;
	size_t j = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u, buf, %lu)\n", __func__, *encoding, *len);
#endif
	if(*encoding != PHONE_ENCODING_UTF8)
		return; /* not for us */
	for(i = 0; i < *len; i++)
	{
		(*buf)[i] ^= smscrypt->secret[j++];
		j %= smscrypt->secret_len;
	}
	*encoding = PHONE_ENCODING_DATA;
}


/* smscrypt_settings */
static void _smscrypt_settings(PhonePlugin * plugin)
{
	/* FIXME implement */
}
