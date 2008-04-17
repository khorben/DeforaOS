/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System libSystem */
/* libSystem is not free software; you can redistribute it and/or modify it
 * under the terms of the Creative Commons Attribution-NonCommercial-ShareAlike
 * 3.0 Unported as published by the Creative Commons organization.
 *
 * libSystem is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with libSystem; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



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
