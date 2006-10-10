/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#ifndef AS_AS_H
# define AS_AS_H

# define AS_FILENAME_DEFAULT "a.out"


/* functions */
/* useful */
int as_error(char const * msg, int ret);

/* plugins helpers */
void * as_plugin_new(char const * type, char const * name,
		char const * description);
void as_plugin_delete(void * plugin);
void as_plugin_list(char const * type, char const * description);

#endif /* !AS_AS_H */
