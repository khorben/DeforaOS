/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System Init */
/* Init is not free software; you can redistribute it and/or modify it
 * under the terms of the Creative Commons Attribution-NonCommercial-ShareAlike
 * 3.0 Unported as published by the Creative Commons organization.
 *
 * Init is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with Init; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#ifndef INIT_SERVICE_H
# define INIT_SERVICE_H

# include <sys/wait.h>		/* for pid_t */


/* Service */
/* protected */
/* types */
typedef struct _Service Service;

typedef enum _ServiceType
{
	ST_NULL = 0,
	ST_COMMAND,
	ST_DAEMON
} ServiceType;
# define ST_LAST ST_DAEMON
# define ST_COUNT (ST_LAST + 1)


/* public */
/* functions */
Service * service_new(char const * name);
Service * service_new_from_config(Config * config);
void service_delete(Service * service);

/* accessors */
int service_get_enabled(Service * service);
char const * service_get_name(Service * service);
pid_t service_get_pid(Service * service);
int service_set_command(Service * service, char const * command);
int service_set_type(Service * service, ServiceType type);

/* useful */
int service_restart(Service * service);
int service_start(Service * service);
int service_stop(Service * service);

#endif /* !INIT_SERVICE_H */
