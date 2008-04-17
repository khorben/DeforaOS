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



#ifndef LIBSYSTEM_APPCLIENT_H
# define LIBSYSTEM_APPCLIENT_H

# include <stdint.h>
# include "event.h"


/* AppClient */
/* types */
typedef struct _AppClient AppClient;


/* functions */
AppClient * appclient_new(char * service);
AppClient * appclient_new_event(char * service, Event * event);
void appclient_delete(AppClient * appclient);

/* useful */
int appclient_call(AppClient * appclient, int32_t * ret, char const * function,
		...);

#endif /* !LIBSYSTEM_APPCLIENT_H */
