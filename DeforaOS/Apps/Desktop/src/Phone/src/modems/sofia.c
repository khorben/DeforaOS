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
} Sofia;


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
	NULL,
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
	Sofia * sofia = modem->priv;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(sofia->nua != NULL) /* already started */
		return 0;
	if((sofia->nua = nua_create(sofia->root, _sofia_callback, sofia,
					TAG_NULL())) == NULL)
		return -1;
	nua_set_params(sofia->nua, TAG_NULL());
	return 0;
}


/* sofia_stop */
static int _sofia_stop(ModemPlugin * modem)
{
	Sofia * sofia = modem->priv;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(sofia->nua != NULL)
	{
		nua_shutdown(sofia->nua);
		nua_destroy(sofia->nua);
	}
	sofia->nua = NULL;
	return 0;
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
	}
	return 0;
}

static int _request_call(ModemPlugin * modem, ModemRequest * request)
{
	Sofia * sofia = modem->priv;
	url_t * url;
	struct { nua_handle_t * handle; } * op;
	sip_to_t * to;

	if((url = url_make(sofia->home, request->call.number)) == NULL)
		return -1;
	if((op = su_zalloc(sofia->home, sizeof(*op))) == NULL)
		return -1; /* XXX free url? */
	if((to = sip_to_create(NULL, url)) == NULL)
		return -1; /* XXX free url and op? */
	to->a_display = "Private"; /* XXX look it up */
	if((op->handle = nua_handle(sofia->nua, op, SIPTAG_TO(to), TAG_END()))
			== NULL)
		return -modem->helper->error(modem->helper->modem,
				"Cannot create operation handle", 1);
	nua_invite(op->handle, /* other tags as needed ... */ TAG_END());
	return 0;
}

static int _request_message_send(ModemPlugin * modem, ModemRequest * request)
{
	Sofia * sofia = modem->priv;
	struct { nua_handle_t * handle; } * op;

	if((op = su_zalloc(sofia->home, sizeof(*op))) == NULL)
		return -1; /* XXX free url? */
	if((op->handle = nua_handle(sofia->nua, op, NUTAG_URL(
						request->message_send.number),
					TAG_END())) == NULL)
		return -1;
	nua_message(op->handle, SIPTAG_CONTENT_TYPE_STR("text/plain"),
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
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	switch(event)
	{
		case nua_i_error:
			/* FIXME report error */
			fprintf(stderr, "%03d %s\n", status, phrase);
			break;
		case nua_i_notify:
			/* FIXME report event */
			fprintf(stderr, "%03d %s\n", status, phrase);
			break;
		case nua_i_state:
			/* FIXME report event */
			fprintf(stderr, "%03d %s\n", status, phrase);
			break;
		case nua_r_invite:
			if(status == 200)
				nua_ack(nh, TAG_END());
			else
				/* FIXME report error */
				fprintf(stderr, "%03d %s\n", status, phrase);
			break;
#ifdef DEBUG
		default:
			fprintf(stderr, "DEBUG: %s() %s%d%s: %03d \"%s\"\n",
					__func__, "event ", event,
					" not handled: ", status, phrase);
			break;
#endif
	}
}
