/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#ifndef AS_SCANNER_H
# define AS_SCANNER_H

# include <stdio.h>
# include "token.h"


/* functions */
Token * scan(FILE * fp);
Token * check(FILE * fp, TokenCode code);

#endif /* !AS_SCANNER_H */
