/* event.h */



#ifndef _EVENT_H
# define _EVENT_H

# include <sys/time.h>
# include <time.h>


/* Event */
/* types */
typedef struct _Event Event;

typedef void (*EventFunc)(int fd, void * data);
typedef void (*EventTimeoutFunc)(void * data);


/* functions */
Event * event_new(void);
void event_delete(Event * event);

/* useful */
int event_loop(Event * event);
int event_register_fd_read(Event * event, int fd, EventFunc * func);
int event_register_timeout(Event * event, EventTimeoutFunc * func,
		struct timeval timeout, void * userdata);

#endif /* !_EVENT_H */
