/* event.c */



#include <sys/time.h>
#include <time.h>
#include <sys/select.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include "array.h"
#include "event.h"


/* Event */
typedef struct _EventTimeout
{
	EventTimeoutFunc * func;
	void * data;
	struct timeval now;
	struct timeval timeout;
} EventTimeout;
ARRAY(EventTimeout, EventTimeout);

struct _Event
{
	fd_set rfds;
	fd_set wfds;
	EventTimeoutArray * timeouts;
	struct timeval now;
	struct timeval * timeout;
};


#ifdef DEBUG
static void _debug_timeval(struct timeval * tv, char * message)
{
	fprintf(stderr, "%s%s%lu%s%lu%s", message, ": ", tv->tv_sec, "s, ",
			tv->tv_usec, "us\n");
}
#endif


Event * event_new(void)
{
	Event * event;

	if((event = malloc(sizeof(Event))) == NULL)
		return NULL;
	if(gettimeofday(&event->now, NULL) != 0
			|| (event->timeouts = EventTimeoutArrayNew()) == NULL)
	{
		perror("event");
		free(event);
		return NULL;
	}
#ifdef DEBUG
	_debug_timeval(&event->now, "event_new()");
#endif
	FD_ZERO(&event->rfds);
	FD_ZERO(&event->wfds);
	event->timeout = NULL;
	return event;
}


void event_delete(Event * event)
{
	free(event);
}


/* internal */
static void _event_timeout_set(Event * event)
{
	static struct timeval tv;

	if(array_count(event->timeouts) == 0)
	{
		event->timeout = NULL;
		return;
	}
	if(gettimeofday(&tv, NULL) != 0)
		perror("gettimeofday");
	/* FIXME set event->timeout */
}

static int _event_timeout_hit(Event * event)
{
	if(event->timeout == NULL)
		return 0;
	/* FIXME look at every timeout func and run it if necessary */
	return 0;
}


/* useful */
int event_loop(Event * event)
{
	int ret;

	for(_event_timeout_set(event);
			(ret = select(0, &event->rfds, &event->wfds, NULL,
				      event->timeout)) != -1;
			_event_timeout_set(event))
	{
		if(_event_timeout_hit(event))
			continue;
		/* FIXME */
	}
	if(ret != -1)
		return 0;
	perror("select");
	return 1;
}


int event_timeout(Event * event, EventTimeoutFunc * func,
		struct timeval timeout, void * data)
{
	EventTimeout * eventtimeout;

	if((eventtimeout = malloc(sizeof(EventTimeout))) == NULL)
		return 1;
	eventtimeout->func = func;
	eventtimeout->timeout = timeout;
	eventtimeout->data = data;
	array_append(event->timeouts, eventtimeout);
	/* FIXME fast recompute next timeout */
	return 0;
}
