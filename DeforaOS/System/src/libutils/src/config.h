/* config.h */



#ifndef ___CONFIG_H
# define ___CONFIG_H

# include "hash.h"


/* types */
typedef struct _Config {
	Hash * sections;
} Config;


/* functions */
Config * config_new(void);
void config_delete(Config * config);

#endif /* !___CONFIG_H */
