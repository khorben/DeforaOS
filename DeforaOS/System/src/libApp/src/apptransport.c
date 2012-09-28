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



#include <string.h>
#include <System.h>
#include "apptransport.h"
#include "../config.h"

#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif
#ifndef LIBDIR
# define LIBDIR		PREFIX "/lib"
#endif


/* AppTransport */
/* private */
/* types */
struct _AppTransport
{
	AppTransportPluginHelper helper;
	Plugin * plugin;
	AppTransportPlugin * tplugin;
	AppTransportPluginDefinition * definition;
};


/* protected */
/* functions */
/* apptransport_new */
AppTransport * apptransport_new(AppTransportMode mode, char const * plugin,
		char const * name)
{
	AppTransport * transport;

	/* allocate the transport */
	if((transport = object_new(sizeof(*transport))) == NULL)
		return NULL;
	memset(transport, 0, sizeof(*transport));
	/* initialize the helper */
	/* FIXME implement */
	/* load the transport plug-in */
	if((transport->plugin = plugin_new(LIBDIR, "App", "transport", plugin))
			== NULL
			|| (transport->definition = plugin_lookup(
					transport->plugin, "transport")) == NULL
			|| transport->definition->init == NULL
			|| transport->definition->destroy == NULL
			|| (transport->tplugin = transport->definition->init(
					&transport->helper, mode, name))
			== NULL)
	{
		apptransport_delete(transport);
		return NULL;
	}
	return transport;
}


/* apptransport_delete */
void apptransport_delete(AppTransport * transport)
{
	if(transport->tplugin != NULL)
		transport->definition->destroy(transport->tplugin);
	if(transport->plugin != NULL)
		plugin_delete(transport->plugin);
	object_delete(transport);
}
