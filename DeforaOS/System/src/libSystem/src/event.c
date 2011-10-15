/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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



#include <assert.h>
#ifdef __WIN32__
# include <Winsock2.h>
#else
# include <sys/select.h>
#endif
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#ifdef DEBUG
# include <stdio.h>
#endif
#include <errno.h>
#include "System.h"

/* macros */
#ifndef max
# define max(a, b) ((a) >= (b)) ? (a) : (b)
#endif


/* Event */
/* private */
/* types */
typedef struct _EventTimeout
{
	struct timeval initial;
	struct timeval timeout;
	EventTimeoutFunc func;
	void * data;
} EventTimeout;
ARRAY(EventTimeout *, eventtimeout)

typedef struct _EventIO
{
	int fd;
	EventIOFunc func;
	void * data;
} EventIO;
ARRAY(EventIO *, eventio)

struct _Event
{
	unsigned int loop;
	int fdmax;
	fd_set rfds;
	fd_set wfds;
	eventioArray * reads;
	eventioArray * writes;
	eventtimeoutArray * timeouts;
	struct timeval timeout;
};


/* public */
/* functions */
/* event_new */
Event * event_new(void)
{
	Event * event;

	if((event = object_new(sizeof(*event))) == NULL)
		return NULL;
	event->timeouts = eventtimeoutarray_new();
	event->loop = 0;
	event->fdmax = -1;
	FD_ZERO(&event->rfds);
	FD_ZERO(&event->wfds);
	event->reads = eventioarray_new();
	event->writes = eventioarray_new();
	event->timeout.tv_sec = (time_t)LONG_MAX;
	event->timeout.tv_usec = (suseconds_t)LONG_MAX;
	if(event->timeouts == NULL || event->reads == NULL
			|| event->writes == NULL)
	{
		event_delete(event);
		return NULL;
	}
	return event;
}


/* event_delete */
void event_delete(Event * event)
{
	unsigned int i;
	EventTimeout * et;
	EventIO * eio;

	for(i = 0; i < array_count(event->timeouts); i++)
	{
		array_get_copy(event->writes, i, &et);
		object_delete(et);
	}
	array_delete(event->timeouts);
	for(i = 0; i < array_count(event->reads); i++)
	{
		array_get_copy(event->reads, i, &eio);
		object_delete(eio);
	}
	array_delete(event->reads);
	for(i = 0; i < array_count(event->writes); i++)
	{
		array_get_copy(event->writes, i, &eio);
		object_delete(eio);
	}
	array_delete(event->writes);
	object_delete(event);
}


/* useful */
/* event_loop */
static int _loop_timeout(Event * event);
static void _loop_io(Event * event, eventioArray * eios, fd_set * fds);

int event_loop(Event * event)
{
	struct timeval * timeout = (event->timeout.tv_sec == (time_t)LONG_MAX
			&& event->timeout.tv_usec == (suseconds_t)LONG_MAX)
		? NULL : &event->timeout;
	fd_set rfds = event->rfds;
	fd_set wfds = event->wfds;

	event->loop++;
	while(event->loop && (timeout != NULL || event->fdmax != -1))
	{
		if(select(event->fdmax + 1, &rfds, &wfds, NULL, timeout) < 0)
			return error_set_code(1, "%s", strerror(errno));
		if(_loop_timeout(event) != 0)
			return 1;
		_loop_io(event, event->reads, &rfds);
		_loop_io(event, event->writes, &wfds);
		if(event->timeout.tv_sec == (time_t)LONG_MAX
				&& event->timeout.tv_usec
				== (suseconds_t)LONG_MAX)
			timeout = NULL;
		else
			timeout = &event->timeout;
		rfds = event->rfds;
		wfds = event->wfds;
	}
	return 0;
}

static int _loop_timeout(Event * event)
{
	struct timeval now;
	unsigned int i = 0;
	EventTimeout * et;

	if(gettimeofday(&now, NULL) != 0)
	{
		error_set_code(1, "%s", strerror(errno));
		return -1;
	}
	event->timeout.tv_sec = (time_t)LONG_MAX;
	event->timeout.tv_usec = (suseconds_t)LONG_MAX;
	while(i < array_count(event->timeouts))
	{
		array_get_copy(event->timeouts, i, &et);
		if(now.tv_sec > et->timeout.tv_sec
				|| (now.tv_sec == et->timeout.tv_sec
					&& now.tv_usec >= et->timeout.tv_usec))
		{
			if(et->func(et->data) != 0)
			{
				array_remove_pos(event->timeouts, i);
				object_delete(et);
				continue;
			}
			et->timeout.tv_sec = et->initial.tv_sec + now.tv_sec;
			et->timeout.tv_usec = et->initial.tv_usec + now.tv_usec;
			if(et->initial.tv_sec < event->timeout.tv_sec
					|| (et->initial.tv_sec == event->timeout.tv_sec
						&& et->initial.tv_usec
						< event->timeout.tv_usec))
			{
				event->timeout.tv_sec = et->initial.tv_sec;
				event->timeout.tv_usec = et->initial.tv_usec;
			}
		}
		else
		{
			if(et->timeout.tv_sec - now.tv_sec < event->timeout.tv_sec
					|| (et->timeout.tv_sec - now.tv_sec == event->timeout.tv_sec
						&& et->timeout.tv_usec - now.tv_usec < event->timeout.tv_usec))
			{
				event->timeout.tv_sec = et->timeout.tv_sec
					- now.tv_sec;
				/* FIXME may be needed elsewhere too */
				if(et->timeout.tv_usec >= now.tv_usec)
					event->timeout.tv_usec
						= et->timeout.tv_usec
						- now.tv_usec;
				else
				{
					event->timeout.tv_sec--;
					event->timeout.tv_usec
						= now.tv_usec
						- et->timeout.tv_usec;
				}
			}
		}
		i++;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() %s%ld%s%ld => 0\n", __func__, "tv_sec=",
			(long)event->timeout.tv_sec, ", tv_usec=",
			(long)event->timeout.tv_usec);
#endif
	return 0;
}

static void _loop_io(Event * event, eventioArray * eios, fd_set * fds)
{
	unsigned int i = 0;
	EventIO * eio;
	int fd;

	while(i < array_count(eios))
	{
		array_get_copy(eios, i, &eio);
		if((fd = eio->fd) <= event->fdmax && FD_ISSET(fd, fds)
				&& eio->func(fd, eio->data) != 0)
		{
			if(eios == event->reads)
				event_unregister_io_read(event, fd);
			else if(eios == event->writes)
				event_unregister_io_write(event, fd);
#ifdef DEBUG
			else
				fprintf(stderr, "DEBUG: %s%s", __func__,
						"(): should not happen\n");
#endif
		}
		else
			i++;
	}
}


/* event_loop_quit */
void event_loop_quit(Event * event)
{
	if(event->loop > 0)
		event->loop--;
}


/* event_register_io_read */
int event_register_io_read(Event * event, int fd, EventIOFunc func,
		void * userdata)
{
	EventIO * eventio;

	assert(fd >= 0);
	if((eventio = object_new(sizeof(*eventio))) == NULL)
		return 1;
	eventio->fd = fd;
	eventio->func = func;
	eventio->data = userdata;
	event->fdmax = max(event->fdmax, fd);
	FD_SET(fd, &event->rfds);
	array_append(event->reads, &eventio);
	return 0;
}


/* event_register_io_write */
int event_register_io_write(Event * event, int fd, EventIOFunc func,
		void * userdata)
{
	EventIO * eventio;

	assert(fd >= 0);
	if((eventio = object_new(sizeof(*eventio))) == NULL)
		return 1;
	eventio->fd = fd;
	eventio->func = func;
	eventio->data = userdata;
	event->fdmax = max(event->fdmax, fd);
	FD_SET(fd, &event->wfds);
	array_append(event->writes, &eventio);
	return 0;
}


/* event_register_timeout */
int event_register_timeout(Event * event, struct timeval * timeout,
		EventTimeoutFunc func, void * data)
{
	EventTimeout * eventtimeout;
	struct timeval now;

	if(gettimeofday(&now, NULL) != 0)
		return error_set_code(1, "%s", strerror(errno));
	if((eventtimeout = object_new(sizeof(*eventtimeout))) == NULL)
		return 1;
	eventtimeout->initial.tv_sec = timeout->tv_sec;
	eventtimeout->initial.tv_usec = timeout->tv_usec;
	eventtimeout->timeout.tv_sec = now.tv_sec + timeout->tv_sec;
	eventtimeout->timeout.tv_usec = now.tv_usec + timeout->tv_usec;
	eventtimeout->func = func;
	eventtimeout->data = data;
	array_append(event->timeouts, &eventtimeout);
	if(event->timeout.tv_sec > timeout->tv_sec
			|| (event->timeout.tv_sec == timeout->tv_sec
				&& event->timeout.tv_usec > timeout->tv_usec))
	{
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s%s%ld%s%ld%s", __func__, "() tv_sec=",
				(long)timeout->tv_sec, ", tv_usec=",
				(long)timeout->tv_usec, "\n");
#endif
		event->timeout.tv_sec = timeout->tv_sec;
		event->timeout.tv_usec = timeout->tv_usec;
	}
	return 0;
}


/* event_unregister_io_read */
static int _unregister_io(eventioArray * eios, fd_set * fds, int fd);

int event_unregister_io_read(Event * event, int fd)
{
	event->fdmax = _unregister_io(event->reads, &event->rfds, fd);
	event->fdmax = max(event->fdmax, _unregister_io(event->writes, NULL,
				-1));
	return 0;
}


/* event_unregister_io_write */
int event_unregister_io_write(Event * event, int fd)
{
	event->fdmax = _unregister_io(event->writes, &event->wfds, fd);
	event->fdmax = max(event->fdmax, _unregister_io(event->reads, NULL,
				-1));
	return 0;
}

static int _unregister_io(eventioArray * eios, fd_set * fds, int fd)
{
	unsigned int i = 0;
	EventIO * eio;
	int fdmax = -1;

	while(i < array_count(eios))
	{
		array_get_copy(eios, i, &eio);
		if(eio->fd != fd)
		{
			fdmax = max(fdmax, eio->fd);
			i++;
			continue;
		}
		FD_CLR(fd, fds);
		array_remove_pos(eios, i);
		object_delete(eio);
	}
	return fdmax;
}


/* event_unregister_timeout */
int event_unregister_timeout(Event * event, EventTimeoutFunc func)
{
	unsigned int i = 0;
	EventTimeout * et;
	struct timeval now;

	while(i < array_count(event->timeouts))
	{
		array_get_copy(event->timeouts, i, &et);
		if(et->func != func)
		{
			i++;
			continue;
		}
		array_remove_pos(event->timeouts, i);
		object_delete(et);
	}
	if(gettimeofday(&now, NULL) != 0)
		return error_set_code(1, "%s", strerror(errno));
	event->timeout.tv_sec = (time_t)LONG_MAX;
	event->timeout.tv_usec = (suseconds_t)LONG_MAX;
	for(i = 0; i < array_count(event->timeouts); i++)
	{
		array_get_copy(event->timeouts, i, &et);
		if(et->timeout.tv_sec < event->timeout.tv_sec
				|| (et->timeout.tv_sec == event->timeout.tv_sec
					&& et->timeout.tv_usec
					< event->timeout.tv_usec))
		{
			if((event->timeout.tv_sec = et->timeout.tv_sec
						- now.tv_sec) < 0)
			{
				event->timeout.tv_sec = 0;
				event->timeout.tv_usec = 0;
				break;
			}
			event->timeout.tv_usec = et->timeout.tv_usec
				- now.tv_usec;
			if(event->timeout.tv_usec >= 0)
				continue;
			event->timeout.tv_sec = max(0, event->timeout.tv_sec-1);
			event->timeout.tv_usec = -event->timeout.tv_usec;
		}
	}
	return 0;
}
