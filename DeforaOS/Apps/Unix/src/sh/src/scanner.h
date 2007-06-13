/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix sh */
/* sh is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * sh is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with sh; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#ifndef SH_SCANNER_H
# define SH_SCANNER_H

# include <stdio.h>
# include "token.h"
# include "sh.h"


/* Scanner */
typedef enum _ScannerPrompt { SP_PS1 = 0, SP_PS2, SP_PS4 } ScannerPrompt;
# define SP_LAST SP_PS4
typedef struct _Scanner
{
	FILE * fp;
	ScannerPrompt prompt;
	const char * string;
	int (*next)(struct _Scanner *);
} Scanner;

void scanner_init(Scanner * scanner, Prefs * prefs, FILE * fp,
		char const * string);
Token * scanner_next(Scanner * scanner);

#endif /* !SH_SCANNER_H */
