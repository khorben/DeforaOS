/* config.c */



#include <stdlib.h>
#include "inetd.h"
#include "config.h"


/* Config */
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

void config_delete(Config * config)
{
	unsigned int i;

	for(i = 0; i < config->services_nb; i++)
		service_delete(config->services[i]);
	free(config->services);
	free(config);
}


/* useful */
int config_service_add(Config * config, Service * service)
{
	Service ** p;

	if(service == NULL)
		return 1;
	if((p = realloc(config->services, sizeof(Service)
					* (config->services_nb + 1))) == NULL)
		return 1; /* FIXME be verbose */
	config->services = p;
	config->services[config->services_nb] = service;
	config->services_nb++;
	return 0;
}
