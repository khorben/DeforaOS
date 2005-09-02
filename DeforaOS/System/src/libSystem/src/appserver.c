/* appserver.c */



#include <sys/socket.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <array.h>
#include <string.h>
#include "appserver.h"


/* AppServerClient */
/* types */
typedef enum _AppServerClientState
{
	ASCS_NEW,
	ASCS_LOGGED
} AppServerClientState;

typedef struct _AppServerClient
{
	AppServerClientState state;
	int fd;
	uint32_t addr; /* FIXME uint8_t[4] instead? */
	uint16_t port;
} AppServerClient;


/* functions */
AppServerClient * appserverclient_new(int fd, uint32_t addr, uint16_t port)
{
	AppServerClient * asc;

	if((asc = malloc(sizeof(AppServerClient))) == NULL)
		return NULL;
	asc->state = ASCS_NEW;
	asc->fd = fd;
	asc->addr = addr;
	asc->port = port;
	return asc;
}

void appserverclient_delete(AppServerClient * appserverclient)
{
	if(close(appserverclient->fd) != 0)
		perror("close"); /* FIXME find a way to properly report error */
	free(appserverclient);
}


/* AppServer */
/* private */
/* types */
ARRAY(AppServerClient, AppServerClient);
struct _AppServer
{
	Event * event;
	uint16_t port;
	AppServerClientArray * clients;
};


/* functions */
static void _appserver_accept(int fd, AppServer * appserver)
{
	struct sockaddr_in sa;
	int sa_size = sizeof(struct sockaddr_in);
	int fd2;
	AppServerClient * asc;

	/* FIXME append client to the clients list with the appropriate state */
	if((fd2 = accept(fd, (struct sockaddr *)&sa, &sa_size)) == -1)
		return perror("accept"); /* FIXME report error */
	if((asc = appserverclient_new(fd2, sa.sin_addr.s_addr, sa.sin_port))
			== NULL)
		return;
	array_append(appserver->clients, asc);
}


/* public */
/* functions */
/* appserver_new */
static int _new_interface(AppServer * appserver, const char * app);
static int _new_server(AppServer * appserver, int options);
AppServer * appserver_new(const char * app, int options)
{
	AppServer * appserver;

	if((appserver = malloc(sizeof(AppServer))) == NULL)
		return NULL;
	if((appserver->event = event_new()) == NULL)
	{
		free(appserver);
		return NULL;
	}
	if(_new_interface(appserver, app) != 0
			|| _new_server(appserver, options) != 0)
	{
		event_delete(appserver->event);
		free(appserver);
		return NULL;
	}
	return appserver;
}


/* appserver_new_event */
AppServer * appserver_new_event(const char * app, int options, Event * event)
{
	AppServer * appserver;

	if((appserver = malloc(sizeof(AppServer))) == NULL)
		return NULL;
	appserver->event = event;
	if(_new_interface(appserver, app) != 0
			|| _new_server(appserver, options) != 0)
	{
		free(appserver);
		return NULL;
	}
	return appserver;
}

static int _new_interface(AppServer * appserver, const char * app)
{
	/* FIXME interfaces are hardcoded */
	if(string_compare(app, "Session") == 0)
	{
		appserver->port = 4242; /* FIXME */
		return 0;
	}
	return 1;
}

static int _new_server(AppServer * appserver, int options)
{
	int fd;
	struct sockaddr_in sa;

	if((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return 1; /* FIXME report error */
	sa.sin_family = AF_INET;
	sa.sin_port = htons(appserver->port);
	sa.sin_addr.s_addr = options & ASO_LOCAL ? INADDR_LOOPBACK : INADDR_ANY;
	if(bind(fd, (struct sockaddr *)&sa, sizeof(sa)) != 0
			|| listen(fd, 5) != 0)
	{
		/* FIXME report error */
		if(close(fd) != 0)
			perror("close"); /* FIXME report error appropriately */
		return 1;
	}
	event_register_fd_read(appserver->event, fd, _appserver_accept);
	return 0;
}


/* appserver_delete */
void appserver_delete(AppServer * appserver)
{
	event_delete(appserver->event);
	free(appserver);
}
