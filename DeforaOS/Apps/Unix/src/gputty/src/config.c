/* config.c */
/* Copyright (C) 2004 Pierre Pronchery */
/* This file is part of GPuTTY. */
/* GPuTTY is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPuTTY is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GPuTTY; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */



#include <stdlib.h>
#include "config.h"


/* Config */
Config * config_new(void)
{
	/* FIXME */
	return NULL;
}

void config_delete(Config * config)
{
	free(config);
}


/* useful */
int config_set(Config * config, char * section, char * variable, char * value)
{
	return 1;
}


int config_load(Config * config, char * filename)
{
	return 1;
}

int config_save(Config * config, char * filename)
{
	return 1;
}
