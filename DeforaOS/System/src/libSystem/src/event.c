/* event.c */



#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>

#include "array.h"
#include "event.h"

#define max(a, b) ((a) >= (b)) ? (a) : (b)


/* Event */
/* private */
/* types */
typedef struct _EventTimeout
{
	struct timeval timeout;
	EventTimeoutFunc func;
	void * data;
} EventTimeout;
ARRAY(EventTimeout, eventtimeout);

typedef struct _EventIO
{
	int fd;
	EventIOFunc func;
	void * data;
} EventIO;
ARRAY(EventIO, eventio);

struct _Event
{
	fd_set rfds;
	fd_set wfds;
	EventTimeoutArray * timeouts;
	struct timeval timeout;
	EventIOArray * reads;
	EventIOArray * writes;
	int fdmax;
};


/* public */
/* functions */
/* event_new */
Event * event_new(void)
{
	Event * event;

	if((event = malloc(sizeof(Event))) == NULL)
		return NULL;
	if((event->timeouts = eventtimeoutarray_new()) == NULL)
	{
		free(event);
		return NULL;
	}
	event->timeout.tv_sec = LONG_MAX;
	event->timeout.tv_usec = LONG_MAX;
	event->reads = eventioarray_new(); /* FIXME */
	event->writes = eventioarray_new(); /* FIXME */
	FD_ZERO(&event->rfds);
	FD_ZERO(&event->wfds);
	return event;
}


/* event_delete */
void event_delete(Event * event)
{
	array_delete(event->timeouts);
	array_delete(event->reads);
	array_delete(event->writes);
	free(event);
}


/* useful */
static void _loop_timeouts(Event * event);
static void _loop_reads(Event * event);
static void _loop_writes(Event * event);
int event_loop(Event * event)
{
	struct timeval * timeout = event->timeout.tv_sec == LONG_MAX
		&& event->timeout.tv_usec == LONG_MAX ? NULL : &event->timeout;
	int ret;

	for(; (ret = select(event->fdmax+1, &event->rfds, &event->wfds, NULL,
					timeout)) != -1;
			timeout = event->timeout.tv_sec == LONG_MAX
			&& event->timeout.tv_usec == LONG_MAX
			? NULL : &event->timeout)
	{
		_loop_timeouts(event);
		_loop_reads(event);
		_loop_writes(event);
	}
	if(ret != -1)
		return 0;
	perror("select");
	return 1;
}

static void _loop_timeouts(Event * event)
{
	struct timeval now;
	unsigned int i = 0;

	if(gettimeofday(&now, NULL) != 0)
		return perror("gettimeofday");
	while(i < array_count(event->timeouts))
	{
		/* FIXME */
		i++;
	}
}

static void _loop_reads(Event * event)
{
	unsigned int i = 0;
	EventIO * eio;

	while(i < array_count(event->reads))
	{
		array_get(event->reads, i, &eio);
		if(FD_ISSET(eio->fd, &event->rfds)
				&& eio->func(eio->fd, eio->data) != 0)
		{
			array_remove_pos(event->reads, i);
			continue;
		}
		i++;
	}
}

static void _loop_writes(Event * event)
{
	unsigned int i = 0;
	EventIO * eio;

	while(i < array_count(event->writes))
	{
		array_get(event->writes, i, &eio);
		if(FD_ISSET(eio->fd, &event->wfds)
				&& eio->func(eio->fd, eio->data) != 0)
		{
			array_remove_pos(event->writes, i);
			continue;
		}
		i++;
	}
}


/* event_register_io_read */
int event_register_io_read(Event * event, int fd, EventIOFunc func,
		void * userdata)
{
	EventIO * eventio;

	if((eventio = malloc(sizeof(EventIO))) == NULL)
		return 1;
	eventio->fd = fd;
	eventio->func = func;
	eventio->data = userdata;
	array_append(event->reads, eventio);
	event->fdmax = max(event->fdmax, fd);
	return 0;
}


/* event_register_io_write */
int event_register_io_write(Event * event, int fd, EventIOFunc func,
		void * userdata)
{
	EventIO * eventio;

	if((eventio = malloc(sizeof(EventIO))) == NULL)
		return 1;
	eventio->fd = fd;
	eventio->func = func;
	eventio->data = userdata;
	array_append(event->writes, eventio);
	event->fdmax = max(event->fdmax, fd);
	return 0;
}


/* event_register_timeout */
int event_register_timeout(Event * event, struct timeval timeout,
		EventTimeoutFunc * func, void * data)
{
	EventTimeout * eventtimeout;

	if((eventtimeout = malloc(sizeof(EventTimeout))) == NULL)
		return 1;
	eventtimeout->timeout = timeout;
	eventtimeout->func = func;
	eventtimeout->data = data;
	array_append(event->timeouts, eventtimeout);
	if(event->timeout.tv_sec > timeout.tv_sec
			|| (event->timeout.tv_sec == timeout.tv_sec
				&& event->timeout.tv_usec > timeout.tv_usec))
	{
		event->timeout.tv_sec = timeout.tv_sec;
		event->timeout.tv_usec = timeout.tv_usec;
	}
	return 0;
}
