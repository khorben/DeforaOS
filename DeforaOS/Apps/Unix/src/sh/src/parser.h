/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#ifndef SH_PARSER_H
# define SH_PARSER_H

# include <stdio.h>
# include "sh.h"


/* functions */
int parser(Prefs * prefs, char const * string, FILE * fp, int argc,
		char * argv[]);

#endif /* !SH_PARSER_H */
