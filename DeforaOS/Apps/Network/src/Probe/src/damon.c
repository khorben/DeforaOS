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
	char * iface;
} Host;


/* DaMon */
typedef struct _Hosts_Event
{
	Host * hosts;
	Event * event;
} Hosts_Event;
static int _damon_error(char * message, int ret);
static int _damon_refresh(Hosts_Event * h_e);
static int _damon(void)
{
	Host hosts[] = {
		{ NULL, "pinge.lan.defora.org", "eth0" },
		{ NULL, "rst.lan.defora.org", NULL },
		{ NULL, "raq3.dmz.defora.org", "eth0" },
		{ NULL, "raq4.dmz.defora.org", "eth0" },
/*		{ NULL, "ss20.dmz.defora.org" }, */
		{ NULL, NULL, NULL }
	};
	Hosts_Event h_e;
	struct timeval tv;
	int i;

	if((h_e.event = event_new()) == NULL)
		return _damon_error("Event", 1);
	h_e.hosts = hosts;
	_damon_refresh(&h_e);
	tv.tv_sec = DAMON_REFRESH;
	tv.tv_usec = 0;
	event_register_timeout(h_e.event, tv,
			(EventTimeoutFunc)_damon_refresh, &h_e);
	if(event_loop(h_e.event) != 0)
		_damon_error("AppClient", 0);
	for(i = 0; hosts[i].hostname != NULL; i++)
		if(hosts[i].appclient != NULL)
			appclient_delete(hosts[i].appclient);
	event_delete(h_e.event);
	return 1;
}

static int _damon_error(char * message, int ret)
{
	fprintf(stderr, "%s", "DaMon: ");
	perror(message);
	return ret;
}

static AppClient * _refresh_connect(Host * host, Event * event);
static int _refresh_uptime(AppClient * ac, Host * host, char * rrd);
static int _refresh_load(AppClient * ac, Host * host, char * rrd);
static int _refresh_ram(AppClient * ac, Host * host, char * rrd);
static int _refresh_swap(AppClient * ac, Host * host, char * rrd);
static int _refresh_procs(AppClient * ac, Host * host, char * rrd);
static int _refresh_users(AppClient * ac, Host * host, char * rrd);
static int _refresh_if(AppClient * ac, Host * host, char * rrd);
static int _damon_refresh(Hosts_Event * h_e)
{
	int i;
	AppClient * ac = NULL;
	char * rrd = NULL;
	char * p;
	Host * hosts = h_e->hosts;

	fprintf(stderr, "%s", "_damon_refresh()\n");
	for(i = 0; hosts[i].hostname != NULL; i++)
	{
		if((ac = hosts[i].appclient) == NULL)
			if((ac = _refresh_connect(&hosts[i], h_e->event))
					== NULL)
				continue;
		if((p = realloc(rrd, string_length(hosts[i].hostname) + 12))
				== NULL)
			break;
		rrd = p;
		if(_refresh_uptime(ac, &hosts[i], rrd) != 0
				|| _refresh_load(ac, &hosts[i], rrd) != 0
				|| _refresh_ram(ac, &hosts[i], rrd) != 0
				|| _refresh_swap(ac, &hosts[i], rrd) != 0
				|| _refresh_procs(ac, &hosts[i], rrd) != 0
				|| _refresh_users(ac, &hosts[i], rrd) != 0
				|| _refresh_if(ac, &hosts[i], rrd) != 0)
		{
			appclient_delete(ac);
			hosts[i].appclient = NULL;
			continue;
		}
		ac = NULL;
	}
	free(rrd);
	if(ac != NULL)
		fprintf(stderr, "%s", "DaMon: refresh: An error occured\n");
	return 0;
}

static AppClient * _refresh_connect(Host * host, Event * event)
{
	if(setenv("APPSERVER_Probe", host->hostname, 1) != 0)
		return NULL;
	if((host->appclient = appclient_new_event("Probe", event))
			== NULL)
		_damon_error(host->hostname, 0);
	return host->appclient;
}

static int _rrd_update(char * file, int args_cnt, ...);
static int _refresh_uptime(AppClient * ac, Host * host, char * rrd)
{
	int res;

	if((res = appclient_call(ac, "uptime", 0)) == -1)
		return 1;
	sprintf(rrd, "%s_%s", host->hostname, "uptime.rrd");
	_rrd_update(rrd, 1, res);
	return 0;
}

static int _refresh_load(AppClient * ac, Host * host, char * rrd)
{
	int res[3];

	if((res[0] = appclient_call(ac, "load_1", 0)) == -1
			|| (res[1] = appclient_call(ac, "load_5", 0)) == -1
			|| (res[2] = appclient_call(ac, "load_15", 0)) == -1)
		return 1;
	sprintf(rrd, "%s_%s", host->hostname, "load.rrd");
	_rrd_update(rrd, 3, res[0], res[1], res[2]);
	return 0;
}

static int _refresh_procs(AppClient * ac, Host * host, char * rrd)
{
	int res;

	if((res = appclient_call(ac, "procs", 0)) == -1)
		return 1;
	sprintf(rrd, "%s_%s", host->hostname, "procs.rrd");
	_rrd_update(rrd, 1, res);
	return 0;
}

static int _refresh_ram(AppClient * ac, Host * host, char * rrd)
{
	int res[4];

	if((res[0] = appclient_call(ac, "ram_total", 0)) == -1
			|| (res[1] = appclient_call(ac, "ram_free", 0)) == -1
			|| (res[2] = appclient_call(ac, "ram_shared", 0)) == -1
			|| (res[3] = appclient_call(ac, "ram_buffer", 0)) == -1)
		return 1;
	sprintf(rrd, "%s_%s", host->hostname, "ram.rrd");
	_rrd_update(rrd, 4, res[0], res[1], res[2], res[3]);
	return 0;
}

static int _refresh_swap(AppClient * ac, Host * host, char * rrd)
{
	int res[2];

	if((res[0] = appclient_call(ac, "swap_total", 0)) == -1
			|| (res[1] = appclient_call(ac, "swap_free", 0)) == -1)
		return 1;
	sprintf(rrd, "%s_%s", host->hostname, "swap.rrd");
	_rrd_update(rrd, 2, res[0], res[1]);
	return 0;
}

static int _refresh_users(AppClient * ac, Host * host, char * rrd)
{
	int res[2];

	if((res[0] = appclient_call(ac, "users", 0)) == -1)
		return 1;
	sprintf(rrd, "%s_%s", host->hostname, "users.rrd");
	_rrd_update(rrd, 1, res[0]);
	return 0;
}

static int _refresh_if(AppClient * ac, Host * host, char * rrd)
{
	int res[2];
	char * iface;

	if((iface = host->iface) == NULL)
		return 0;
	if((res[0] = appclient_call(ac, "ifrxbytes", 1, iface)) == -1
			|| (res[1] = appclient_call(ac, "iftxbytes", 1, iface))
			== -1)
		return 1;
	sprintf(rrd, "%s_%s%s", host->hostname, iface, ".rrd");
	_rrd_update(rrd, 2, res[0], res[1]);
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
		pos+=sprintf(&argv[3][pos], ":%u", va_arg(args, int));
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
int main(void)
{
	return _damon() == 0 ? 0 : 2;
}
