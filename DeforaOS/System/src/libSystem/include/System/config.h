/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
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



#ifndef LIBSYSTEM_CONFIG_H
# define LIBSYSTEM_CONFIG_H

# include "hash.h"


/* Config */
/* types */
typedef Hash Config;


/* functions */
Config * config_new(void);
void config_delete(Config * config);

/* useful */
char * config_get(Config * config, char const * section, char const * variable);
int config_set(Config * config, char const * section, char const * variable,
		char const * value);

int config_load(Config * config, char const * filename);
int config_save(Config * config, char const * filename);

#endif /* !LIBSYSTEM_CONFIG_H */
