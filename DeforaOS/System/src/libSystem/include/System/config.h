/* config.h */



#ifndef _CONFIG_H_
# define _CONFIG_H_

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

#endif /* !_CONFIG_H_ */
