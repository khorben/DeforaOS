/* appclient.c */



#include <sys/socket.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#ifdef DEBUG
# include <stdio.h>
#endif

#include "string.h"
#include "appinterface.h"
#include "appclient.h"


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
};


/* private */
static int _appclient_timeout(AppClient * appclient)
{
	static int cnt = 0;

#ifdef DEBUG
	fprintf(stderr, "%s%d%s", "appclient_timeout(): ", cnt, "\n");
#endif
	return cnt++ > 1000 || appclient->fd > -1; /* FIXME use a constant */
}


/* static int _appclient_read(int fd, AppClient * ac)
{
	ssize_t len;

	if((len = sizeof(ac->buf_read) - ac->buf_read_cnt) < 0
			|| (len = read(fd, &ac->buf_read[ac->buf_read_cnt],
					len)) <= 0)
	{
		/ * FIXME * /
		return 1;
	}
	ac->buf_read_cnt+=len;
	/ * FIXME * /
	return 0;
} */


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
		return 1;
	}
	memmove(ac->buf_write, &ac->buf_write[len], len);
	ac->buf_write_cnt-=len;
	return 0;
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
	fprintf(stderr, "%s%s%s", "appclient_new(", app, ")\n");
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

static int _new_connect(AppClient * appclient, char * app)
{
	struct sockaddr_in sa;
	int port;

	if((appclient->fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return 1;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(4242); /* FIXME */
	sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	if(connect(appclient->fd, &sa, sizeof(sa)) != 0)
		return 1;
	if((port = appclient_call(appclient, "port", app)) == -1)
		return 1;
	if(port == 0)
		return 0;
	close(appclient->fd);
	appinterface_delete(appclient->interface);
	if((appclient->interface = appinterface_new(app)) == NULL)
		return 1;
	if((appclient->fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return 1;
	sa.sin_port = htons(port);
	if(connect(appclient->fd, &sa, sizeof(sa)) != 0)
		return 1;
	return 0;
}


/* appclient_delete */
void appclient_delete(AppClient * appclient)
{
	free(appclient);
}


/* useful */
int appclient_call(AppClient * ac, char * function, ...)
{
	va_list arg;
	int i;
	void ** args = NULL;
	void ** p;
	struct timeval tv = { 0, 10 };

	va_start(arg, function);
	for(i = 0; i < 0 /* FIXME XXX */; i++)
	{
		if((p = realloc(args, i * sizeof(void*))) == NULL)
			break;
		args = p;
		args[i] = va_arg(arg, void *);
	}
	va_end(arg);
	if(appinterface_call(ac->interface, function,
				&ac->buf_write[ac->buf_write_cnt],
				sizeof(ac->buf_write) - ac->buf_write_cnt,
				args) != 0)
		return -1;
	event_register_timeout(ac->event, tv,
			(EventTimeoutFunc)_appclient_timeout, ac);
	event_register_io_write(ac->event, ac->fd,
			(EventIOFunc)_appclient_write, ac);
	event_loop(ac->event);
	return 0;
}
