/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System libSystem */
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



#ifndef LIBSYSTEM_CONFIG_H
# define LIBSYSTEM_CONFIG_H

# include "hash.h"


/* Config */
/* types */
typedef Hash Config;


/* functions */
Config * config_new(void);
void config_delete(Config * config);

/* accessors */
char const * config_get(Config * config, char const * section,
		char const * variable);
int config_set(Config * config, char const * section, char const * variable,
		char const * value);

/* useful */
int config_load(Config * config, char const * filename);
int config_reset(Config * config);
int config_save(Config * config, char const * filename);

#endif /* !LIBSYSTEM_CONFIG_H */
