/* appserver.h */



#ifndef _APPSERVER_H_
# define _APPSERVER_H_

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
AppServer * appserver_new(char const * app, int options);
AppServer * appserver_new_event(char const * app, int options, Event * event);
void appserver_delete(AppServer * appserver);

/* useful */
int appserver_loop(AppServer * appserver);

#endif /* !_APPSERVER_H_ */
