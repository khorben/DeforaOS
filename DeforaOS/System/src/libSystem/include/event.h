/* event.h */



#ifndef _EVENT_H
# define _EVENT_H

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

#endif /* !_EVENT_H */
