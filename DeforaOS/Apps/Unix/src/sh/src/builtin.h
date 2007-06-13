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
/* FIXME
 * - implement these each in its own file?
 * - include their .h in this file? */
/* FIXME need to define and track down State
 * - define it in builtin.c? */



#ifndef SH_BUILTIN_H
# define SH_BUILTIN_H


/* functions */
int builtin_alias(int argc, char * argv[]);
int builtin_bg(int argc, char * argv[]); /* FIXME only in XSI */
int builtin_cd(int argc, char * argv[]);
int builtin_exec(int argc, char * argv[]);
int builtin_exit(int argc, char * argv[]);
int builtin_export(int argc, char * argv[]);
int builtin_fg(int argc, char * argv[]); /* FIXME only in XSI */
int builtin_jobs(int argc, char * argv[]); /* FIXME only in XSI */
int builtin_read(int argc, char * argv[]);
int builtin_set(int argc, char * argv[]);
int builtin_umask(int argc, char * argv[]);
int builtin_unalias(int argc, char * argv[]);
int builtin_unset(int argc, char * argv[]);

#endif /* !SH_BUILTIN_H */
