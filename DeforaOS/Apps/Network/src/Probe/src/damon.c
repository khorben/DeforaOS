/* damon.c */



#include <System.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#define DAMON_REFRESH 5


/* DaMon */
static int _damon_error(char * message, int ret);
static int _damon_refresh(AppClient * appclient);
static int _damon(void)
{
	AppClient * appclient;
	Event * event;
	struct timeval tv;

	if((event = event_new()) == NULL)
		return _damon_error("Event", 2);
	if((appclient = appclient_new("Probe")) == NULL)
	{
		event_delete(event);
		return _damon_error("AppClient", 2);
	}
	tv.tv_sec = DAMON_REFRESH;
	tv.tv_usec = 0;
	fprintf(stderr, "appclient = %p;\n", appclient);
	event_register_timeout(event, tv, (EventTimeoutFunc)_damon_refresh,
			appclient);
	_damon_refresh(appclient);
	if(event_loop(event) != 0)
		_damon_error("AppClient", 0);
	appclient_delete(appclient);
	event_delete(event);
	return 2;
}

static int _damon_error(char * message, int ret)
{
	fprintf(stderr, "%s", "DaMon: ");
	perror(message);
	return ret;
}

static int _rrd_update(char * file, int args_cnt, ...);
static int _damon_refresh(AppClient * appclient)
{
	_rrd_update("uptime.rrd", 1, appclient_call(appclient, "uptime", 0));
	_rrd_update("load.rrd", 3, appclient_call(appclient, "load1", 0),
			appclient_call(appclient, "load5", 0),
			appclient_call(appclient, "load15", 0));
	return 0;
}

static int _exec(char * argv[]);
static int _rrd_update(char * file, int args_cnt, ...)
{
	char * argv[] = { "rrdtool", "update", file, NULL, NULL };
	struct timeval tv;
	int pos;
	int i;
	va_list args;
	int ret;

	if(gettimeofday(&tv, NULL) != 0)
		return _damon_error("gettimeofday", -1);
	if((argv[3] = malloc((args_cnt+1) * 12)) == NULL)
		return _damon_error("malloc", -1);
	pos = sprintf(argv[3], "%ld", tv.tv_sec);
	va_start(args, args_cnt);
	for(i = 0; i < args_cnt; i++)
		pos+=sprintf(&argv[3][pos], ":%d", va_arg(args, int));
	va_end(args);
	ret = _exec(argv);
	free(argv[3]);
	return ret;
}

static int _exec(char * argv[])
{
	pid_t pid;
	int status;
	int ret;

	if((pid = fork()) == -1)
		return _damon_error("fork", -1);
	if(pid == 0)
	{
		execvp(argv[0], argv);
		exit(_damon_error(argv[0], 2));
	}
	while(*argv != NULL)
		fprintf(stderr, "%s ", *argv++);
	fprintf(stderr, "\n");
	while((ret = waitpid(pid, &status, 0)) != -1)
		if(WIFEXITED(status))
			break;
	if(ret == -1)
		return _damon_error("waitpid", -1);
	return WEXITSTATUS(status);
}


/* main */
int main(int argc, char * argv[])
{
	return _damon();
}
