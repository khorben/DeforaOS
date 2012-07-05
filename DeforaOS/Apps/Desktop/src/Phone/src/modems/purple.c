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
#include <purple.h>
#include "../../config.h"


/* constants */
#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif
#ifndef LIBDIR
# define LIBDIR		PREFIX "/lib"
#endif


/* Purple */
/* private */
/* types */
typedef struct _ModemPlugin
{
	ModemPluginHelper * helper;

	PurpleCoreUiOps ops_ui;
	PurpleEventLoopUiOps ops_glib;
} Purple;


/* variables */
static ModemConfig _purple_config[] =
{
	{ "username",		"Username",	MCT_STRING	},
	{ NULL,			NULL,		MCT_NONE	}
};


/* prototypes */
static ModemPlugin * _purple_init(ModemPluginHelper * helper);
static void _purple_destroy(ModemPlugin * modem);
static int _purple_start(ModemPlugin * modem, unsigned int retry);
static int _purple_stop(ModemPlugin * modem);
static int _purple_request(ModemPlugin * modem, ModemRequest * request);

/* callbacks */
static void _purple_on_ui_init(void);
static void _purple_on_ui_prefs_init(void);


/* public */
/* variables */
ModemPluginDefinition plugin =
{
	NULL,
	"Purple",
	NULL,
	_purple_config,
	_purple_init,
	_purple_destroy,
	_purple_start,
	_purple_stop,
	_purple_request,
	NULL
};


/* private */
/* functions */
/* purple_init */
static ModemPlugin * _purple_init(ModemPluginHelper * helper)
{
	Purple * purple;
	char const * homedir;
	char * p;

	if((purple = object_new(sizeof(*purple))) == NULL)
		return NULL;
	memset(purple, 0, sizeof(*purple));
	purple->helper = helper;
	purple->ops_ui.ui_prefs_init = _purple_on_ui_prefs_init;
	purple->ops_ui.ui_init = _purple_on_ui_init;
	if((homedir = getenv("HOME")) == NULL)
		homedir = g_get_home_dir();
	p = g_build_filename(homedir, ".purple", NULL);
	purple_util_set_user_dir(p);
	g_free(p);
	purple_debug_set_enabled(FALSE);
	purple_core_set_ui_ops(&purple->ops_ui);
	purple_eventloop_set_ui_ops(&purple->ops_glib);
	p = g_build_filename(purple_user_dir(), "plugins", NULL);
	purple_plugins_add_search_path(p);
	g_free(p);
	purple_plugins_add_search_path(LIBDIR);
	if(purple_core_init("phone") == 0)
	{
		_purple_destroy(purple);
		return NULL;
	}
	purple_set_blist(purple_blist_new());
	purple_blist_load();
	purple_prefs_load();
	purple_plugins_load_saved("/phone/plugins/loaded");
	purple_pounces_load();
	return purple;
}


/* purple_destroy */
static void _purple_destroy(ModemPlugin * modem)
{
	Purple * purple = modem;

	_purple_stop(modem);
	object_delete(purple);
}


/* purple_start */
static int _purple_start(ModemPlugin * modem, unsigned int retry)
{
	Purple * purple = modem;
	PurplePlugin * plugin;
	PurplePluginInfo * info;
	GList * list;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	list = purple_plugins_get_protocols();
	for(; list != NULL; list = list->next)
	{
		plugin = list->data;
		info = plugin->info;
	}
	return -1;
}


/* purple_stop */
static int _purple_stop(ModemPlugin * modem)
{
	Purple * purple = modem;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	return 0;
}


/* purple_request */
static int _request_call(ModemPlugin * modem, ModemRequest * request);
static int _request_message_send(ModemPlugin * modem, ModemRequest * request);

static int _purple_request(ModemPlugin * modem, ModemRequest * request)
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
	Purple * purple = modem;

	/* FIXME implement */
	return -1;
}

static int _request_message_send(ModemPlugin * modem, ModemRequest * request)
{
	Purple * purple = modem;

	return -1;
}


/* callbacks */
/* purple_on_ui_init */
static void _purple_on_ui_init(void)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
}


/* purple_on_ui_prefs_init */
static void _purple_on_ui_prefs_init(void)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	purple_prefs_add_path_list("/phone/plugins/loaded", NULL);
}
