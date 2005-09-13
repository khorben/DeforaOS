/* appserver.h */



#ifndef _APPSERVER_H
# define _APPSERVER_H

# include "event.h"


/* AppServer */
/* types */
typedef struct _AppServer AppServer;
enum _AppServerOptions
{
	ASO_LOCAL = 1,
	ASO_REMOTE = 2
} AppServerOptions;


/* functions */
AppServer * appserver_new(const char * app, int options);
AppServer * appserver_new_event(const char * app, int options, Event * event);
void appserver_delete(AppServer * appserver);

/* useful */
int appserver_loop(AppServer * appserver);

#endif /* !_APPSERVER_H */
