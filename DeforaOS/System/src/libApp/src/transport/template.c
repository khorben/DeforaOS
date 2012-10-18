/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System libApp */
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



#include <System.h>
#include "App/apptransport.h"


/* Template */
/* private */
/* types */
typedef struct _AppTransportPlugin Template;

struct _AppTransportPlugin
{
	AppTransportPluginHelper * helper;
};


/* protected */
/* prototypes */
/* plug-in */
static Template * _template_init(AppTransportPluginHelper * helper,
		AppTransportMode mode, char const * name);
static void _template_destroy(Template * template);


/* public */
/* constants */
/* plug-in */
AppTransportPluginDefinition transport =
{
	"Template",
	NULL,
	_template_init,
	_template_destroy,
	NULL
};


/* protected */
/* functions */
/* plug-in */
/* template_init */
static int _init_client(Template * template, char const * name);
static int _init_server(Template * template, char const * name);

static Template * _template_init(AppTransportPluginHelper * helper,
		AppTransportMode mode, char const * name)
{
	Template * template;
	int res = -1;

	if((template = object_new(sizeof(*template))) == NULL)
		return NULL;
	template->helper = helper;
	switch(mode)
	{
		case ATM_CLIENT:
			res = _init_client(template, name);
			break;
		case ATM_SERVER:
			res = _init_server(template, name);
			break;
	}
	/* check for errors */
	if(res != 0)
	{
		_template_destroy(template);
		return NULL;
	}
	return template;
}

static int _init_client(Template * template, char const * name)
{
	/* FIXME really implement */
	return -1;
}

static int _init_server(Template * template, char const * name)
{
	/* FIXME really implement */
	return -1;
}


/* template_destroy */
static void _template_destroy(Template * template)
{
	/* FIXME really implement */
	object_delete(template);
}
