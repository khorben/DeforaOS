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
};


/* public */
/* functions */
static int _new_append(AppInterface * appinterface, AppInterfaceCallType type,
		char const * function, int args_cnt, ...);
static int _new_session(AppInterface * appinterface);
static int _new_hello(AppInterface * appinterface);
AppInterface * appinterface_new(char const * app)
{
	AppInterface * appinterface;
	struct iface { char * name; int (* func)(AppInterface *); } ifaces[] = {
		{ "Session", _new_session },
		{ "Hello", _new_hello }
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
	fprintf(stderr, "%s%s%s%d%s", "interface supports ", function, "(",
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

static int _new_hello(AppInterface * appinterface)
{
	return _new_append(appinterface, AICT_VOID, "hello", 0);
}


/* appinterface_new_server */
AppInterface * appinterface_new_server(char const * app)
{
	AppInterface * appinterface;
	void * handle;
	int i;

	if((handle = dlopen(NULL, RTLD_LAZY)) == NULL)
		return NULL;
	if((appinterface = appinterface_new(app)) == NULL)
		return NULL;
	for(i = 0; i < appinterface->calls_cnt; i++)
	{
		appinterface->calls[i].func = dlsym(handle,
				appinterface->calls[i].name);
#ifdef DEBUG
		if(appinterface->calls[i].func == NULL)
			fprintf(stderr, "%s%s%s", "AppServer lacks function \"",
					appinterface->calls[i].name, "\"\n");
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

static int _send_string(char * string, char * buf, int buflen, int * pos)
{
	int i = 0;

#ifdef DEBUG
	fprintf(stderr, "%s%s%s", "send_string(", string, ");\n");
#endif
	while(*pos < buflen)
	{
		buf[*pos] = string[i++];
		(*pos)++;
		if(string[i] == '\0')
			return 0;
	}
	return 1;
}
