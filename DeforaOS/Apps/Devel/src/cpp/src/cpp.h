/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel cpp */
/* cpp is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * cpp is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with cpp; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#ifndef CPP_CPP_H
# define CPP_CPP_H


/* cpp */
/* types */
typedef struct _Cpp Cpp;


/* functions */
Cpp * cpp_new(void);
void cpp_delete(Cpp * cpp);

char const * cpp_get_error(Cpp * cpp);

/* accessors */
char const * cpp_get_filename(Cpp * cpp);

/* useful */
int cpp_parse(Cpp * cpp, char const * pathname);
ssize_t cpp_read(Cpp * cpp, char * buf, size_t cnt);

#endif /* !CPP_CPP_H */
