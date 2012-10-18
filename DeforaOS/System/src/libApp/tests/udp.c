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



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <System.h>
#include "App.h"


/* private */
/* prototypes */
static int _udp(char const * name);

static int _usage(void);


/* functions */
/* udp */
static int _udp(char const * name)
{
	char * cwd;
	Plugin * plugin;
	AppTransportPluginDefinition * plugind;
	AppTransportPlugin * server;
	AppTransportPlugin * client;

	/* load the transport plug-in */
	if((cwd = getcwd(NULL, 0)) == NULL)
		return error_set_print("udp", 2, "%s", strerror(errno));
	/* XXX rather ugly but does the trick */
	plugin = plugin_new(cwd, "../src", "transport", "udp");
	free(cwd);
	if(plugin == NULL)
		return error_print("udp");
	if((plugind = plugin_lookup(plugin, "transport")) == NULL)
	{
		plugin_delete(plugin);
		return error_print("udp");
	}
	/* create a server and a client */
	server = plugind->init(NULL, ATM_SERVER, name);
	client = plugind->init(NULL, ATM_CLIENT, name);
	if(server == NULL || client == NULL)
	{
		if(client != NULL)
			plugind->destroy(client);
		if(server != NULL)
			plugind->destroy(server);
		plugin_delete(plugin);
		return error_print("udp");
	}
	/* FIXME really implement */
	plugin_delete(plugin);
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: udp name\n", stderr);
	return 1;
}


/* public */
/* functions */
/* main */
int main(int argc, char * argv[])
{
	char const * name = "127.0.0.1:4242";
	int o;

	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind == argc - 1)
		name = argv[optind];
	else if(optind != argc)
		return _usage();
	return (_udp(name) == 0) ? 0 : 2;
}
