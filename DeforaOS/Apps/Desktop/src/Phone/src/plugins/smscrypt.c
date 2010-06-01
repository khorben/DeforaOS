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
#include <stdio.h>
#include "Phone.h"


/* SMSCrypt */
/* private */
static int _smscrypt_init(PhonePlugin * plugin);
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
	NULL,
	_smscrypt_event,
	_smscrypt_settings,
	NULL
};


/* private */
/* functions */
/* smscrypt_init */
static int _smscrypt_init(PhonePlugin * plugin)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() secret=\"%s\"\n", __func__,
			plugin->helper->config_get(plugin->helper->phone,
				"smscrypt", "secret"));
#endif
	return 0;
}


/* smscrypt_event */
static void _smscrypt_event_sms_received(PhoneEncoding * encoding, char ** buf,
		size_t * len);
static void _smscrypt_event_sms_sending(PhoneEncoding * encoding, char ** buf,
		size_t * len);

static int _smscrypt_event(PhonePlugin * plugin, PhoneEvent event, ...)
{
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
			_smscrypt_event_sms_received(encoding, buf, len);
			break;
		case PHONE_EVENT_SMS_SENDING:
			encoding = va_arg(ap, PhoneEncoding *);
			buf = va_arg(ap, char **);
			len = va_arg(ap, size_t *);
			_smscrypt_event_sms_sending(encoding, buf, len);
			break;
		/* ignore the rest */
		default:
			break;
	}
	va_end(ap);
	return 0;
}

static void _smscrypt_event_sms_received(PhoneEncoding * encoding, char ** buf,
		size_t * len)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u, buf, %lu)\n", __func__, *encoding,
			*len);
#endif
	if(*encoding == PHONE_ENCODING_UTF8)
		return; /* not for us */
	/* FIXME really implement */
	*encoding = PHONE_ENCODING_UTF8;
}

static void _smscrypt_event_sms_sending(PhoneEncoding * encoding, char ** buf,
		size_t * len)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u, buf, %lu)\n", __func__, *encoding, *len);
#endif
	if(*encoding != PHONE_ENCODING_UTF8)
		return; /* not for us */
	/* FIXME really implement */
	*encoding = PHONE_ENCODING_DATA;
}


/* smscrypt_settings */
static void _smscrypt_settings(PhonePlugin * plugin)
{
	/* FIXME implement */
}
