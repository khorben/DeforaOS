/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Servers inetd */
/* inetd is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * inetd is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * inetd; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA */



#include <stdlib.h>
#include "inetd.h"
#include "config.h"


/* Config */
/* config_new */
Config * config_new(void)
{
	Config * config;

	if((config = malloc(sizeof(Config))) == NULL)
	{
		inetd_error("malloc", 0);
		return NULL;
	}
	config->services = NULL;
	config->services_nb = 0;
	return config;
}


/* config_delete */
void config_delete(Config * config)
{
	unsigned int i;

	for(i = 0; i < config->services_nb; i++)
		service_delete(config->services[i]);
	free(config->services);
	free(config);
}


/* useful */
/* config_service_add */
int config_service_add(Config * config, Service * service)
{
	Service ** p;

	if(service == NULL)
		return 1;
	if((p = realloc(config->services, sizeof(Service)
					* (config->services_nb + 1))) == NULL)
		return inetd_error(service->name, 1);
	config->services = p;
	config->services[config->services_nb] = service;
	config->services_nb++;
	return 0;
}
