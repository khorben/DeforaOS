/* appserver.h */



#ifndef _APPSERVER_H
# define _APPSERVER_H

# include "event.h"


/* AppServer */
/* types */
typedef struct _AppServer AppServer;


/* functions */
AppServer * appserver_new(const char * app);
AppServer * appserver_new_event(const char * app, Event * event);
void appserver_delete(AppServer * appserver);

#endif /* !_APPSERVER_H */
