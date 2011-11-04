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



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <System.h>
#include <Phone/modem.h>
#include <sofia-sip/nua.h>
#include <sofia-sip/sip_header.h>
#include <sofia-sip/su_glib.h>
#include <sofia-sip/url.h>


/* Sofia */
/* private */
/* types */
typedef struct _Sofia
{
	su_home_t home[1];
	su_root_t * root;
	guint source;
	nua_t * nua;
	nua_handle_t * handle;
} Sofia;

typedef enum _SofiaConfig
{
	SOFIA_CONFIG_USERNAME = 0,
	SOFIA_CONFIG_FULLNAME,
	SOFIA_CONFIG_REGISTRAR_HOSTNAME = 3,
	SOFIA_CONFIG_REGISTRAR_USERNAME,
	SOFIA_CONFIG_REGISTRAR_PASSWORD,
	SOFIA_CONFIG_PROXY_HOSTNAME = 7
} SofiaConfig;
#define SOFIA_CONFIG_LAST SOFIA_CONFIG_PROXY_HOSTNAME
#define SOFIA_CONFIG_COUNT (SOFIA_CONFIG_LAST + 1)


/* variables */
static ModemConfig _sofia_config[SOFIA_CONFIG_COUNT + 1] =
{
	{ "username",		"Username",	MCT_STRING,	NULL	},
	{ "fullname",		"Full name",	MCT_STRING,	NULL	},
	{ NULL,			"Registrar",	MCT_SUBSECTION,	NULL	},
	{ "registrar_hostname",	"Hostname",	MCT_STRING,	NULL	},
	{ "registrar_username",	"Username",	MCT_STRING,	NULL	},
	{ "registrar_password",	"Password",	MCT_STRING,	NULL	},
	{ NULL,			"Proxy",	MCT_SUBSECTION,	NULL	},
	{ "proxy_hostname",	"Hostname",	MCT_STRING,	NULL	},
	{ NULL,			NULL,		MCT_NONE,	NULL	},
};


/* prototypes */
static int _sofia_init(ModemPlugin * modem);
static int _sofia_destroy(ModemPlugin * modem);
static int _sofia_start(ModemPlugin * modem, unsigned int retry);
static int _sofia_stop(ModemPlugin * modem);
static int _sofia_request(ModemPlugin * modem, ModemRequest * request);

/* callbacks */
static void _sofia_callback(nua_event_t event, int status, char const * phrase,
		nua_t * nua, nua_magic_t * magic, nua_handle_t * nh,
		nua_hmagic_t * hmagic, sip_t const * sip, tagi_t tags[]);


/* public */
/* variables */
ModemPlugin plugin =
{
	NULL,
	"Sofia",
	NULL,
	_sofia_config,
	_sofia_init,
	_sofia_destroy,
	_sofia_start,
	_sofia_stop,
	_sofia_request,
	NULL,
	NULL
};


/* private */
/* functions */
/* sofia_init */
static int _sofia_init(ModemPlugin * modem)
{
	Sofia * sofia;
	GSource * gsource;

	if((sofia = object_new(sizeof(*sofia))) == NULL)
		return -1;
	memset(sofia, 0, sizeof(*sofia));
	modem->priv = sofia;
	su_init();
	su_home_init(sofia->home);
	if((sofia->root = su_glib_root_create(NULL)) == NULL)
	{
		_sofia_destroy(modem);
		return -1;
	}
	gsource = su_glib_root_gsource(sofia->root);
	sofia->source = g_source_attach(gsource, g_main_context_default());
	return 0;
}


/* sofia_destroy */
static int _sofia_destroy(ModemPlugin * modem)
{
	Sofia * sofia = modem->priv;

	_sofia_stop(modem);
	if(sofia->source != 0)
		g_source_remove(sofia->source);
	sofia->source = 0;
	su_root_destroy(sofia->root);
	su_home_deinit(sofia->home);
	su_deinit();
	object_delete(sofia);
	return 0;
}


/* sofia_start */
static int _sofia_start(ModemPlugin * modem, unsigned int retry)
{
	ModemPluginHelper * helper = modem->helper;
	Sofia * sofia = modem->priv;
	char const * username = modem->config[SOFIA_CONFIG_USERNAME].value;
	char const * fullname = modem->config[SOFIA_CONFIG_FULLNAME].value;
	url_string_t us;
	char const * s;
	url_t * url;
	sip_from_t * from;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(sofia->nua != NULL) /* already started */
		return 0;
	if((sofia->nua = nua_create(sofia->root, _sofia_callback, modem,
					TAG_END())) == NULL)
		return -1;
	/* registrar */
	s = modem->config[SOFIA_CONFIG_REGISTRAR_HOSTNAME].value;
	url = url_make(sofia->home, s);
	nua_set_params(sofia->nua, NUTAG_REGISTRAR(url), TAG_END());
	s = modem->config[SOFIA_CONFIG_REGISTRAR_USERNAME].value;
	/* XXX url_make() doesn't prefix with the protocol */
	snprintf(us.us_str, sizeof(us.us_str), "%s%s", "sip:", s);
	url = url_make(sofia->home, us.us_str);
	us.us_url[0] = *url;
	from = sip_from_create(sofia->home, &us);
	/* proxy */
	s = modem->config[SOFIA_CONFIG_PROXY_HOSTNAME].value;
	url = url_make(sofia->home, s);
	nua_set_params(sofia->nua, NUTAG_PROXY(url), TAG_END());
	if((sofia->handle = nua_handle(sofia->nua, modem, TAG_END())) == NULL)
		return -helper->error(helper->modem,
				"Cannot create operation handle", 1);
	nua_register(sofia->handle, NUTAG_M_USERNAME(username),
				NUTAG_M_DISPLAY(fullname), SIPTAG_FROM(from),
				TAG_END());
	return 0;
}


/* sofia_stop */
static void _stop_handle(nua_handle_t ** handle);

static int _sofia_stop(ModemPlugin * modem)
{
	Sofia * sofia = modem->priv;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	_stop_handle(&sofia->handle);
	if(sofia->nua != NULL)
	{
		nua_shutdown(sofia->nua);
		su_root_run(sofia->root);
		nua_destroy(sofia->nua);
	}
	sofia->nua = NULL;
	return 0;
}

static void _stop_handle(nua_handle_t ** handle)
{
	if(*handle == NULL)
		return;
	nua_handle_destroy(*handle);
	*handle = NULL;
}


/* sofia_request */
static int _request_call(ModemPlugin * modem, ModemRequest * request);
static int _request_message_send(ModemPlugin * modem, ModemRequest * request);

static int _sofia_request(ModemPlugin * modem, ModemRequest * request)
{
	switch(request->type)
	{
		case MODEM_REQUEST_CALL:
			return _request_call(modem, request);
		case MODEM_REQUEST_MESSAGE_SEND:
			return _request_message_send(modem, request);
#ifndef DEBUG
		default:
			break;
#endif
	}
	return 0;
}

static int _request_call(ModemPlugin * modem, ModemRequest * request)
{
	Sofia * sofia = modem->priv;
	url_t * url;
	url_string_t us;
	sip_to_t * to;

	if(sofia->handle == NULL || nua_handle_has_active_call(sofia->handle))
		/* FIXME report error, keep track, allow multiple */
		return -1;
	if((url = url_make(sofia->home, request->call.number)) == NULL)
		return -1;
	us.us_url[0] = *url;
	if((to = sip_to_create(NULL, &us)) == NULL)
		return -1; /* XXX free url */
	to->a_display = request->call.number;
	nua_invite(sofia->handle, SIPTAG_TO(to), TAG_END());
	/* FIXME free url? more? */
	return 0;
}

static int _request_message_send(ModemPlugin * modem, ModemRequest * request)
{
	Sofia * sofia = modem->priv;
	url_string_t us;

	if(sofia->handle == NULL)
		/* FIXME report error */
		return -1;
	snprintf(us.us_str, sizeof(us.us_str), "%s%s", "sip:",
			request->message_send.number);
	nua_message(sofia->handle,
			NUTAG_URL(&us),
			SIPTAG_CONTENT_TYPE_STR("text/plain"),
			SIPTAG_PAYLOAD_STR(request->message_send.content),
			TAG_END());
	return 0;
}


/* callbacks */
/* sofia_callback */
static void _sofia_callback(nua_event_t event, int status, char const * phrase,
		nua_t * nua, nua_magic_t * magic, nua_handle_t * nh,
		nua_hmagic_t * hmagic, sip_t const * sip, tagi_t tags[])
{
	ModemPlugin * modem = magic;
	Sofia * sofia = modem->priv;
	ModemEvent mevent;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	switch(event)
	{
		case nua_i_error:
			/* FIXME report error */
			fprintf(stderr, "i_error %03d %s\n", status, phrase);
			break;
		case nua_i_notify:
			/* FIXME report event */
			fprintf(stderr, "i_notify %03d %s\n", status, phrase);
			break;
		case nua_i_state:
			/* FIXME report event */
			fprintf(stderr, "i_state %03d %s\n", status, phrase);
			break;
		case nua_i_terminated:
			memset(&mevent, 0, sizeof(mevent));
			mevent.type = MODEM_EVENT_TYPE_CALL;
			/* FIXME also remember the other fields */
			mevent.call.status = MODEM_CALL_STATUS_NONE;
			modem->helper->event(modem->helper->modem, &mevent);
			break;
		case nua_r_invite:
			memset(&mevent, 0, sizeof(mevent));
			mevent.type = MODEM_EVENT_TYPE_CALL;
			mevent.call.call_type = MODEM_CALL_TYPE_VOICE;
			mevent.call.direction = MODEM_CALL_DIRECTION_OUTGOING;
			if(status == 200)
			{
				mevent.call.status = MODEM_CALL_STATUS_RINGING;
				nua_ack(nh, TAG_END());
			}
			else
			{
				mevent.call.status = MODEM_CALL_STATUS_NONE;
				/* FIXME report error */
				fprintf(stderr, "r_invite %03d %s\n", status,
						phrase);
			}
			modem->helper->event(modem->helper->modem, &mevent);
			break;
		case nua_r_message:
			/* FIXME report event */
			fprintf(stderr, "r_message %03d %s\n", status, phrase);
			break;
		case nua_r_register:
			memset(&mevent, 0, sizeof(mevent));
			mevent.type = MODEM_EVENT_TYPE_REGISTRATION;
			mevent.registration.mode
				= MODEM_REGISTRATION_MODE_AUTOMATIC;
			if(status == 200)
				mevent.registration.status
					= MODEM_REGISTRATION_STATUS_REGISTERED;
			else if(status == 401 || status == 405)
				mevent.registration.status
					= MODEM_REGISTRATION_STATUS_DENIED;
			else if(status >= 400 && status <= 499)
				mevent.registration.status
					= MODEM_REGISTRATION_STATUS_NOT_SEARCHING;
			modem->helper->event(modem->helper->modem, &mevent);
			/* FIXME report errors */
			fprintf(stderr, "r_register %03d %s\n", status, phrase);
			break;
		case nua_r_set_params:
			if(status == 200)
				break;
			/* FIXME implement */
			fprintf(stderr, "r_set_params %03d %s\n", status,
					phrase);
			break;
		case nua_r_shutdown:
			/* exit the background loop when ready */
			if(status == 200)
				su_root_break(sofia->root);
			break;
		default:
#ifdef DEBUG
			fprintf(stderr, "DEBUG: %s() %s%d%s: %03d \"%s\"\n",
					__func__, "event ", event,
					" not handled: ", status, phrase);
#endif
			break;
	}
}
