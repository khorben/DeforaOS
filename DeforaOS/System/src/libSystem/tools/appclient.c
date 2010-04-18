/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System libSystem */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <System.h>

#define APPCLIENT_PROGNAME "AppClient"


/* AppClient */
/* private */
/* types */
typedef enum _AppClientCallArgType
{
	ACCAT_INTEGER = 0, ACCAT_STRING
} AppClientCallArgType;

typedef struct _AppClientCallArg
{
	AppClientCallArgType type;
	int integer;
	char const * string;
} AppClientCallArg;

typedef struct _AppClientCall
{
	char const * name;
	AppClientCallArg * args;
	size_t args_cnt;
} AppClientCall;


/* prototypes */
static int _error(char const * message, int ret);


/* functions */
static int _appclient_hostname(int verbose, char const * hostname,
		char const * service);
static int _appclient_call(int verbose, AppClient * ac, AppClientCall * call);

static int _appclient(int verbose, char const * hostname, char const * service,
		AppClientCall calls[], size_t calls_cnt)
{
	int ret = 0;
	AppClient * ac;
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d, %s, %s, %p, %zu)\n", __func__, verbose,
			hostname, service, (void*)calls, calls_cnt);
#endif
	if(_appclient_hostname(verbose, hostname, service) != 0
			|| (ac = appclient_new(service)) == NULL)
		return _error(APPCLIENT_PROGNAME, 1);
	for(i = 0; i < calls_cnt; i++)
		if(_appclient_call(verbose, ac, &calls[i]) != 0)
			ret |= _error(APPCLIENT_PROGNAME, 1);
	if(verbose != 0)
		printf("%s", "Disconnecting\n");
	appclient_delete(ac);
	return ret;
}

static int _appclient_hostname(int verbose, char const * hostname,
		char const * service)
{
	char const prefix[] = "APPSERVER_";
	char * buf;
	size_t len;
	char const * env;
	int res;

	len = sizeof(prefix) + strlen(service) + 1;
	if((buf = malloc(len)) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	snprintf(buf, len, "%s%s", prefix, service);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__, buf);
#endif
	if((env = getenv(buf)) == NULL && errno != ENOENT)
		return error_set_code(1, "%s", strerror(errno));
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__, env);
#endif
	if(hostname == NULL)
	{
		if(verbose != 0)
			printf("%s%s%s%s\n", "Connecting to service ", service,
					" on ", (env != NULL) ? env
					: "localhost");
		free(buf);
		return 0;
	}
	res = setenv(buf, hostname, 1);
	free(buf);
	if(res != 0)
		return error_set_code(1, "%s", strerror(errno));
	if(verbose != 0)
		printf("%s%s%s%s\n", "Connecting to service ", service, " on ",
				hostname);
	return 0;
}

static int _appclient_call(int verbose, AppClient * ac, AppClientCall * call)
{
	int res = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	/* FIXME may segfault (check interface), use appclient_callv? */
	switch(call->args_cnt)
	{
		case 0:
#ifdef DEBUG
			fprintf(stderr, "DEBUG: %s() %s()\n", __func__,
					call->name);
#endif
			if(appclient_call(ac, &res, call->name, NULL) != 0)
				return 1;
			break;
		case 1:
#ifdef DEBUG
			fprintf(stderr, "DEBUG: %s() %s(\"%s\")\n", __func__,
					call->name, call->args[0].string);
#endif
			/* FIXME arguments may be of different types */
			if(appclient_call(ac, &res, call->name,
						call->args[0].string) != 0)
				return 1;
			break;
		default:
			return error_set_code(1, "%s",
					"Unsupported number of arguments");
	}
	if(verbose)
		printf("\"%s\"%s%d\n", call->name, " returned ", res);
	return 0;
}


/* error */
static int _error(char const * message, int ret)
{
	error_print(message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: " APPCLIENT_PROGNAME " [-v][-H hostname] -S service"
" [-C call [-s string|-i integer]...]...\n"
"  -v	Be more verbose\n"
"  -H	Hostname to connect to\n"
"  -S	Service to connect to\n"
"  -C	Enqueue a given call\n"
"  -s	Add a string as an argument to the current call\n"
"  -i	Add an integer as an argument to the current call\n", stderr);
	return 1;
}


/* main */
static int _main_call(AppClientCall ** calls, size_t * calls_cnt,
		char const * name);
static int _main_call_arg_string(AppClientCall * calls, size_t calls_cnt,
		char const * string);

int main(int argc, char * argv[])
{
	int o;
	int verbose = 0;
	char const * hostname = NULL;
	char const * service = NULL;
	AppClientCall * calls = NULL;
	size_t calls_cnt = 0;

	while((o = getopt(argc, argv, "vH:S:C:s:")) != -1)
		switch(o)
		{
			case 'v':
				verbose = 1;
				break;
			case 'H':
				hostname = optarg;
				break;
			case 'S':
				service = optarg;
				break;
			case 'C':
				if(_main_call(&calls, &calls_cnt, optarg) != 0)
					return _error(APPCLIENT_PROGNAME, 2);
				break;
			case 's':
				if(_main_call_arg_string(calls, calls_cnt,
							optarg) != 0)
					return _error(APPCLIENT_PROGNAME, 2);
				break;
				/* FIXME implement case 'i' */
			default:
				return _usage();
		}
	if(service == NULL)
		return _usage();
	return (_appclient(verbose, hostname, service, calls, calls_cnt) == 0)
		? 0 : 2;
}

static int _main_call(AppClientCall ** calls, size_t * calls_cnt,
		char const * name)
{
	AppClientCall * p;

	if((p = realloc(*calls, sizeof(*p) * ((*calls_cnt) + 1))) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	*calls = p;
	p = &(*calls)[(*calls_cnt)++];
	memset(p, 0, sizeof(*p));
	p->name = name;
	p->args = NULL;
	p->args_cnt = 0;
	return 0;
}

static int _main_call_arg_string(AppClientCall * calls, size_t calls_cnt,
		char const * string)
{
	AppClientCall * p;
	AppClientCallArg * q;

	if(calls_cnt == 0)
		return 1; /* FIXME report error */
	p = &calls[calls_cnt - 1];
	if((q = realloc(p->args, sizeof(*q) * (p->args_cnt + 1))) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	p->args = q;
	q = &q[p->args_cnt++];
	memset(q, 0, sizeof(*q));
	q->type = ACCAT_STRING;
	q->string = string;
	return 0;
}
