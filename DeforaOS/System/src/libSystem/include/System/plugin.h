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



#ifndef LIBSYSTEM_PLUGIN_H
# define LIBSYSTEM_PLUGIN_H


/* Object */
typedef void Plugin;


/* functions */
Plugin * plugin_new(char const * libdir, char const * package,
		char const * type, char const * name);
Plugin * plugin_new_self(void);
void plugin_delete(Plugin * plugin);


/* useful */
void * plugin_lookup(Plugin * plugin, char const * symbol);

#endif /* !LIBSYSTEM_PLUGIN_H */
