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



#ifndef LIBSYSTEM_APPSERVER_H
# define LIBSYSTEM_APPSERVER_H

# include "event.h"


/* AppServer */
/* types */
typedef struct _AppServer AppServer;
typedef enum _AppServerOptions
{
	ASO_LOCAL = 1,
	ASO_REMOTE = 2
} AppServerOptions;


/* functions */
AppServer * appserver_new(char const * app, int options);
AppServer * appserver_new_event(char const * app, int options, Event * event);
void appserver_delete(AppServer * appserver);

/* accessors */
void * appserver_get_client_id(AppServer * appserver);

/* useful */
int appserver_loop(AppServer * appserver);

#endif /* !LIBSYSTEM_APPSERVER_H */
