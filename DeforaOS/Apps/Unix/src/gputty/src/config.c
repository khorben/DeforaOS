/* config.c */



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
int config_load(Config * config, char const * filename)
{
	return 0;
}

int config_save(Config * config, char const * filename)
{
	return 0;
}
