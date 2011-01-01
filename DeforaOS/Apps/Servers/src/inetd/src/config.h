/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Servers inetd */
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



#ifndef __CONFIG_H
# define __CONFIG_H

# include "service.h"


/* types */
typedef struct _Config {
	Service ** services;
	unsigned int services_nb;
} Config;


/* functions */
Config * config_new(void);
void config_delete(Config * config);

/* useful */
int config_service_add(Config * config, Service * service);

#endif /* !__CONFIG_H */
