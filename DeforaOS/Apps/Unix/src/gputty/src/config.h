/* config.h */



#ifndef _CONFIG_H
# define _CONFIG_H


/* Config */
/* types */
typedef struct _Config {
} Config;


/* functions */
Config * config_new(void);
void config_delete(Config * config);

/* useful */
int config_load(Config * config, char const * filename);
int config_save(Config * config, char const * filename);

# endif /* !_CONFIG_H */
