/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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



#ifndef SH_SH_H
# define SH_SH_H


/* types */
/* Prefs */
typedef int Prefs;
#define PREFS_c 0x1
#define PREFS_i 0x2
#define PREFS_s 0x4


/* variables */
extern char ** export;


/* functions */
int sh_error(char const * message, int ret);
char ** sh_export(void);
void sh_handler(int signum);

#endif /* !SH_SH_H */
