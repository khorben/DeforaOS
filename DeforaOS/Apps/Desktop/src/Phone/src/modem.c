/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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
#ifdef DEBUG
# include <stdio.h>
#endif
#include <string.h>
#include <System.h>
#include "Phone/phone.h"
#include "modem.h"
#include "../config.h"

#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif
#ifndef LIBDIR
# define LIBDIR		PREFIX "/lib"
#endif


/* Modem */
/* private */
/* types */
struct _Modem
{
	Plugin * plugin;
	ModemPluginHelper helper;
	ModemPlugin * modem;
	ModemEventCallback callback;
	unsigned int retry;
	int active;
	void * priv;
};


/* prototypes */
static void _modem_event_callback(Modem * modem, ModemEvent * event);
static int _modem_error(Modem * modem, char const * message, int ret);


/* public */
/* functions */
/* modem_new */
static void _new_config(Config * config, char const * name,
		ModemPlugin * plugin);

Modem * modem_new(Config * config, char const * name, unsigned int retry)
{
	Modem * modem;

	if((modem = object_new(sizeof(*modem))) == NULL)
		return NULL;
	modem->modem = NULL;
	modem->retry = retry;
	modem->active = 0;
	modem->callback = NULL;
	modem->priv = NULL;
	if((modem->plugin = plugin_new(LIBDIR, PACKAGE, "modem", name))
			== NULL || (modem->modem = plugin_lookup(modem->plugin,
					"plugin")) == NULL)
	{
		modem_delete(modem);
		return NULL;
	}
	modem->helper.modem = modem;
	modem->helper.event = _modem_event_callback;
	modem->helper.error = _modem_error;
	modem->modem->helper = &modem->helper;
	_new_config(config, name, modem->modem);
	if(modem->modem->init != NULL && modem->modem->init(modem->modem) != 0)
	{
		modem->modem = NULL;
		modem_delete(modem);
		return NULL;
	}
	return modem;
}

static void _new_config(Config * config, char const * name,
		ModemPlugin * plugin)
{
	size_t i;
	String * section;
	char const * p;
	unsigned long u;

	if(plugin->config == NULL)
		return;
	if((section = string_new_append("modem_", name, NULL)) == NULL)
		return; /* XXX report error */
	for(i = 0; plugin->config[i].name != NULL; i++)
	{
		if((p = config_get(config, section, plugin->config[i].name))
				== NULL)
			continue;
		switch(plugin->config[i].type)
		{
			case MCT_NONE: /* XXX should not happen */
				break;
			case MCT_BOOLEAN:
				plugin->config[i].value = (strcmp(p, "1") == 0)
					? (void*)1 : NULL;
				break;
			case MCT_FILENAME: /* FIXME really implement */
			case MCT_STRING:
				/* FIXME should copy the string */
				plugin->config[i].value = (void*)p;
				break;
			case MCT_UINT32:
				u = strtoul(p, NULL, 10);
				plugin->config[i].value = (void*)u;
				break;
		}
	}
	string_delete(section);
}


/* modem_delete */
void modem_delete(Modem * modem)
{
	if(modem->modem != NULL)
		modem_stop(modem);
	if(modem->plugin != NULL)
		plugin_delete(modem->plugin);
	object_delete(modem);
}


/* accessors */
/* modem_get_config */
ModemConfig * modem_get_config(Modem * modem)
{
	return modem->modem->config;
}


/* modem_set_callback */
void modem_set_callback(Modem * modem, ModemEventCallback callback, void * priv)
{
	modem->callback = callback;
	modem->priv = priv;
}


/* useful */
/* modem_request */
int modem_request(Modem * modem, ModemRequest * request)
{
	if(modem->modem->request == NULL)
		return -1;
	return modem->modem->request(modem->modem, request);
}


/* modem_request_type */
int modem_request_type(Modem * modem, ModemRequestType type, ...)
{
	va_list ap;
	ModemRequest request;

	va_start(ap, type);
	memset(&request, 0, sizeof(request));
	switch((request.type = type))
	{
		/* no arguments */
		case MODEM_REQUEST_CALL_ANSWER:
		case MODEM_REQUEST_CALL_HANGUP:
		case MODEM_REQUEST_CONTACT_LIST:
		case MODEM_REQUEST_MESSAGE_LIST:
			break;
		case MODEM_REQUEST_AUTHENTICATE:
			request.authenticate.name = va_arg(ap,
					char const *);
			request.authenticate.username = va_arg(ap,
					char const *);
			request.authenticate.password = va_arg(ap,
					char const *);
			break;
		case MODEM_REQUEST_CALL:
			request.call.call_type = va_arg(ap, ModemCallType);
			request.call.number = va_arg(ap, char const *);
			request.call.anonymous = va_arg(ap, int);
			break;
		case MODEM_REQUEST_CALL_PRESENTATION:
			request.call_presentation.enabled = va_arg(ap, int);
			break;
		case MODEM_REQUEST_CONTACT:
			request.contact.id = va_arg(ap, unsigned int);
			break;
		case MODEM_REQUEST_CONTACT_DELETE:
			request.contact_delete.id = va_arg(ap, unsigned int);
			break;
		case MODEM_REQUEST_MESSAGE:
			request.message.id = va_arg(ap, unsigned int);
			break;
		case MODEM_REQUEST_MESSAGE_SEND:
			request.message_send.number = va_arg(ap, char const *);
			request.message_send.encoding = va_arg(ap,
					unsigned int);
			request.message_send.length = va_arg(ap, size_t);
			request.message_send.content = va_arg(ap, char const *);
			break;
		case MODEM_REQUEST_REGISTRATION:
			request.registration.mode = va_arg(ap,
					ModemRegistrationMode);
			break;
		default: /* XXX unknown request type */
#ifdef DEBUG
			fprintf(stderr, "DEBUG: %s(%u) %s\n", __func__, type,
					"Unsupported request type");
#endif
			va_end(ap);
			return -1;
	}
	va_end(ap);
	return modem_request(modem, &request);
}


/* modem_start */
int modem_start(Modem * modem)
{
	int ret = 0;

	if(modem->active != 0)
		return 0;
	if(modem->modem->start != NULL)
		ret = modem->modem->start(modem->modem, modem->retry);
	if(ret == 0)
		modem->active = 1;
	return ret;
}


/* modem_stop */
int modem_stop(Modem * modem)
{
	int ret = 0;

	if(modem->active == 0)
		return 0;
	if(modem->modem->stop != NULL)
		ret = modem->modem->stop(modem->modem);
	if(ret == 0)
		modem->active = 0;
	return ret;
}


/* modem_trigger */
int modem_trigger(Modem * modem, ModemEventType event)
{
	if(modem->active == 0)
		return -1; /* XXX report error */
	if(modem->modem->trigger == NULL)
		return -1;
	return modem->modem->trigger(modem->modem, event);
}


/* private */
/* functions */
/* modem_event_callback */
static void _modem_event_callback(Modem * modem, ModemEvent * event)
{
	if(modem->callback == NULL)
		return;
	modem->callback(modem->priv, event);
}


/* modem_error */
static int _modem_error(Modem * modem, char const * message, int ret)
{
	ModemEvent event;

	if(modem == NULL || modem->callback == NULL)
		return error_set_print(PACKAGE, ret, "%s", message);
	event.type = MODEM_EVENT_TYPE_ERROR;
	event.error.message = message;
	modem->callback(modem->priv, &event);
	return ret;
}
