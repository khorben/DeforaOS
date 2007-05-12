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
	char const * lastfunc;
	void ** lastargs;
	int32_t * lastret;
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
	ac->buf_read_cnt += len;
#ifdef DEBUG
	fprintf(stderr, "%s%d%s", "appclient_read() ", len, " bytes\n");
#endif
	len = appinterface_call_receive(ac->interface, ac->buf_read,
			ac->buf_read_cnt, ac->lastret, ac->lastfunc,
			ac->lastargs);
	if(len < 0 || len > ac->buf_read_cnt)
	{
		/* FIXME report error */
		close(fd);
		return 1;
	}
	if(len == 0) /* EAGAIN */
		return 0;
	ac->ret = 0;
	ac->buf_read_cnt -= len;
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
	if(_new_connect(appclient, app) != 0)
	{
		free(appclient);
		return NULL;
	}
	return appclient;
}

static int _connect_addr(char * service, uint32_t * addr);
static int _new_connect(AppClient * appclient, char * app)
{
	struct sockaddr_in sa;
	int32_t port = -1;

	if((appclient->fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return 1;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(appinterface_get_port(appclient->interface));
	if(_connect_addr("Session", &sa.sin_addr.s_addr) != 0)
		return 1;
	if(connect(appclient->fd, (struct sockaddr *)&sa, sizeof(sa)) != 0)
		return 1;
	if(appclient_call(appclient, &port, "port", app) != 0)
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
	if(_connect_addr(app, &sa.sin_addr.s_addr) != 0)
		return 1;
	sa.sin_port = htons(port);
	if(connect(appclient->fd, (struct sockaddr *)&sa, sizeof(sa)) != 0)
		return 1;
	return 0;
}

static int _connect_addr(char * service, uint32_t * addr)
{
	char prefix[] = "APPSERVER_";
	int len = sizeof(prefix);
	char * env;
	char * server;
	struct hostent * he;

	if((env = malloc(len + string_length(service) + 1)) == NULL)
		return 1;
	sprintf(env, "%s%s", prefix, service);
	server = getenv(env);
	free(env);
	if(server == NULL)
	{
		*addr = htonl(INADDR_LOOPBACK);
		return 0;
	}
	if((he = gethostbyname(server)) == NULL)
		return 1;
	*addr = *((uint32_t*)he->h_addr);
	return 0;
}


/* appclient_delete */
void appclient_delete(AppClient * appclient)
{
	appinterface_delete(appclient->interface);
	if(appclient->fd != -1)
		close(appclient->fd);
	free(appclient);
}


/* useful */
/* appclient_call */
static int _call_event(AppClient * ac);

int appclient_call(AppClient * ac, int32_t * ret, char const * function, ...)
{
	int _ret;
	void ** args = NULL;
	va_list arg;
	size_t left = sizeof(ac->buf_write) - ac->buf_write_cnt;
	int i;

#ifdef DEBUG
	fprintf(stderr, "%s%p%s", "appclient_call(), interface ", ac->interface,
			"\n");
#endif
	if((i = appinterface_get_args_count(ac->interface, function)) < 0)
		return -1;
	if(i > 0 && (args = calloc(sizeof(*args), i)) == NULL)
		return -1;
	va_start(arg, function);
	i = appinterface_call(ac->interface, &ac->buf_write[ac->buf_write_cnt],
			left, function, args, arg);
	va_end(arg);
	if(i <= 0 || i > left)
	{
		free(args);
		return -1;
	}
	ac->lastfunc = function; /* XXX safe for now because synchronous only */
	ac->lastargs = args;
	ac->lastret = ret;
	ac->buf_write_cnt += i;
	_ret = _call_event(ac);
	free(ac->lastargs);
	return _ret;
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
