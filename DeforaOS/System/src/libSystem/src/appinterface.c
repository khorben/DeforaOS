/* appinterface.c */



#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#ifdef DEBUG
# include <stdio.h>
#endif
#include <dlfcn.h>

#include "string.h"
#include "appinterface.h"


/* AppInterface */
/* private */
/* types */
typedef enum _AppInterfaceCallType {
	AICT_VOID, AICT_BOOL,
	AICT_INT8, AICT_UINT8,
	AICT_INT16, AICT_UINT16,
	AICT_INT32, AICT_UINT32,
	AICT_INT64, AICT_UINT64,
	AICT_STRING, AICT_BUFFER
} AppInterfaceCallType;

typedef struct _AppInterfaceCall
{
	char * name;
	AppInterfaceCallType type;
	AppInterfaceCallType * args;
	int args_cnt;
	void * func;
} AppInterfaceCall;

struct _AppInterface
{
	AppInterfaceCall * calls;
	int calls_cnt;
	int port;
};


/* public */
/* functions */
static int _new_append(AppInterface * appinterface, AppInterfaceCallType type,
		char const * function, int args_cnt, ...);
static int _new_session(AppInterface * appinterface);
static int _new_probe(AppInterface * appinterface);
static int _new_hello(AppInterface * appinterface);
AppInterface * appinterface_new(char const * app)
{
	AppInterface * appinterface;
	struct iface {
		char * name;
		int (*func)(AppInterface *);
		int port;
	} ifaces[] = {
		{ "Session", _new_session, 4242 },
		{ "Probe", _new_probe, 4243 },
		{ "Hello", _new_hello, 4244 }
	};
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "%s%s%s", "appinterface_new(", app, ");\n");
#endif
	if((appinterface = malloc(sizeof(AppInterface))) == NULL)
		return NULL;
	appinterface->calls = NULL;
	appinterface->calls_cnt = 0;
	for(i = 0; i < sizeof(ifaces) / sizeof(struct iface); i++)
	{
		if(string_compare(app, ifaces[i].name) != 0)
			continue;
#ifdef DEBUG
		fprintf(stderr, "%s%s%s", "AppInterface \"", app, "\"\n");
#endif
		if(ifaces[i].func(appinterface) != 0)
			i = sizeof(ifaces) / sizeof(struct iface);
		break;
	}
	if(i == sizeof(ifaces) / sizeof(struct iface))
	{
#ifdef DEBUG
		fprintf(stderr, "%s", "AppInterface creation error\n");
#endif
		free(appinterface);
		return NULL;
	}
	appinterface->port = ifaces[i].port;
#ifdef DEBUG
	fprintf(stderr, "%p%s", appinterface, "\n");
#endif
	return appinterface;
}

static int _new_append(AppInterface * ai, AppInterfaceCallType type,
		char const * function, int args_cnt, ...)
{
	AppInterfaceCall * p;
	va_list args;
	int i;
	int j;

#ifdef DEBUG
	fprintf(stderr, "%s%s%s%d%s", "AppInterface supports ", function, "(",
			args_cnt, ")\n");
#endif
	for(i = 0; i < ai->calls_cnt; i++)
		if(string_compare(ai->calls[i].name, function) == 0)
			return 1;
	if((p = realloc(ai->calls, sizeof(AppInterfaceCall) * (i + 1)))
			== NULL)
	{
		/* FIXME */
		return 1;
	}
	ai->calls = p;
	ai->calls_cnt++;
	ai->calls[i].type = type;
	ai->calls[i].name = string_new(function);
	ai->calls[i].args = malloc(sizeof(AppInterfaceCallType) * args_cnt);
	ai->calls[i].args_cnt = args_cnt;
	va_start(args, args_cnt);
	for(j = 0; j < args_cnt; j++)
		ai->calls[i].args[j] = va_arg(args, AppInterfaceCallType);
	va_end(args);
	return 0;
}

static int _new_session(AppInterface * appinterface)
{
	int ret = 0;

	ret += _new_append(appinterface, AICT_UINT16, "port", 1, AICT_STRING);
	ret += _new_append(appinterface, AICT_VOID, "list", 0);
	ret += _new_append(appinterface, AICT_BOOL, "start", 1, AICT_STRING);
	ret += _new_append(appinterface, AICT_BOOL, "stop", 1, AICT_STRING);
	return ret;
}

static int _new_probe(AppInterface * appinterface)
{
	int ret = 0;

	ret += _new_append(appinterface, AICT_UINT32, "uptime", 0);
	ret += _new_append(appinterface, AICT_UINT32, "load1", 0);
	ret += _new_append(appinterface, AICT_UINT32, "load5", 0);
	ret += _new_append(appinterface, AICT_UINT32, "load15", 0);
	ret += _new_append(appinterface, AICT_UINT32, "totalram", 0);
	ret += _new_append(appinterface, AICT_UINT32, "freeram", 0);
	ret += _new_append(appinterface, AICT_UINT32, "sharedram", 0);
	ret += _new_append(appinterface, AICT_UINT32, "bufferram", 0);
	ret += _new_append(appinterface, AICT_UINT32, "totalswap", 0);
	ret += _new_append(appinterface, AICT_UINT32, "freeswap", 0);
	ret += _new_append(appinterface, AICT_UINT32, "procs", 0);
	return ret;
}

static int _new_hello(AppInterface * appinterface)
{
	return _new_append(appinterface, AICT_VOID, "hello", 0);
}


/* appinterface_new_server */
/* FIXME */
extern void * handle;
AppInterface * appinterface_new_server(char const * app)
{
	AppInterface * appinterface;
	void * handle;
	int i;
#ifdef DEBUG
	char * error;
#endif

	if((handle = dlopen(NULL, RTLD_LAZY)) == NULL)
		return NULL;
	if((appinterface = appinterface_new(app)) == NULL)
		return NULL;
	for(i = 0; i < appinterface->calls_cnt; i++)
	{
#ifdef DEBUG
		dlerror();
#endif
		appinterface->calls[i].func = dlsym(handle,
				appinterface->calls[i].name);
#ifdef DEBUG
		if((error = dlerror()) != NULL)
			fprintf(stderr, "%s%s\n", "AppServer: ", error);
#endif
	}
	dlclose(handle);
	return appinterface;
}


/* appinterface_delete */
void appinterface_delete(AppInterface * appinterface)
{
	int i;

	for(i = 0; i < appinterface->calls_cnt; i++)
	{
		free(appinterface->calls[i].name);
		free(appinterface->calls[i].args);
	}
	free(appinterface->calls);
	free(appinterface);
}


/* returns */
int appinterface_port(AppInterface * appinterface)
{
	return appinterface->port;
}


/* useful */
/* appinterface_call */
static int _send_buffer(char * data, int datalen, char * buf, int buflen,
		int * pos);
static int _send_string(char * string, char * buf, int buflen, int * pos);
int appinterface_call(AppInterface * appinterface, char * call, char buf[],
		int buflen, void ** args)
{
	AppInterfaceCall * aic;
	int pos = 0;
	int i;
	int size;

#ifdef DEBUG
	fprintf(stderr, "%s%s%s", "appinterface_call(", call, ");\n");
#endif
	for(i = 0; i < appinterface->calls_cnt; i++)
		if(string_compare(appinterface->calls[i].name, call) == 0)
			break;
	if(i == appinterface->calls_cnt)
		return -1;
	aic = &appinterface->calls[i];
	if(_send_string(call, buf, buflen, &pos) != 0)
		return -1;
	for(i = 0; i < aic->args_cnt; i++)
	{
		switch(aic->args[i])
		{
			case AICT_BOOL:
			case AICT_INT8:
			case AICT_UINT8:
				size = sizeof(int8_t);
				break;
			case AICT_INT16:
			case AICT_UINT16:
				size = sizeof(int16_t);
				break;
			case AICT_INT32:
			case AICT_UINT32:
				size = sizeof(int32_t);
				break;
			case AICT_INT64:
			case AICT_UINT64:
				size = sizeof(int64_t);
				break;
			case AICT_STRING:
				_send_string(args[i], buf, buflen, &pos);
				continue;
			default:
				/* FIXME */
#ifdef DEBUG
				fprintf(stderr, "%s, %d: %s", __FILE__,
						__LINE__,
						"Should not happen\n");
#endif
				return -1;
		}
		if(_send_buffer(args[i], size, buf, buflen, &pos) != 0)
			return -1;
	}
	return pos;
}

static int _send_buffer(char * data, int datalen, char * buf, int buflen,
		int * pos)
{
	if(*pos + datalen > buflen)
		return 1;
	memcpy(&buf[*pos], data, datalen);
	*pos += datalen;
	return 0;
}

static int _send_string(char * string, char buf[], int buflen, int * pos)
{
	int i = 0;

#ifdef DEBUG
	fprintf(stderr, "%s%s%s", "send_string(", string, ");\n");
#endif
	while(*pos < buflen)
	{
		buf[*pos] = string[i];
		(*pos)++;
		if(string[i++] == '\0')
			return 0;
	}
	return 1;
}


/* appinterface_receive */
static char * _read_string(char buf[], int buflen, int * pos);
static int _receive_args(AppInterfaceCall * calls, char buf[], int buflen,
		int * pos);
int appinterface_receive(AppInterface * appinterface, char buf[], int buflen,
		int * ret)
{
	int pos = 0;
	char * func;
	int i;

#ifdef DEBUG
	fprintf(stderr, "%s", "appinterface_receive()\n");
#endif
	if((func = _read_string(buf, buflen, &pos)) == NULL)
		return -1;
#ifdef DEBUG
	fprintf(stderr, "%s%s%s", "appinterface_receive(): ", func, "\n");
#endif
	for(i = 0; i < appinterface->calls_cnt; i++)
		if(string_compare(appinterface->calls[i].name, func) == 0)
			break;
	string_delete(func);
	if(i == appinterface->calls_cnt)
		return -1;
	/* FIXME give a way to catch errors if any */
	*ret = _receive_args(&appinterface->calls[i], buf, buflen, &pos);
	return pos;
}

static char * _read_string(char buf[], int buflen, int * pos)
{
	char * str = &buf[*pos];

	for(; *pos < buflen && buf[*pos] != '\0'; (*pos)++);
	if(*pos == buflen)
		return NULL;
	(*pos)++;
	return string_new(str);
}

static int _read_buffer(char ** data, int datalen, char buf[], int buflen,
		int * pos);
static int _receive_exec(AppInterfaceCall * calls, char ** args);
static int _receive_args(AppInterfaceCall * calls, char buf[], int buflen,
		int * pos)
{
	int i;
	char ** args;
	size_t size;
	int j;
	int ret;

	if((args = malloc(sizeof(char*) * calls->args_cnt)) == NULL)
		return 1;
	for(i = 0; i < calls->args_cnt; i++)
	{
#ifdef DEBUG
		fprintf(stderr, "%s%d%s", "_receive_args() reading arg ", i+1,
				"\n");
#endif
		switch(calls->args[i])
		{
			case AICT_VOID:
				continue;
			case AICT_BOOL:
			case AICT_INT8:
			case AICT_UINT8:
				size = sizeof(int8_t);
				break;
			case AICT_INT16:
			case AICT_UINT16:
				size = sizeof(int16_t);
				break;
			case AICT_INT32:
			case AICT_UINT32:
				size = sizeof(int32_t);
				break;
			case AICT_INT64:
			case AICT_UINT64:
				size = sizeof(int64_t);
				break;
			case AICT_STRING:
				args[i] = _read_string(buf, buflen, pos);
				continue;
		}
		if(sizeof(char*) < size)
		{
			if((args[i] = malloc(size)) == NULL)
				break;
			if(_read_buffer(args[i], size, buf, buflen, pos) != 0)
				break;
		}
		else if(_read_buffer(&args[i], size, buf, buflen, pos) != 0)
			break;
	}
	ret = _receive_exec(calls, args);
	/* FIXME free everything allocated */
	for(j = 0; j < i; j++)
	{
#ifdef DEBUG
		fprintf(stderr, "%s%d%s", "_receive_args() freeing arg ", j+1,
				"\n");
#endif
		switch(calls->args[j])
		{
			case AICT_VOID:
				continue;
			case AICT_BOOL:
			case AICT_INT8:
			case AICT_UINT8:
				size = sizeof(int8_t);
				break;
			case AICT_INT16:
			case AICT_UINT16:
				size = sizeof(int16_t);
				break;
			case AICT_INT32:
			case AICT_UINT32:
				size = sizeof(int32_t);
				break;
			case AICT_INT64:
			case AICT_UINT64:
				size = sizeof(int64_t);
				break;
			case AICT_STRING:
				free(args[j]);
				continue;
		}
		if(sizeof(char*) < size)
			free(args[i]);
	}
	free(args);
	return ret;
}

static int _read_buffer(char ** data, int datalen, char buf[], int buflen,
		int * pos)
{
	if(datalen > buflen - *pos)
		return 1;
	memcpy(data, buf, datalen);
	(*pos)+=datalen;
	return 0;
}

static int _receive_exec(AppInterfaceCall * calls, char ** args)
{
	int (*func0)(void);
	int (*func1)(char *);
	int (*func2)(char *, char *);

#ifdef DEBUG
	fprintf(stderr, "%s", "_receive_exec()\n");
#endif
	/* FIXME */
	switch(calls->args_cnt)
	{
		case 0:
			func0 = calls->func;
			return func0();
		case 1:
			func1 = calls->func;
			return func1(args[0]);
		case 2:
			func2 = calls->func;
			return func2(args[0], args[1]);
		default:
#ifdef DEBUG
			fprintf(stderr, "%s%d%s",
					"AppInterface: functions with ",
					calls->args_cnt,
					" arguments are not supported\n");
#endif
			return -1;
	}
	return -1;
}
