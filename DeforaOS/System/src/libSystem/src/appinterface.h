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



#ifndef LIBSYSTEM_APPINTERFACE_H
# define LIBSYSTEM_APPINTERFACE_H

# include <stdarg.h>


/* AppInterface */
/* types */
typedef struct _AppInterface AppInterface;


/* functions */
AppInterface * appinterface_new(char const * app);
AppInterface * appinterface_new_server(char const * app);
void appinterface_delete(AppInterface * appinterface);

/* accessors */
int appinterface_get_port(AppInterface * appinterface);
int appinterface_get_args_count(AppInterface * appinterface,
		char const * function);

/* useful */
int appinterface_call(AppInterface * appinterface, char buf[], size_t buflen,
		char const * function, void ** args, va_list arg);
int appinterface_call_receive(AppInterface * appinterface, int32_t * ret,
		char buf[], size_t buflen, char const * function, void ** args);
int appinterface_receive(AppInterface * appinterface, int * ret, char buf[],
		size_t buflen, char bufw[], size_t bufwlen, size_t * bufwpos);

#endif /* !LIBSYSTEM_APPINTERFACE_H */
