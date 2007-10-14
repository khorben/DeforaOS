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



#ifndef LIBSYSTEM_ERROR_H
# define LIBSYSTEM_ERROR_H


/* functions */
/* accessors */
char const * error_get(void);
char const * error_get_code(int * code);

void error_set(char const * message);
int error_set_code(char const * message, int code);
int error_set_print(char const * prefix, char const * message, int code);

/* useful */
int error_print(char const * prefix);

#endif /* !LIBSYSTEM_ERROR_H */
