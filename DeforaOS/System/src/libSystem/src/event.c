/* event.c */



#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h>
#ifdef DEBUG
# include <stdio.h>
#endif

#include "array.h"
#include "event.h"

#define max(a, b) ((a) >= (b)) ? (a) : (b)


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
	event->fdmax = -1;
	FD_ZERO(&event->rfds);
	FD_ZERO(&event->wfds);
	event->reads = eventioarray_new(); /* FIXME */
	event->writes = eventioarray_new(); /* FIXME */
	event->timeout.tv_sec = LONG_MAX;
	event->timeout.tv_usec = LONG_MAX;
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
static void _loop_io(Event * event, eventioArray * eios, fd_set * fds);
int event_loop(Event * event)
{
	struct timeval * timeout = event->timeout.tv_sec == LONG_MAX
		&& event->timeout.tv_usec == LONG_MAX ? NULL : &event->timeout;
	fd_set rfds = event->rfds;
	fd_set wfds = event->wfds;
	int ret = 0;

	while(!(timeout == NULL && event->fdmax == -1)
			&& (ret = select(event->fdmax+1, &rfds, &wfds, NULL,
					timeout)) != -1)
	{
		_loop_timeout(event);
		_loop_io(event, event->reads, &rfds);
		_loop_io(event, event->writes, &wfds);
		if(event->timeout.tv_sec == LONG_MAX
				&& event->timeout.tv_usec == LONG_MAX)
			timeout = NULL;
		else
			timeout = &event->timeout;
		rfds = event->rfds;
		wfds = event->wfds;
	}
	if(ret != -1)
		return 0;
#ifdef DEBUG
	fprintf(stderr, "%s", "event_loop(): ");
	perror("select");
	sleep(1);
#endif
	return 1;
}

static void _loop_timeout(Event * event)
{
	struct timeval now;
	unsigned int i = 0;
	EventTimeout * et;

#ifdef DEBUG
	fprintf(stderr, "%s", "_loop_timeout()\n");
#endif
	if(gettimeofday(&now, NULL) != 0)
#ifdef DEBUG
		return perror("gettimeofday");
# else
		return;
#endif
	event->timeout.tv_sec = LONG_MAX;
	event->timeout.tv_usec = LONG_MAX;
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
				free(et);
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
	fprintf(stderr, "%s%ld%s%ld%s", "_loop_timeout() tv_sec=",
			event->timeout.tv_sec, ", tv_usec=",
			event->timeout.tv_usec, "\n");
#endif
}

static void _loop_io(Event * event, eventioArray * eios, fd_set * fds)
{
	unsigned int i = 0;
	EventIO * eio;
	int fd;

#ifdef DEBUG
	fprintf(stderr, "%s", "_loop_io()\n");
#endif
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
				fprintf(stderr, "%s%s%d%s", __FILE__, ", ",
						__LINE__,
						": should not happen\n");
#endif
		}
		else
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

	if((eventio = malloc(sizeof(EventIO))) == NULL)
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
int event_register_timeout(Event * event, struct timeval timeout,
		EventTimeoutFunc func, void * data)
{
	EventTimeout * eventtimeout;
	struct timeval now;

	if(gettimeofday(&now, NULL) != 0)
		return 1;
	if((eventtimeout = malloc(sizeof(EventTimeout))) == NULL)
		return 1;
	eventtimeout->initial.tv_sec = timeout.tv_sec;
	eventtimeout->initial.tv_usec = timeout.tv_usec;
	eventtimeout->timeout.tv_sec = now.tv_sec + timeout.tv_sec;
	eventtimeout->timeout.tv_usec = now.tv_usec + timeout.tv_usec;
	eventtimeout->func = func;
	eventtimeout->data = data;
	array_append(event->timeouts, &eventtimeout);
	if(event->timeout.tv_sec > timeout.tv_sec
			|| (event->timeout.tv_sec == timeout.tv_sec
				&& event->timeout.tv_usec > timeout.tv_usec))
	{
#ifdef DEBUG
		fprintf(stderr, "%s%ld%s%ld%s",
				"event_register_timeout() tv_sec=",
				timeout.tv_sec, ", tv_usec=", timeout.tv_usec,
				"\n");
#endif
		event->timeout.tv_sec = timeout.tv_sec;
		event->timeout.tv_usec = timeout.tv_usec;
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
		free(eio);
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
		free(et);
	}
	if(gettimeofday(&now, NULL) != 0)
		return 1;
	event->timeout.tv_sec = LONG_MAX;
	event->timeout.tv_usec = LONG_MAX;
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
