/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



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
