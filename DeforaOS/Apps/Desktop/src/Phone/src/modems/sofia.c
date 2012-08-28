/* $Id$ */
/* Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org> */
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
typedef struct _ModemPlugin
{
	ModemPluginHelper * helper;

	su_home_t home[1];
	su_root_t * root;
	guint source;
	nua_t * nua;
	nua_handle_t * handle;
} Sofia;


/* variables */
static ModemConfig _sofia_config[] =
{
	{ "username",		"Username",	MCT_STRING	},
	{ "fullname",		"Full name",	MCT_STRING	},
	{ NULL,			"Registrar",	MCT_SUBSECTION	},
	{ "registrar_hostname",	"Hostname",	MCT_STRING	},
	{ "registrar_username",	"Username",	MCT_STRING	},
	{ "registrar_password",	"Password",	MCT_PASSWORD	},
	{ NULL,			"Proxy",	MCT_SUBSECTION	},
	{ "proxy_hostname",	"Hostname",	MCT_STRING	},
	{ NULL,			NULL,		MCT_NONE	},
};


/* prototypes */
static ModemPlugin * _sofia_init(ModemPluginHelper * helper);
static void _sofia_destroy(ModemPlugin * modem);
static int _sofia_start(ModemPlugin * modem, unsigned int retry);
static int _sofia_stop(ModemPlugin * modem);
static int _sofia_request(ModemPlugin * modem, ModemRequest * request);

/* callbacks */
static void _sofia_callback(nua_event_t event, int status, char const * phrase,
		nua_t * nua, nua_magic_t * magic, nua_handle_t * nh,
		nua_hmagic_t * hmagic, sip_t const * sip, tagi_t tags[]);


/* public */
/* variables */
ModemPluginDefinition plugin =
{
	"Sofia",
	NULL,
	_sofia_config,
	_sofia_init,
	_sofia_destroy,
	_sofia_start,
	_sofia_stop,
	_sofia_request,
	NULL
};


/* private */
/* functions */
/* sofia_init */
static ModemPlugin * _sofia_init(ModemPluginHelper * helper)
{
	Sofia * sofia;
	GSource * gsource;

	if((sofia = object_new(sizeof(*sofia))) == NULL)
		return NULL;
	memset(sofia, 0, sizeof(*sofia));
	sofia->helper = helper;
	su_init();
	su_home_init(sofia->home);
	if((sofia->root = su_glib_root_create(NULL)) == NULL)
	{
		_sofia_destroy(sofia);
		return NULL;
	}
	gsource = su_glib_root_gsource(sofia->root);
	sofia->source = g_source_attach(gsource, g_main_context_default());
	return sofia;
}


/* sofia_destroy */
static void _sofia_destroy(ModemPlugin * modem)
{
	Sofia * sofia = modem;

	_sofia_stop(modem);
	if(sofia->source != 0)
		g_source_remove(sofia->source);
	sofia->source = 0;
	su_root_destroy(sofia->root);
	su_home_deinit(sofia->home);
	su_deinit();
	object_delete(sofia);
}


/* sofia_start */
static int _sofia_start(ModemPlugin * modem, unsigned int retry)
{
	Sofia * sofia = modem;
	ModemPluginHelper * helper = sofia->helper;
	char const * username;
	char const * fullname;
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
					NUTAG_URL("sip:0.0.0.0:5060"),
					TAG_END())) == NULL)
		return -1;
	/* username */
	username = helper->config_get(helper->modem, "username");
	fullname = helper->config_get(helper->modem, "fullname");
	/* registrar */
	s = helper->config_get(helper->modem, "registrar_hostname");
	url = url_make(sofia->home, s);
	nua_set_params(sofia->nua, NUTAG_REGISTRAR(url), TAG_END());
	s = helper->config_get(helper->modem, "registrar_username");
	/* XXX url_make() doesn't prefix with the protocol */
	snprintf(us.us_str, sizeof(us.us_str), "%s%s", "sip:", s);
	url = url_make(sofia->home, us.us_str);
	us.us_url[0] = *url;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() sip_from_create(\"%s\")\n", __func__,
			us.us_str);
#endif
	from = sip_from_create(sofia->home, &us);
	/* proxy */
	if((s = helper->config_get(helper->modem, "proxy_hostname")) != NULL)
	{
		url = url_make(sofia->home, s);
		nua_set_params(sofia->nua, NUTAG_PROXY(url), TAG_END());
	}
	if((sofia->handle = nua_handle(sofia->nua, modem, TAG_END())) == NULL)
		return -helper->error(helper->modem,
				"Cannot create operation handle", 1);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() nua_register(\"%s\", \"%s\", \"%s\")\n",
			__func__, username, fullname, us.us_str);
#endif
	nua_register(sofia->handle, NUTAG_M_USERNAME(username),
				NUTAG_M_DISPLAY(fullname), SIPTAG_FROM(from),
				TAG_END());
	return 0;
}


/* sofia_stop */
static void _stop_handle(nua_handle_t ** handle);

static int _sofia_stop(ModemPlugin * modem)
{
	Sofia * sofia = modem;

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
	Sofia * sofia = modem;
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
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() nua_invite(\"%s\")\n", __func__,
			us.us_str);
#endif
	nua_invite(sofia->handle, SIPTAG_TO(to), TAG_END());
	/* FIXME free url? more? */
	return 0;
}

static int _request_message_send(ModemPlugin * modem, ModemRequest * request)
{
	Sofia * sofia = modem;
	url_string_t us;

	if(sofia->handle == NULL)
		/* FIXME report error */
		return -1;
	snprintf(us.us_str, sizeof(us.us_str), "%s%s", "sip:",
			request->message_send.number);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__, us.us_str);
#endif
	nua_message(sofia->handle,
			NUTAG_URL(&us),
			SIPTAG_CONTENT_TYPE_STR("text/plain"),
			SIPTAG_PAYLOAD_STR(request->message_send.content),
			TAG_END());
	return 0;
}


/* callbacks */
/* sofia_callback */
static void _callback_message(ModemPlugin * modem, int status,
		char const * phrase);
static void _callback_register(ModemPlugin * modem, int status,
		char const * phrase, nua_handle_t * nh, sip_t const * sip,
		tagi_t tags[]);

static void _sofia_callback(nua_event_t event, int status, char const * phrase,
		nua_t * nua, nua_magic_t * magic, nua_handle_t * nh,
		nua_hmagic_t * hmagic, sip_t const * sip, tagi_t tags[])
{
	ModemPlugin * modem = magic;
	Sofia * sofia = modem;
	ModemPluginHelper * helper = modem->helper;
	ModemEvent mevent;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u)\n", __func__, event);
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
			helper->event(helper->modem, &mevent);
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
			helper->event(helper->modem, &mevent);
			break;
		case nua_r_message:
			_callback_message(modem, status, phrase);
			break;
		case nua_r_register:
			_callback_register(modem, status, phrase, nh, sip,
					tags);
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

static void _callback_message(ModemPlugin * modem, int status,
		char const * phrase)
{
	Sofia * sofia = modem;
	ModemPluginHelper * helper = sofia->helper;
	ModemEvent mevent;

#ifdef DEBUG
	fprintf(stderr, "%s() %03d %s\n", __func__, status, phrase);
#endif
	memset(&mevent, 0, sizeof(mevent));
	mevent.type = MODEM_EVENT_TYPE_MESSAGE_SENT;
	if(status == 200)
		helper->event(helper->modem, &mevent);
	else
		/* FIXME really report an error */
		helper->event(helper->modem, &mevent);
}

static void _callback_register(ModemPlugin * modem, int status,
		char const * phrase, nua_handle_t * nh, sip_t const * sip,
		tagi_t tags[])
{
	Sofia * sofia = modem;
	ModemPluginHelper * helper = sofia->helper;
	ModemEvent mevent;
	sip_www_authenticate_t const * wa;
	char const * username;
	char const * password;
	char const * scheme;
	char const * realm;
	char * authstring;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() %03d %s\n", __func__, status, phrase);
#endif
	memset(&mevent, 0, sizeof(mevent));
	mevent.type = MODEM_EVENT_TYPE_REGISTRATION;
	mevent.registration.mode = MODEM_REGISTRATION_MODE_AUTOMATIC;
	mevent.registration.status = MODEM_REGISTRATION_STATUS_UNKNOWN;
	if(status == 200)
		mevent.registration.status
			= MODEM_REGISTRATION_STATUS_REGISTERED;
	else if(status == 401 || status == 405)
	{
		mevent.registration.status
			= MODEM_REGISTRATION_STATUS_SEARCHING;
		wa = (sip != NULL) ? sip->sip_www_authenticate : NULL;
		tl_gets(tags, SIPTAG_WWW_AUTHENTICATE_REF(wa), TAG_NULL());
		username = helper->config_get(helper->modem,
				"registrar_username");
		password = helper->config_get(helper->modem,
				"registrar_password");
		if(wa != NULL && username != NULL && password != NULL)
		{
			scheme = wa->au_scheme;
			realm = msg_params_find(wa->au_params, "realm=");
			authstring = su_sprintf(sofia->home, "%s:%s:%s:%s",
					scheme, realm, username, password);
#ifdef DEBUG
			fprintf(stderr, "DEBUG: %s() authstring=\"%s\"\n",
					__func__, authstring);
#endif
			nua_authenticate(nh, NUTAG_AUTH(authstring), TAG_END());
			su_free(sofia->home, authstring);
		}
	}
	else if(status == 403)
		mevent.registration.status = MODEM_REGISTRATION_STATUS_DENIED;
	else if(status >= 400 && status <= 499)
		mevent.registration.status
			= MODEM_REGISTRATION_STATUS_NOT_SEARCHING;
	helper->event(helper->modem, &mevent);
}
