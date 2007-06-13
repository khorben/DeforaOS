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



#ifndef LIBSYSTEM_EVENT_H
# define LIBSYSTEM_EVENT_H

# include <sys/time.h>
# include <time.h>


/* Event */
/* types */
typedef struct _Event Event;

typedef int (*EventIOFunc)(int fd, void * data);
typedef int (*EventTimeoutFunc)(void * data);


/* functions */
Event * event_new(void);
void event_delete(Event * event);

/* useful */
int event_loop(Event * event);
int event_register_io_read(Event * event, int fd, EventIOFunc func,
		void * userdata);
int event_register_io_write(Event * event, int fd, EventIOFunc func,
		void * userdata);
int event_register_timeout(Event * event, struct timeval timeout,
		EventTimeoutFunc func, void * userdata);
int event_unregister_io_read(Event * event, int fd);
int event_unregister_io_write(Event * event, int fd);
int event_unregister_timeout(Event * event, EventTimeoutFunc func);

#endif /* !LIBSYSTEM_EVENT_H */
