/* config.h */



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
