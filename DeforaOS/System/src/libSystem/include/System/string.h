/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System libSystem */
/* libSystem is not free software; you can redistribute it and/or modify it
 * under the terms of the Creative Commons Attribution-NonCommercial-ShareAlike
 * 3.0 Unported as published by the Creative Commons organization.
 *
 * libSystem is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with libSystem; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#ifndef LIBSYSTEM_STRING_H
# define LIBSYSTEM_STRING_H


/* String */
/* types */
typedef char String;


/* functions */
String * string_new(String const * string);
void string_delete(String * string);

/* returns */
size_t string_length(String const * string);

/* useful */
int string_append(String ** string, String const * append);
void string_cut(String * string, size_t length);

int string_compare(String const * string, String const * string2);
int string_compare_length(String const * string, String const * string2,
		size_t length);

String * string_find(String const * string, String const * key);

#endif /* !LIBSYSTEM_STRING_H */
