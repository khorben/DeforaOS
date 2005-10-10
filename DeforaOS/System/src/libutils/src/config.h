/* config.h */



#ifndef ___CONFIG_H
# define ___CONFIG_H

# include "hash.h"


/* Config */
/* types */
typedef Hash Config;


/* functions */
Config * config_new(void);
void config_delete(Config * config);

/* useful */
char * config_get(Config * config, char * section, char * variable);
int config_set(Config * config, char const * section, char const * variable,
		char const * value);

int config_load(Config * config, char * filename);
int config_save(Config * config, char * filename);

#endif /* !___CONFIG_H */
