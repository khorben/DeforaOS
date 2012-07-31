/* $Id$ */
/* Copyright (c) 2010-2012 Pierre Pronchery <khorben@defora.org> */
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
#include "../include/Phone/phone.h"
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
	String * name;
	Config * config;
	Plugin * plugin;
	ModemPluginHelper helper;
	ModemPluginDefinition * definition;
	ModemPlugin * modem;
	ModemEventCallback callback;
	unsigned int retry;
	int active;
	void * priv;
};


/* prototypes */
static void _modem_event_callback(Modem * modem, ModemEvent * event);

/* helpers */
static char const * _modem_helper_config_get(Modem * modem,
		char const * variable);
static int _modem_helper_config_set(Modem * modem, char const * variable,
		char const * value);
static int _modem_helper_error(Modem * modem, char const * message, int ret);


/* public */
/* functions */
/* modem_new */
Modem * modem_new(Config * config, char const * name, unsigned int retry)
{
	Modem * modem;

	if((modem = object_new(sizeof(*modem))) == NULL)
		return NULL;
	modem->name = string_new(name);
	modem->modem = NULL;
	modem->config = config;
	modem->retry = retry;
	modem->active = 0;
	modem->callback = NULL;
	modem->priv = NULL;
	if((modem->plugin = plugin_new(LIBDIR, PACKAGE, "modem", name)) != NULL)
		modem->definition = plugin_lookup(modem->plugin, "plugin");
	/* check errors */
	if(modem->name == NULL || modem->plugin == NULL)
	{
		modem_delete(modem);
		return NULL;
	}
	modem->helper.modem = modem;
	modem->helper.config_get = _modem_helper_config_get;
	modem->helper.config_set = _modem_helper_config_set;
	modem->helper.event = _modem_event_callback;
	modem->helper.error = _modem_helper_error;
	if(modem->definition->init == NULL
			|| (modem->modem = modem->definition->init(
					&modem->helper)) == NULL)
	{
		modem->modem = NULL;
		modem_delete(modem);
		return NULL;
	}
	return modem;
}


/* modem_delete */
void modem_delete(Modem * modem)
{
	if(modem->modem != NULL)
		modem_stop(modem);
	if(modem->plugin != NULL)
		plugin_delete(modem->plugin);
	string_delete(modem->name);
	object_delete(modem);
}


/* accessors */
/* modem_get_config */
ModemConfig * modem_get_config(Modem * modem)
{
	return modem->definition->config;
}


/* modem_get_name */
char const * modem_get_name(Modem * modem)
{
	return modem->name;
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
	if(modem->definition->request == NULL)
		return -1;
	return modem->definition->request(modem->modem, request);
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
		case MODEM_REQUEST_BATTERY_LEVEL:
		case MODEM_REQUEST_CALL_ANSWER:
		case MODEM_REQUEST_CALL_HANGUP:
		case MODEM_REQUEST_CONTACT_LIST:
		case MODEM_REQUEST_MESSAGE_LIST:
		case MODEM_REQUEST_SIGNAL_LEVEL:
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
			request.call_presentation.enabled = va_arg(ap,
					unsigned int);
			break;
		case MODEM_REQUEST_CONNECTIVITY:
			request.connectivity.enabled = va_arg(ap, unsigned int);
			break;
		case MODEM_REQUEST_CONTACT:
			request.contact.id = va_arg(ap, unsigned int);
			break;
		case MODEM_REQUEST_CONTACT_EDIT:
			request.contact_edit.id = va_arg(ap, unsigned int);
			request.contact_edit.name = va_arg(ap, char const *);
			request.contact_edit.number = va_arg(ap, char const *);
			break;
		case MODEM_REQUEST_CONTACT_NEW:
			request.contact_new.name = va_arg(ap, char const *);
			request.contact_new.number = va_arg(ap, char const *);
			break;
		case MODEM_REQUEST_CONTACT_DELETE:
			request.contact_delete.id = va_arg(ap, unsigned int);
			break;
		case MODEM_REQUEST_MESSAGE:
		case MODEM_REQUEST_MESSAGE_DELETE:
			request.message.id = va_arg(ap, unsigned int);
			break;
		case MODEM_REQUEST_MESSAGE_SEND:
			request.message_send.number = va_arg(ap, char const *);
			request.message_send.encoding = va_arg(ap,
					unsigned int);
			request.message_send.length = va_arg(ap, size_t);
			request.message_send.content = va_arg(ap, char const *);
			break;
		case MODEM_REQUEST_MUTE:
			request.mute.enabled = va_arg(ap, unsigned int);
			break;
		case MODEM_REQUEST_REGISTRATION:
			request.registration.mode = va_arg(ap,
					ModemRegistrationMode);
			if(request.registration.mode
					== MODEM_REGISTRATION_MODE_MANUAL)
				request.registration._operator = va_arg(ap,
						char const *);
			break;
		case MODEM_REQUEST_UNSUPPORTED:
		default:
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
	if(modem->definition->start != NULL)
		ret = modem->definition->start(modem->modem, modem->retry);
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
	if(modem->definition->stop != NULL)
		ret = modem->definition->stop(modem->modem);
	if(ret == 0)
		modem->active = 0;
	return ret;
}


/* modem_trigger */
int modem_trigger(Modem * modem, ModemEventType event)
{
	if(modem->active == 0)
		return -1; /* XXX report error */
	if(modem->definition->trigger == NULL)
		return -1;
	return modem->definition->trigger(modem->modem, event);
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


/* helpers */
/* modem_helper_config_get */
static char const * _modem_helper_config_get(Modem * modem,
		char const * variable)
{
	char const * ret;
	String * s;

	if((s = string_new_append("modem::", modem->name, NULL)) == NULL)
		return NULL;
	ret = config_get(modem->config, s, variable);
	string_delete(s);
	return ret;
}


/* modem_helper_config_set */
static int _modem_helper_config_set(Modem * modem, char const * variable,
		char const * value)
{
	int ret;
	String * s;

	if((s = string_new_append("modem::", modem->name, NULL)) == NULL)
		return -1;
	ret = config_set(modem->config, s, variable, value);
	string_delete(s);
	return ret;
}


/* modem_helper_error */
static int _modem_helper_error(Modem * modem, char const * message, int ret)
{
	ModemEvent event;

	if(modem == NULL || modem->callback == NULL)
		return error_set_print(PACKAGE, ret, "%s", message);
	event.type = MODEM_EVENT_TYPE_ERROR;
	event.error.message = message;
	modem->callback(modem->priv, &event);
	return ret;
}
