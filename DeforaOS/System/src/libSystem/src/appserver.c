/* appserver.c */



#include <stdlib.h>
#include "appserver.h"


/* AppServer */
struct _AppServer
{
	Event * event;
};


AppServer * appserver_new(const char * app)
{
	AppServer * appserver;

	if((appserver = malloc(sizeof(AppServer))) == NULL)
		return NULL;
	if((appserver->event = event_new()) == NULL)
	{
		free(appserver);
		return NULL;
	}
	/* FIXME initialize according to app */
	return appserver;
}


AppServer * appserver_new_event(const char * app, Event * event)
{
	AppServer * appserver;

	if((appserver = malloc(sizeof(AppServer))) == NULL)
		return NULL;
	appserver->event = event;
	/* FIXME initialize according to app */
	return appserver;
}


void appserver_delete(AppServer * appserver)
{
	event_delete(appserver->event);
	free(appserver);
}
