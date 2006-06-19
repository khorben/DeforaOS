/* appserver.h */



#ifndef LIBSYSTEM_APPSERVER_H
# define LIBSYSTEM_APPSERVER_H

# include "event.h"


/* AppServer */
/* types */
typedef struct _AppServer AppServer;
typedef enum _AppServerOptions
{
	ASO_LOCAL = 1,
	ASO_REMOTE = 2
} AppServerOptions;


/* functions */
AppServer * appserver_new(char const * app, int options);
AppServer * appserver_new_event(char const * app, int options, Event * event);
void appserver_delete(AppServer * appserver);

/* useful */
int appserver_loop(AppServer * appserver);

#endif /* !LIBSYSTEM_APPSERVER_H */
