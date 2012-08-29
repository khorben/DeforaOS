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
	nua_handle_t ** handles;
	size_t handles_cnt;
} Sofia;


/* variables */
static ModemConfig _sofia_config[] =
{
	{ "username",		"Username",	MCT_STRING	},
	{ "fullname",		"Full name",	MCT_STRING	},
	{ NULL,			"Network:",	MCT_SUBSECTION	},
	{ "bind",		"Bind address",	MCT_STRING	},
	{ NULL,			"Registrar:",	MCT_SUBSECTION	},
	{ "registrar_hostname",	"Hostname",	MCT_STRING	},
	{ "registrar_username",	"Username",	MCT_STRING	},
	{ "registrar_password",	"Password",	MCT_PASSWORD	},
	{ NULL,			"Proxy:",	MCT_SUBSECTION	},
	{ "proxy_hostname",	"Hostname",	MCT_STRING	},
	{ NULL,			NULL,		MCT_NONE	},
};


/* prototypes */
/* plug-in */
static ModemPlugin * _sofia_init(ModemPluginHelper * helper);
static void _sofia_destroy(ModemPlugin * modem);
static int _sofia_start(ModemPlugin * modem, unsigned int retry);
static int _sofia_stop(ModemPlugin * modem);
static int _sofia_request(ModemPlugin * modem, ModemRequest * request);

/* useful */
static nua_handle_t * _sofia_handle_add(Sofia * sofia, sip_to_t * to);
static int _sofia_handle_remove(Sofia * sofia, nua_handle_t * handle);

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
	sofia->handles = NULL;
	sofia->handles_cnt = 0;
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
	url_string_t us;
	char const * p;
	char const * q;
	nua_handle_t * handle;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(sofia->nua != NULL) /* already started */
		return 0;
	/* bind address */
	if((p = helper->config_get(helper->modem, "bind")) != NULL
			&& strlen(p) > 0)
		snprintf(us.us_str, sizeof(us.us_str), "%s%s", "sip:", p);
	else
		p = NULL;
	/* initialization */
	if((sofia->nua = nua_create(sofia->root, _sofia_callback, modem,
					TAG_IF(p, NUTAG_URL(us.us_str)),
					SOATAG_AF(SOA_AF_IP4_IP6),
					TAG_END())) == NULL)
		return -1;
	/* username */
	if((p = helper->config_get(helper->modem, "username")) != NULL
			&& strlen(p) > 0)
		nua_set_params(sofia->nua, NUTAG_M_USERNAME(p), TAG_END());
	/* fullname */
	if((p = helper->config_get(helper->modem, "fullname")) != NULL
			&& strlen(p) > 0)
		nua_set_params(sofia->nua, NUTAG_M_DISPLAY(p), TAG_END());
	/* proxy */
	if((p = helper->config_get(helper->modem, "proxy_hostname")) != NULL
			&& strlen(p) > 0)
	{
		snprintf(us.us_str, sizeof(us.us_str), "%s%s", "sip:", p);
		nua_set_params(sofia->nua, NUTAG_PROXY(us.us_str), TAG_END());
	}
	/* registration */
	if((p = helper->config_get(helper->modem, "registrar_username"))
			!= NULL && strlen(p) > 0
			&& (q = helper->config_get(helper->modem,
					"registrar_hostname")) != NULL
			&& strlen(q) > 0)
	{
		if((handle = _sofia_handle_add(sofia, NULL)) == NULL)
			return -helper->error(helper->modem,
					"Cannot create registration handle", 1);
		snprintf(us.us_str, sizeof(us.us_str), "%s%s", "sip:", q);
		nua_set_params(sofia->nua, NUTAG_REGISTRAR(us.us_str),
				TAG_END());
		snprintf(us.us_str, sizeof(us.us_str), "%s%s@%s", "sip:", p, q);
		nua_register(handle, SIPTAG_FROM_STR(us.us_str), TAG_END());
	}
	nua_set_params(sofia->nua, NUTAG_ENABLEMESSAGE(1),
			NUTAG_ENABLEINVITE(1),
			NUTAG_AUTOALERT(1),
			NUTAG_AUTOANSWER(0),
			TAG_END());
	nua_get_params(sofia->nua, TAG_ANY(), TAG_END());
	return 0;
}


/* sofia_stop */
static void _stop_handle(nua_handle_t * handle);

static int _sofia_stop(ModemPlugin * modem)
{
	Sofia * sofia = modem;
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	for(i = 0; i < sofia->handles_cnt; i++)
		_stop_handle(sofia->handles[i]);
	free(sofia->handles);
	sofia->handles = NULL;
	sofia->handles_cnt = 0;
	if(sofia->nua != NULL)
	{
		nua_shutdown(sofia->nua);
		su_root_run(sofia->root);
		nua_destroy(sofia->nua);
	}
	sofia->nua = NULL;
	return 0;
}

static void _stop_handle(nua_handle_t * handle)
{
	if(handle == NULL)
		return;
	nua_handle_destroy(handle);
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
	ModemPluginHelper * helper = sofia->helper;
	nua_handle_t * handle;
	url_string_t us;
	sip_to_t * to;

	snprintf(us.us_str, sizeof(us.us_str), "%s%s", "sip:",
			request->call.number);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__, us.us_str);
#endif
	if((to = sip_to_make(sofia->home, us.us_str)) == NULL)
		return -helper->error(helper->modem,
				"Could not initiate the call", 1);
	if((handle = _sofia_handle_add(sofia, to)) == NULL)
		return -helper->error(helper->modem,
				"Could not initiate the call", 1);
	to->a_display = request->call.number;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() nua_invite(\"%s\")\n", __func__,
			us.us_str);
#endif
	nua_invite(handle, SOATAG_RTP_SORT(SOA_RTP_SORT_REMOTE),
			SOATAG_RTP_SELECT(SOA_RTP_SELECT_ALL), TAG_END());
	return 0;
}

static int _request_message_send(ModemPlugin * modem, ModemRequest * request)
{
	Sofia * sofia = modem;
	ModemPluginHelper * helper = sofia->helper;
	url_string_t us;
	sip_to_t * to;
	nua_handle_t * handle;

	snprintf(us.us_str, sizeof(us.us_str), "%s%s", "sip:",
			request->message_send.number);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__, us.us_str);
#endif
	if((to = sip_to_make(sofia->home, us.us_str)) == NULL)
		return -helper->error(helper->modem, "Could not send message",
				1);
	if((handle = _sofia_handle_add(sofia, to)) == NULL)
		return -helper->error(helper->modem, "Could not send message",
				1);
	nua_message(handle, SIPTAG_CONTENT_TYPE_STR("text/plain"),
			SIPTAG_PAYLOAD_STR(request->message_send.content),
			TAG_END());
	return 0;
}


/* useful */
/* sofia_handle_add */
static nua_handle_t * _sofia_handle_add(Sofia * sofia, sip_to_t * to)
{
	size_t i;
	nua_handle_t ** p;

	for(i = 0; i < sofia->handles_cnt; i++)
		if(sofia->handles[i] == NULL)
			break;
	if(i == sofia->handles_cnt)
	{
		if((p = realloc(sofia->handles, sizeof(*p) * (i + 1))) == NULL)
			return NULL;
		sofia->handles = p;
		sofia->handles_cnt++;
	}
	sofia->handles[i] = nua_handle(sofia->nua, sofia,
			TAG_IF(to, SIPTAG_TO(to)), TAG_END());
	return sofia->handles[i];
}


/* sofia_handle_remove */
static int _sofia_handle_remove(Sofia * sofia, nua_handle_t * handle)
{
	size_t i;

	for(i = 0; i < sofia->handles_cnt; i++)
		if(sofia->handles[i] == handle)
		{
			/* FIXME also free memory */
			nua_handle_destroy(sofia->handles[i]);
			sofia->handles[i] = NULL;
			return 0;
		}
	return -1;
}


/* callbacks */
/* sofia_callback */
static void _callback_invite(ModemPlugin * modem, int status,
		char const * phrase, nua_handle_t * handle);
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
		case nua_i_invite:
			/* FIXME report event: incoming call */
			fprintf(stderr, "i_invite %03d %s\n", status, phrase);
			break;
		case nua_i_cancel:
			/* FIXME report event: incoming call was cancelled */
			fprintf(stderr, "i_invite %03d %s\n", status, phrase);
			break;
		case nua_i_notify:
			/* FIXME report event */
			fprintf(stderr, "i_notify %03d %s\n", status, phrase);
			break;
		case nua_i_outbound:
			/* FIXME what to do? */
			fprintf(stderr, "i_outbound %03d %s\n", status, phrase);
			break;
		case nua_i_state:
			/* FIXME report event, particularly if 180 Ringing! */
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
			_callback_invite(modem, status, phrase, nh);
			break;
		case nua_r_get_params:
			if(status == 200)
				break;
			/* FIXME what to do? */
			fprintf(stderr, "r_get_params %03d %s\n", status,
					phrase);
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

static void _callback_invite(ModemPlugin * modem, int status,
		char const * phrase, nua_handle_t * handle)
{
	Sofia * sofia = modem;
	ModemPluginHelper * helper = sofia->helper;
	ModemEvent mevent;

#ifdef DEBUG
	fprintf(stderr, "%s() %03d %s\n", __func__, status, phrase);
#endif
	memset(&mevent, 0, sizeof(mevent));
	mevent.type = MODEM_EVENT_TYPE_CALL;
	mevent.call.call_type = MODEM_CALL_TYPE_VOICE;
	mevent.call.direction = MODEM_CALL_DIRECTION_OUTGOING;
	if(status == 200)
	{
		mevent.call.status = MODEM_CALL_STATUS_RINGING;
		nua_ack(handle, TAG_END());
	}
	else
	{
		mevent.call.status = MODEM_CALL_STATUS_NONE;
		/* FIXME report error */
		fprintf(stderr, "r_invite %03d %s\n", status, phrase);
		_sofia_handle_remove(sofia, handle);
	}
	helper->event(helper->modem, &mevent);
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
	char const * hostname;
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
		tl_gets(tags, SIPTAG_WWW_AUTHENTICATE_REF(wa), TAG_END());
		hostname = helper->config_get(helper->modem,
				"registrar_hostname");
		username = helper->config_get(helper->modem,
				"registrar_username");
		password = helper->config_get(helper->modem,
				"registrar_password");
		if(wa != NULL && username != NULL && password != NULL)
		{
			scheme = wa->au_scheme;
			realm = msg_params_find(wa->au_params, "realm=");
			authstring = su_sprintf(sofia->home, "%s:%s:%s@%s:%s",
					scheme, realm, username, hostname,
					password);
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
