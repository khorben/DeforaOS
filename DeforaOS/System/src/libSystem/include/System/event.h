/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System libSystem */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



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
void event_loop_quit(Event * event);
int event_register_io_read(Event * event, int fd, EventIOFunc func,
		void * userdata);
int event_register_io_write(Event * event, int fd, EventIOFunc func,
		void * userdata);
int event_register_timeout(Event * event, struct timeval * timeout,
		EventTimeoutFunc func, void * userdata);
int event_unregister_io_read(Event * event, int fd);
int event_unregister_io_write(Event * event, int fd);
int event_unregister_timeout(Event * event, EventTimeoutFunc func);

#endif /* !LIBSYSTEM_EVENT_H */
