/* config.c */



#include <stdlib.h>
#include "config.h"


/* Config */
Config * config_new(void)
{
	Config * config;

	if((config = malloc(sizeof(Config))) == NULL)
		return NULL;
	return config;
}

void config_delete(Config * config)
{
	int i;

	for(i = hash_get_size(config->sections) - 1; i >= 0; i--)
		hash_delete(hash_get_index(config->sections, i));
	hash_delete(config->sections);
}
