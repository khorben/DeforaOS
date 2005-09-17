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
ARRAY(EventTimeout *, eventtimeout);

typedef struct _EventIO
{
	int fd;
	EventIOFunc func;
	void * data;
} EventIO;
ARRAY(EventIO *, eventio);

struct _Event
{
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
static void _loop_timeout(Event * event);
static void _loop_io_read(Event * event, fd_set * rfds);
static void _loop_io_write(Event * event, fd_set * wfds);
int event_loop(Event * event)
{
	struct timeval * timeout = event->timeout.tv_sec == LONG_MAX
		&& event->timeout.tv_usec == LONG_MAX ? NULL : &event->timeout;
	fd_set rfds = event->rfds;
	fd_set wfds = event->wfds;
	int ret;

	while((ret = select(event->fdmax+1, &rfds, &wfds, NULL, timeout)) != -1)
	{
		_loop_timeout(event);
		_loop_io_read(event, &rfds);
		_loop_io_write(event, &wfds);
		timeout = event->timeout.tv_sec == LONG_MAX
			&& event->timeout.tv_usec == LONG_MAX
			? NULL : &event->timeout;
		rfds = event->rfds;
		wfds = event->wfds;
	}
	if(ret != -1)
		return 0;
#ifdef DEBUG
	sleep(1);
	perror("select");
#endif
	return 1;
}

static void _loop_timeout(Event * event)
{
	struct timeval now;
	unsigned int i = 0;

	if(gettimeofday(&now, NULL) != 0)
#ifdef DEBUG
		return perror("gettimeofday");
# else
		return;
#endif
	while(i < array_count(event->timeouts))
	{
		/* FIXME */
		i++;
	}
}

static int _io_fd_remove(Event * event, int fd);
static void _loop_io_read(Event * event, fd_set * rfds)
{
	unsigned int i = 0;
	EventIO * eio;

	while(i < array_count(event->reads))
	{
		array_get(event->reads, i, &eio);
#ifdef DEBUG
		fprintf(stderr, "%s%d%s%p%s", "_loop_reads(): i=", i,
				", eio=", eio, "\n");
#endif
		if(FD_ISSET(eio->fd, rfds)
				&& eio->func(eio->fd, eio->data) != 0)
		{
			event->fdmax = _io_fd_remove(event, eio->fd);
			continue;
		}
		i++;
	}
}

static void _loop_io_write(Event * event, fd_set * wfds)
{
	unsigned int i = 0;
	EventIO * eio;

	while(i < array_count(event->writes))
	{
		array_get(event->writes, i, &eio); /* FIXME check error */
#ifdef DEBUG
		fprintf(stderr, "%s%d%s%p%s", "_loop_writes(): i=", i,
				", eio=", eio, "\n");
#endif
		if(FD_ISSET(eio->fd, wfds)
				&& eio->func(eio->fd, eio->data) != 0)
		{
			event->fdmax = _io_fd_remove(event, eio->fd);
			continue;
		}
		i++;
	}
}

static int _io_fd_remove(Event * event, int fd)
{
	unsigned int i;
	int fdmax = -1;
	EventIO * eio;

	for(i = 0; i < array_count(event->reads); i++)
	{
		array_get(event->reads, i, &eio); /* FIXME check error */
		if(eio->fd != fd)
		{
			fdmax = max(fdmax, eio->fd);
			continue;
		}
		FD_CLR(eio->fd, &event->rfds);
		free(eio);
		array_remove_pos(event->reads, i);
	}
	for(i = 0; i < array_count(event->writes); i++)
	{
		array_get(event->writes, i, &eio); /* FIXME check error */
		if(eio->fd != fd)
		{
			fdmax = max(fdmax, eio->fd);
			continue;
		}
		FD_CLR(eio->fd, &event->wfds);
		free(eio);
		array_remove_pos(event->writes, i);
	}
	return fdmax;
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
	event->fdmax = max(event->fdmax, fd);
	FD_SET(fd, &event->rfds);
	array_append(event->reads, eventio);
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
	event->fdmax = max(event->fdmax, fd);
	FD_SET(fd, &event->wfds);
	array_append(event->writes, eventio);
	return 0;
}


/* event_register_timeout */
int event_register_timeout(Event * event, struct timeval timeout,
		EventTimeoutFunc func, void * data)
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
