/* appclient.c */



#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>

#include "System.h"
#include "appinterface.h"


/* AppClient */
/* private */
/* types */
struct _AppClient
{
	AppInterface * interface;
	Event * event;
	int fd;
#define ASC_BUFSIZE 65536 /* FIXME */
	char buf_read[ASC_BUFSIZE];
	int buf_read_cnt;
	char buf_write[ASC_BUFSIZE];
	int buf_write_cnt;
	int ret;
	char * lastfunc;
	void ** lastargs;
};


/* private */
static int _appclient_timeout(AppClient * appclient)
{
#ifdef DEBUG
	fprintf(stderr, "%s", "appclient_timeout()\n");
#endif
	close(appclient->fd);
	return 1;
}


static int _appclient_read(int fd, AppClient * ac)
{
	ssize_t len;

#ifdef DEBUG
	fprintf(stderr, "%s%d%s", "appclient_read(", fd, ")\n");
#endif
	if((len = (sizeof(ac->buf_read) - ac->buf_read_cnt)) < 0
			|| (len = read(fd, &ac->buf_read[ac->buf_read_cnt],
					len)) <= 0)
	{
		/* FIXME */
		close(fd);
		return 1;
	}
	ac->buf_read_cnt+=len;
#ifdef DEBUG
	fprintf(stderr, "%s%d%s", "appclient_read() ", len, " bytes\n");
#endif
	len = appinterface_call_receive(ac->interface, &ac->ret, ac->lastfunc,
			ac->lastargs, ac->buf_read, ac->buf_read_cnt);
	if(len < 0)
	{
		/* FIXME */
		close(fd);
		return 1;
	}
	if(len == 0)
		return 0;
	ac->buf_read_cnt-=len;
	event_unregister_timeout(ac->event,
			(EventTimeoutFunc)_appclient_timeout);
	return 1;
}


static int _appclient_write(int fd, AppClient * ac)
{
	ssize_t len;

#ifdef DEBUG
	fprintf(stderr, "%s%d%s", "appclient_write(", fd, ")\n");
#endif
	len = ac->buf_write_cnt;
	if((len = write(fd, ac->buf_write, len)) <= 0)
	{
		/* FIXME */
		close(fd);
		return 1;
	}
	memmove(ac->buf_write, &ac->buf_write[len], len);
	ac->buf_write_cnt-=len;
	if(ac->buf_write_cnt > 0)
		return 0;
	event_register_io_read(ac->event, fd,
			(EventIOFunc)_appclient_read, ac);
	return 1;
}


/* public */
/* functions */
AppClient * appclient_new(char * app)
{
	AppClient * appclient;
	Event * event;

	if((event = event_new()) == NULL)
		return NULL;
	if((appclient = appclient_new_event(app, event)) == NULL)
	{
		event_delete(event);
		return NULL;
	}
	return appclient;
}


static int _new_connect(AppClient * appclient, char * app);
AppClient * appclient_new_event(char * app, Event * event)
{
	AppClient * appclient;

#ifdef DEBUG
	fprintf(stderr, "%s%s%s", "appclient_new(\"", app, "\")\n");
#endif
	if((appclient = malloc(sizeof(AppClient))) == NULL)
		return NULL;
	if((appclient->interface = appinterface_new("Session")) == NULL)
	{
		free(appclient);
		return NULL;
	}
	appclient->event = event;
	appclient->fd = -1;
	appclient->buf_read_cnt = 0;
	appclient->buf_write_cnt = 0;
	appclient->ret = -1;
	if(_new_connect(appclient, app) != 0)
	{
		free(appclient);
		return NULL;
	}
	return appclient;
}

static uint32_t _connect_addr(char * service);
static int _new_connect(AppClient * appclient, char * app)
{
	struct sockaddr_in sa;
	int port;

	if((appclient->fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return 1;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(appinterface_port(appclient->interface));
	sa.sin_addr.s_addr = _connect_addr("Session");
	if(connect(appclient->fd, (struct sockaddr *)&sa, sizeof(sa)) != 0)
		return 1;
	if((port = appclient_call(appclient, "port", 1, app)) == -1)
		return 1;
	if(port == 0)
		return 0;
	close(appclient->fd);
#ifdef DEBUG
	fprintf(stderr, "%s%d%s", "AppClient bouncing to port ", port, "\n");
#endif
	appinterface_delete(appclient->interface);
	if((appclient->interface = appinterface_new(app)) == NULL)
		return 1;
	if((appclient->fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return 1;
	sa.sin_addr.s_addr = _connect_addr(app);
	sa.sin_port = htons(port);
	if(connect(appclient->fd, (struct sockaddr *)&sa, sizeof(sa)) != 0)
		return 1;
	return 0;
}

static uint32_t _connect_addr(char * service)
{
	char prefix[] = "APPSERVER_";
	int len = sizeof(prefix);
	char * env;
	char * appserver;
	struct hostent * he;

	if((env = malloc(len + string_length(service) + 1)) == NULL)
		return htonl(INADDR_LOOPBACK);
	sprintf(env, "%s%s", prefix, service);
	appserver = getenv(env);
	free(env);
	if(appserver == NULL || (he = gethostbyname(appserver)) == NULL)
		/* FIXME this is an error case */
		return htonl(INADDR_LOOPBACK);
	return *((uint32_t*)(he->h_addr));
}


/* appclient_delete */
void appclient_delete(AppClient * appclient)
{
	appinterface_delete(appclient->interface);
	free(appclient);
}


/* useful */
static int _call_event(AppClient * ac);
int appclient_call(AppClient * ac, char * function, int args_cnt, ...)
{
	va_list arg;
	int i;
	void ** args = NULL;

#ifdef DEBUG
	fprintf(stderr, "%s%p%s", "appclient_call(), interface ", ac->interface,
			"\n");
#endif
	if((args = malloc(sizeof(void*) * args_cnt)) == NULL)
		return -1;
	va_start(arg, args_cnt);
	for(i = 0; i < args_cnt; i++)
		args[i] = va_arg(arg, void *);
	va_end(arg);
	i = appinterface_call(ac->interface, function,
			&ac->buf_write[ac->buf_write_cnt],
			sizeof(ac->buf_write) - ac->buf_write_cnt, args);
	if(i <= 0)
	{
		free(args);
		return -1;
	}
	ac->lastfunc = function;
	ac->lastargs = args;
	ac->buf_write_cnt+=i;
	return _call_event(ac);
}

static int _call_event(AppClient * ac)
{
	Event * eventtmp;
	struct timeval tv = { 10, 0 };

	eventtmp = ac->event;
	ac->event = event_new();
	ac->ret = -1;
	event_register_timeout(ac->event, tv,
			(EventTimeoutFunc)_appclient_timeout, ac);
	event_register_io_write(ac->event, ac->fd,
			(EventIOFunc)_appclient_write, ac);
#ifdef DEBUG
	fprintf(stderr, "%s", "AppClient looping in wait for answer()\n");
#endif
	event_loop(ac->event);
	event_delete(ac->event);
	ac->event = eventtmp;
	return ac->ret;
}
