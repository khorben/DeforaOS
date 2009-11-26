/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Network Probe */
/* Probe is not free software; you can redistribute it and/or modify it under
 * the terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * Probe is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with Probe; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */
/* FIXME: catch SIGPIPE, determine if we can avoid catching it if an AppServer
 * exits (eg avoid writing to a closed socket) */



#include <System.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "../config.h"

#ifndef PREFIX
# define PREFIX "/usr/local"
#endif
#ifndef ETCDIR
# define ETCDIR PREFIX "/etc"
#endif


/* DaMon */
/* private */
/* types */
typedef struct _DaMon DaMon;

typedef struct _Host
{
	DaMon * damon;
	AppClient * appclient;
	char * hostname;
	char ** ifaces;
	char ** vols;
} Host;

struct _DaMon
{
	char const * prefix;
	unsigned int refresh;
	Host * hosts;
	unsigned int hosts_cnt;
	Event * event;
};


/* constants */
#define DAMON_DEFAULT_REFRESH	10
#define DAMON_PROGNAME		"DaMon"
#define DAMON_SEP		'/'


/* prototypes */
static int _damon_perror(char const * message, int error);


/* functions */
/* public */
/* damon */
static int _damon_init(DaMon * damon, char const * config);
static void _damon_destroy(DaMon * damon);

static int _damon(char const * config)
{
	DaMon damon;

	if(_damon_init(&damon, config) != 0)
		return 1;
	if(event_loop(damon.event) != 0)
		error_print(DAMON_PROGNAME);
	_damon_destroy(&damon);
	return 1;
}


/* _damon_init */
static int _init_config(DaMon * damon, char const * filename);
static int _damon_refresh(DaMon * damon);

static int _damon_init(DaMon * damon, char const * filename)
{
	struct timeval tv;

	if(_init_config(damon, filename) != 0)
		return 1;
	if((damon->event = event_new()) == NULL)
		return error_print(DAMON_PROGNAME);
	_damon_refresh(damon);
	tv.tv_sec = damon->refresh;
	tv.tv_usec = 0;
	event_register_timeout(damon->event, &tv,
			(EventTimeoutFunc)_damon_refresh, damon);
	return 0;
}

/* _init_config */
static int _config_hosts(DaMon * damon, Config * config, char const * hosts);

static int _init_config(DaMon * damon, char const * filename)
{
	Config * config;
	char const * p;
	char * q;
	int tmp;

	if((config = config_new()) == NULL)
		return 1;
	damon->prefix = NULL;
	damon->refresh = DAMON_DEFAULT_REFRESH;
	damon->hosts = NULL;
	damon->hosts_cnt = 0;
	if(filename == NULL)
		filename = ETCDIR "/" DAMON_PROGNAME ".conf";
	if(config_load(config, filename) != 0)
	{
		error_print(DAMON_PROGNAME);
		config_delete(config);
		return 1;
	}
	if((damon->prefix = config_get(config, "", "prefix")) == NULL)
		damon->prefix = ".";
	if((p = config_get(config, "", "refresh")) != NULL)
	{
		tmp = strtol(p, &q, 10);
		damon->refresh = (*p == '\0' || *q != '\0' || tmp <= 0)
			? DAMON_DEFAULT_REFRESH : tmp;
#ifdef DEBUG
		fprintf(stderr, "refresh set to %d\n", damon->refresh);
#endif
	}
	if((p = config_get(config, "", "hosts")) != NULL)
		_config_hosts(damon, config, p);
	config_delete(config);
	return 0;
}

static int _hosts_host(DaMon * damon, Config * config, Host * host,
		char const * h, unsigned int pos);
static int _config_hosts(DaMon * damon, Config * config, char const * hosts)
{
	char const * h = hosts;
	unsigned int pos = 0;
	Host * p;

	while(h[0] != '\0')
	{
		if(h[pos] != '\0' && h[pos] != ',')
		{
			pos++;
			continue;
		}
		if(pos == 0)
		{
			h++;
			continue;
		}
		if((p = realloc(damon->hosts, sizeof(*p) * (damon->hosts_cnt
							+ 1))) == NULL)
			return _damon_perror(NULL, 1);
		damon->hosts = p;
		p = &damon->hosts[damon->hosts_cnt++];
		if(_hosts_host(damon, config, p, h, pos) != 0)
			return 1;
		h += pos;
		pos = 0;
	}
	return 0;
}

static char ** _host_comma(char const * line);
static int _hosts_host(DaMon * damon, Config * config, Host * host,
		char const * h, unsigned int pos)
{
	char const * p;

	host->damon = damon;
	host->appclient = NULL;
	host->ifaces = NULL;
	host->vols = NULL;
	if((host->hostname = malloc(pos + 1)) == NULL)
		return _damon_perror(NULL, 1);
	strncpy(host->hostname, h, pos);
	host->hostname[pos] = '\0';
#ifdef DEBUG
	fprintf(stderr, "config: Host %s\n", host->hostname);
#endif
	if((p = config_get(config, host->hostname, "interfaces")) != NULL)
		host->ifaces = _host_comma(p);
	if((p = config_get(config, host->hostname, "volumes")) != NULL)
		host->vols = _host_comma(p);
	return 0;
}

static char ** _host_comma(char const * line)
{
	char const * l = line;
	unsigned int pos = 0;
	char ** values = NULL;
	char ** p;
	unsigned int cnt = 0;

	while(l[0] != '\0')
	{
		if(l[pos] != '\0' && l[pos] != ',')
		{
			pos++;
			continue;
		}
		if(pos == 0)
		{
			l++;
			continue;
		}
		if((p = realloc(values, sizeof(char*) * (cnt + 2))) == NULL)
			break;
		values = p;
		if((values[cnt] = malloc(pos + 1)) != NULL)
		{
			strncpy(values[cnt], l, pos);
			values[cnt][pos] = '\0';
#ifdef DEBUG
			fprintf(stderr, "config: %s\n", values[cnt]);
#endif
		}
		values[++cnt] = NULL;
		if(values[cnt - 1] == NULL)
			break;
		l += pos;
		pos = 0;
	}
	if(l[0] == '\0')
		return values;
	if(values == NULL)
		return NULL;
	for(p = values; *p != NULL; p++)
		free(*p);
	free(values);
	return NULL;
}

static void _damon_destroy(DaMon * damon)
{
	unsigned int i;

	for(i = 0; i < damon->hosts_cnt; i++)
	{
		free(damon->hosts[i].hostname);
		if(damon->hosts[i].appclient != NULL)
			appclient_delete(damon->hosts[i].appclient);
	}
	event_delete(damon->event);
	free(damon->hosts);
}

static AppClient * _refresh_connect(Host * host, Event * event);
static int _refresh_uptime(AppClient * ac, Host * host, char * rrd);
static int _refresh_load(AppClient * ac, Host * host, char * rrd);
static int _refresh_ram(AppClient * ac, Host * host, char * rrd);
static int _refresh_swap(AppClient * ac, Host * host, char * rrd);
static int _refresh_procs(AppClient * ac, Host * host, char * rrd);
static int _refresh_users(AppClient * ac, Host * host, char * rrd);
static int _refresh_ifaces(AppClient * ac, Host * host, char * rrd);
static int _refresh_vols(AppClient * ac, Host * host, char * rrd);
static int _damon_refresh(DaMon * damon)
{
	unsigned int i;
	AppClient * ac = NULL;
	char * rrd = NULL;
	char * p;
	Host * hosts = damon->hosts;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	for(i = 0; i < damon->hosts_cnt; i++)
	{
		if((ac = hosts[i].appclient) == NULL)
			if((ac = _refresh_connect(&hosts[i], damon->event))
					== NULL)
				continue;
		if((p = realloc(rrd, string_length(hosts[i].hostname) + 12))
				== NULL) /* XXX avoid this constant */
			break;
		rrd = p;
		if(_refresh_uptime(ac, &hosts[i], rrd) != 0
				|| _refresh_load(ac, &hosts[i], rrd) != 0
				|| _refresh_ram(ac, &hosts[i], rrd) != 0
				|| _refresh_swap(ac, &hosts[i], rrd) != 0
				|| _refresh_procs(ac, &hosts[i], rrd) != 0
				|| _refresh_users(ac, &hosts[i], rrd) != 0
				|| _refresh_ifaces(ac, &hosts[i], rrd) != 0
				|| _refresh_vols(ac, &hosts[i], rrd) != 0)
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
		error_print(DAMON_PROGNAME);
	return host->appclient;
}

static int _rrd_update(DaMon * damon, char const * filename, int args_cnt, ...);
static int _refresh_uptime(AppClient * ac, Host * host, char * rrd)
{
	int32_t ret;

	if(appclient_call(ac, &ret, "uptime") != 0)
		return error_print(DAMON_PROGNAME);
	sprintf(rrd, "%s%c%s", host->hostname, DAMON_SEP, "uptime.rrd");
	_rrd_update(host->damon, rrd, 1, ret);
	return 0;
}

static int _refresh_load(AppClient * ac, Host * host, char * rrd)
{
	int32_t res;
	uint32_t load[3];

	if(appclient_call(ac, &res, "load", &load[0], &load[1], &load[2]) != 0)
		return error_print(DAMON_PROGNAME);
	if(res != 0)
		return 0;
	sprintf(rrd, "%s%c%s", host->hostname, DAMON_SEP, "load.rrd");
	_rrd_update(host->damon, rrd, 3, load[0], load[1], load[2]);
	return 0;
}

static int _refresh_procs(AppClient * ac, Host * host, char * rrd)
{
	int32_t res;

	if(appclient_call(ac, &res, "procs") != 0)
		return 1;
	sprintf(rrd, "%s%c%s", host->hostname, DAMON_SEP, "procs.rrd");
	_rrd_update(host->damon, rrd, 1, res);
	return 0;
}

static int _refresh_ram(AppClient * ac, Host * host, char * rrd)
{
	int32_t res;
	uint32_t ram[4];

	if(appclient_call(ac, &res, "ram", &ram[0], &ram[1], &ram[2], &ram[3])
			!= 0)
		return 1;
	sprintf(rrd, "%s%c%s", host->hostname, DAMON_SEP, "ram.rrd");
	_rrd_update(host->damon, rrd, 4, ram[0], ram[1], ram[2], ram[3]);
	return 0;
}

static int _refresh_swap(AppClient * ac, Host * host, char * rrd)
{
	int32_t res;
	uint32_t swap[2];

	if(appclient_call(ac, &res, "swap", &swap[0], &swap[1]) != 0)
		return 1;
	sprintf(rrd, "%s%c%s", host->hostname, DAMON_SEP, "swap.rrd");
	_rrd_update(host->damon, rrd, 2, swap[0], swap[1]);
	return 0;
}

static int _refresh_users(AppClient * ac, Host * host, char * rrd)
{
	int32_t res;

	if(appclient_call(ac, &res, "users") != 0)
		return 1;
	sprintf(rrd, "%s%c%s", host->hostname, DAMON_SEP, "users.rrd");
	_rrd_update(host->damon, rrd, 1, res);
	return 0;
}

static int _ifaces_if(AppClient * ac, Host * host, char * rrd,
		char const * iface);
static int _refresh_ifaces(AppClient * ac, Host * host, char * rrd)
{
	char ** p = host->ifaces;
	int ret = 0;

	if(p == NULL)
		return 0;
	for(; *p != NULL; p++)
		ret |= _ifaces_if(ac, host, rrd, *p);
	return ret;
}

static int _ifaces_if(AppClient * ac, Host * host, char * rrd,
		char const * iface)
{
	int32_t res[2];

	if(appclient_call(ac, &res[0], "ifrxbytes", iface) != 0
			|| appclient_call(ac, &res[1], "iftxbytes", iface) != 0)
		return 1;
	sprintf(rrd, "%s%c%s%s", host->hostname, DAMON_SEP, iface, ".rrd");
	_rrd_update(host->damon, rrd, 2, res[0], res[1]);
	return 0;
}

static int _vols_vol(AppClient * ac, Host * host, char * rrd, char * vol);
static int _refresh_vols(AppClient * ac, Host * host, char * rrd)
{
	char ** p = host->vols;
	int ret = 0;

	if(p == NULL)
		return 0;
	for(; *p != NULL; p++)
		ret |= _vols_vol(ac, host, rrd, *p);
	return ret;
}

static int _vols_vol(AppClient * ac, Host * host, char * rrd, char * vol)
{
	int32_t res[2];

	if(appclient_call(ac, &res[0], "voltotal", vol) != 0
			|| appclient_call(ac, &res[1], "volfree", vol)
			!= 0)
		return 1;
	sprintf(rrd, "%s%s%s", host->hostname, vol, ".rrd"); /* FIXME */
	_rrd_update(host->damon, rrd, 2, res[0], res[1]);
	return 0;
}

static int _exec(char * argv[]);
static int _rrd_update(DaMon * damon, char const * filename, int args_cnt, ...)
{
	char * argv[] = { "rrdtool", "update", NULL, NULL, NULL };
	struct timeval tv;
	int pos;
	int i;
	va_list args;
	int ret;

	if(gettimeofday(&tv, NULL) != 0)
		return _damon_perror("gettimeofday", 1);
	argv[2] = string_new_append(damon->prefix, "/", filename, NULL);
	if((argv[3] = malloc((args_cnt + 1) * 12)) == NULL)
		return _damon_perror(NULL, 1);
	pos = sprintf(argv[3], "%ld", tv.tv_sec);
	va_start(args, args_cnt);
	for(i = 0; i < args_cnt; i++)
		pos += sprintf(&argv[3][pos], ":%u", va_arg(args, unsigned));
	va_end(args);
	ret = _exec(argv);
	free(argv[3]);
	string_delete(argv[2]);
	return ret;
}

static int _exec(char * argv[])
{
	pid_t pid;
	int status;
	int ret;

	if((pid = fork()) == -1)
		return _damon_perror("fork", 1);
	if(pid == 0)
	{
		execvp(argv[0], argv);
		_damon_perror(argv[0], 1);
		exit(2);
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() ", __func__);
	while(*argv != NULL)
		fprintf(stderr, "%s ", *argv++);
	fprintf(stderr, "\n");
#endif
	while((ret = waitpid(pid, &status, 0)) != -1)
		if(WIFEXITED(status))
			break;
	if(ret == -1)
		return _damon_perror("waitpid", -1);
	return WEXITSTATUS(status);
}


/* private */
static int _damon_perror(char const * message, int ret)
{
	return error_set_print(DAMON_PROGNAME, ret, "%s%s%s\n",
			message ? message : "",
			message ? ": " : "", strerror(errno));
}


/* usage */
static int _usage(void)
{
	fputs("Usage: " DAMON_PROGNAME " [-f filename]\n"
"  -f\tConfiguration file to load\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	char const * config = NULL;

	while((o = getopt(argc, argv, "f:")) != -1)
		switch(o)
		{
			case 'f':
				config = optarg;
				break;
			default:
				return _usage();
		}
	if(optind != argc)
		return _usage();
	return (_damon(config) == 0) ? 0 : 2;
}
