/* damon.c */



#include <System.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#define DAMON_REFRESH 10


/* types */
typedef struct _Host
{
	AppClient * appclient;
	char * hostname;
} Host;


/* DaMon */
static int _damon_error(char * message, int ret);
static int _damon_refresh(Host * hosts);
static int _damon(void)
{
	Host hosts[] = {
		{ NULL, "pinge.lan.defora.org" },
		{ NULL, "rst.defora.org" },
		{ NULL, "raq3.dmz.defora.org" },
		{ NULL, "raq4.dmz.defora.org" },
/*		{ NULL, "ss20.dmz.defora.org" }, */
		{ NULL, NULL }
	};
	Event * event;
	struct timeval tv;
	int i;
	int j;

	if((event = event_new()) == NULL)
		return _damon_error("Event", 2);
	for(i = 0; hosts[i].hostname != NULL; i++)
	{
		if(setenv("APPSERVER_Probe", hosts[i].hostname, 1) != 0)
			break;
		if((hosts[i].appclient = appclient_new_event("Probe", event))
				== NULL)
			break;
		fprintf(stderr, "AppClientNew => %p\n", hosts[i].appclient);
	}
	if(hosts[i].hostname == NULL)
	{
		_damon_refresh(hosts);
		tv.tv_sec = DAMON_REFRESH;
		tv.tv_usec = 0;
		event_register_timeout(event, tv,
				(EventTimeoutFunc)_damon_refresh, hosts);
		if(event_loop(event) != 0)
			_damon_error("AppClient", 0);
	}
	for(j = 0; j < i; j++)
		appclient_delete(hosts[j].appclient);
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
static int _damon_refresh(Host * hosts)
{
	int i;
	AppClient * ac = NULL;
	char * rrd = NULL;
	char * p;

	fprintf(stderr, "%s", "_damon_refresh()\n");
	for(i = 0; (ac = hosts[i].appclient) != NULL; i++)
	{
		if((p = realloc(rrd, string_length(hosts[i].hostname) + 12))
				== NULL)
			break;
		rrd = p;
		sprintf(rrd, "%s/%s", hosts[i].hostname, "uptime.rrd");
		_rrd_update(rrd, 1, appclient_call(ac, "uptime", 0));
		sprintf(rrd, "%s/%s", hosts[i].hostname, "load.rrd");
		_rrd_update(rrd, 3, appclient_call(ac, "load1", 0),
				appclient_call(ac, "load5", 0),
				appclient_call(ac, "load15", 0));
		sprintf(rrd, "%s/%s", hosts[i].hostname, "procs.rrd");
		_rrd_update(rrd, 1, appclient_call(ac, "procs", 0));
	}
	free(rrd);
	if(ac != NULL)
		fprintf(stderr, "%s", "DaMon: refresh: An error occured\n");
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
