/* appinterface.c */
/* TODO:
 * - isn't there a problem if data is gone through faster than the end of the
 *   call transmission? */



#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#ifdef DEBUG
# include <stdio.h>
#endif
#include <string.h>
#include <dlfcn.h>

#include "System.h"
#include "appinterface.h"


/* AppInterface */
/* private */
/* types */
typedef enum _AppInterfaceCallType {
	AICT_VOID = 000,	AICT_BOOL = 001,
	AICT_INT8 = 002,	AICT_UINT8 = 003,
	AICT_INT16 = 004, 	AICT_UINT16 = 005,
	AICT_INT32 = 006, 	AICT_UINT32 = 007,
	AICT_INT64 = 010, 	AICT_UINT64 = 011,
	AICT_STRING = 012, 	AICT_BUFFER = 013
} AppInterfaceCallType;
#define AICT_LAST AICT_BUFFER
int _aict_size[AICT_LAST+1] = {
	0,		sizeof(char),
	sizeof(int8_t),	sizeof(uint8_t),
	sizeof(int16_t),sizeof(uint16_t),
	sizeof(int32_t),sizeof(uint32_t),
	sizeof(int64_t),sizeof(uint64_t),
	0,		0
};

typedef enum _AppInterfaceCallDirection {
	AICD_IN = 0000,
	AICD_OUT = 0100,
	AICD_IN_OUT = 0200
} AppInterfaceCallDirection;

typedef struct _AppInterfaceCallArg
{
	AppInterfaceCallType type;
	AppInterfaceCallDirection direction;
	int size;
} AppInterfaceCallArg;

typedef struct _AppInterfaceCall
{
	char * name;
	AppInterfaceCallArg type;
	AppInterfaceCallArg * args;
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
static int _new_gserver(AppInterface * appinterface);
static int _new_probe(AppInterface * appinterface);
static int _new_hello(AppInterface * appinterface);
static int _new_vfs(AppInterface * appinterface);
AppInterface * appinterface_new(char const * app)
{
	AppInterface * appinterface;
	/* FIXME read this from available Servers configuration, or imagine a
	 * solution to negociate it directly */
	struct iface {
		char * name;
		int (*func)(AppInterface *);
		int port;
	} ifaces[] = {
		{ "Session",	_new_session,	4242 },
		{ "GServer",	_new_gserver,	4246 },
		{ "Probe",	_new_probe,	4243 },
		{ "Hello",	_new_hello,	4244 },
		{ "VFS",	_new_vfs,	4245 }
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
	fprintf(stderr, "%s%p%s", "AppInterface: ", appinterface, "\n");
#endif
	return appinterface;
}

static void _append_arg(AppInterfaceCallArg * arg, AppInterfaceCallType type,
		AppInterfaceCallDirection direction);
static int _new_append(AppInterface * ai, AppInterfaceCallType type,
		char const * function, int args_cnt, ...)
{
	AppInterfaceCall * p;
	va_list args;
	int i;
	int j;
	int type_direction;
	int direction;

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
	_append_arg(&ai->calls[i].type, type, AICD_OUT);
	ai->calls[i].name = string_new(function);
	ai->calls[i].args = malloc(sizeof(AppInterfaceCallArg) * args_cnt);
	ai->calls[i].args_cnt = args_cnt;
	va_start(args, args_cnt);
	for(j = 0; j < args_cnt; j++)
	{
		type_direction = va_arg(args, AppInterfaceCallType);
		direction = type_direction & (AICD_IN | AICD_OUT | AICD_IN_OUT);
		_append_arg(&ai->calls[i].args[j], type_direction-direction,
				direction);
	}
	va_end(args);
	return 0;
}

static void _append_arg(AppInterfaceCallArg * arg, AppInterfaceCallType type,
		AppInterfaceCallDirection direction)
{
	arg->type = type;
	arg->direction = direction;
	arg->size = _aict_size[type];
#ifdef DEBUG
	fprintf(stderr, "type: %d, direction: %d, size: %d\n", type, direction,
			arg->size);
#endif
}

static int _new_session(AppInterface * ai)
{
	int ret = 0;

	ret += _new_append(ai, AICT_UINT16, "port", 1, AICT_STRING);
	ret += _new_append(ai, AICT_VOID, "list", 0);
	ret += _new_append(ai, AICT_BOOL, "start", 1, AICT_STRING);
	ret += _new_append(ai, AICT_BOOL, "stop", 1, AICT_STRING);
	return ret;
}

static int _new_gserver(AppInterface * ai)
{
	return 0;
}

static int _new_probe(AppInterface * ai)
{
	int ret = 0;

	/* FIXME need a way to list capabilities, such as network interfaces */
	ret += _new_append(ai, AICT_UINT32, "uptime", 0);
	ret += _new_append(ai, AICT_UINT32, "load_1", 0);
	ret += _new_append(ai, AICT_UINT32, "load_5", 0);
	ret += _new_append(ai, AICT_UINT32, "load_15", 0);
	ret += _new_append(ai, AICT_UINT32, "ram_total", 0);
	ret += _new_append(ai, AICT_UINT32, "ram_free", 0);
	ret += _new_append(ai, AICT_UINT32, "ram_shared", 0);
	ret += _new_append(ai, AICT_UINT32, "ram_buffer", 0);
	ret += _new_append(ai, AICT_UINT32, "swap_total", 0);
	ret += _new_append(ai, AICT_UINT32, "swap_free", 0);
	ret += _new_append(ai, AICT_UINT32, "users", 0);
	ret += _new_append(ai, AICT_UINT32, "procs", 0);
	ret += _new_append(ai, AICT_UINT32, "ifrxbytes", 1, AICT_STRING);
	ret += _new_append(ai, AICT_UINT32, "iftxbytes", 1, AICT_STRING);
	ret += _new_append(ai, AICT_UINT32, "voltotal", 1, AICT_STRING);
	ret += _new_append(ai, AICT_UINT32, "volfree", 1, AICT_STRING);
	return ret;
}

static int _new_hello(AppInterface * ai)
{
	return _new_append(ai, AICT_VOID, "hello", 0);
}

static int _new_vfs(AppInterface * ai)
	/* FIXME read, *stat */
{
	int ret = 0;

	ret+=_new_append(ai, AICT_UINT32, "chmod", 2, AICT_STRING, AICT_UINT32);
	ret+=_new_append(ai, AICT_UINT32, "chown", 3, AICT_STRING, AICT_UINT32,
			AICT_UINT32);
	ret+=_new_append(ai, AICT_UINT32, "close", 1, AICT_UINT32);
	ret+=_new_append(ai, AICT_UINT32, "creat", 2, AICT_STRING, AICT_UINT32);
	ret+=_new_append(ai, AICT_UINT32, "fchmod", 2, AICT_UINT32,
			AICT_UINT32);
	ret+=_new_append(ai, AICT_UINT32, "fchown", 3, AICT_UINT32, AICT_UINT32,
			AICT_UINT32);
	ret+=_new_append(ai, AICT_UINT32, "flock", 2, AICT_UINT32, AICT_UINT32);
/*	ret+=_new_append(ai, AICT_UINT32, "fstat", 2, AICT_UINT32,
			AICT_BUFFER | AICD_OUT); */
	ret+=_new_append(ai, AICT_UINT32, "lchown", 3, AICT_STRING, AICT_UINT32,
			AICT_UINT32);
	ret+=_new_append(ai, AICT_UINT32, "link", 2, AICT_STRING, AICT_STRING);
	ret+=_new_append(ai, AICT_UINT32, "lseek", 3, AICT_UINT32, AICT_UINT32,
			AICT_UINT32);
	ret+=_new_append(ai, AICT_UINT32, "lstat", 2, AICT_STRING,
			AICT_BUFFER | AICD_OUT);
	ret+=_new_append(ai, AICT_UINT32, "mkdir", 2, AICT_STRING, AICT_UINT32);
/*	ret+=_new_append(ai, AICT_UINT32, "mknod", 2, AICT_STRING, AICT_UINT32,
			AICT_UINT32); */
	ret+=_new_append(ai, AICT_UINT32, "open", 3, AICT_STRING, AICT_UINT32,
			AICT_UINT32);
	ret+=_new_append(ai, AICT_UINT32, "read", 3, AICT_UINT32,
			AICT_BUFFER | AICD_OUT, AICT_UINT32);
	ret+=_new_append(ai, AICT_UINT32, "rename", 2, AICT_STRING,
			AICT_STRING);
	ret+=_new_append(ai, AICT_UINT32, "rmdir", 1, AICT_STRING);
	ret+=_new_append(ai, AICT_UINT32, "stat", 2, AICT_STRING,
			AICT_BUFFER | AICD_OUT);
	ret+=_new_append(ai, AICT_UINT32, "symlink", 2, AICT_STRING,
			AICT_STRING);
	ret+=_new_append(ai, AICT_UINT32, "umask", 1, AICT_UINT32);
	ret+=_new_append(ai, AICT_UINT32, "unlink", 1, AICT_STRING);
	ret+=_new_append(ai, AICT_UINT32, "write", 3, AICT_UINT32, AICT_BUFFER,
			AICT_UINT32);
	return ret;
}


/* appinterface_new_server */
/* FIXME */
AppInterface * appinterface_new_server(char const * app)
{
	AppInterface * ai;
	void * handle;
	int i;
#ifdef DEBUG
	char * error;
#endif

	if((handle = dlopen(NULL, RTLD_LAZY)) == NULL)
		return NULL;
	if((ai = appinterface_new(app)) == NULL)
		return NULL;
	for(i = 0; i < ai->calls_cnt; i++)
	{
#ifdef DEBUG
		dlerror();
#endif
		ai->calls[i].func = dlsym(handle, ai->calls[i].name);
#ifdef DEBUG
		if((error = dlerror()) != NULL)
			fprintf(stderr, "%s%s\n", "AppServer: ", error);
#endif
	}
	dlclose(handle);
	return ai;
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
static AppInterfaceCall * _call_call(AppInterface * appinterface, char * call);
static int _send_buffer(char * data, size_t datalen, char * buf, size_t buflen,
		size_t * pos);
static int _send_string(char * string, char * buf, size_t buflen, size_t * pos);
int appinterface_call(AppInterface * appinterface, char * call, char buf[],
		size_t buflen, void ** args)
{
	AppInterfaceCall * aic;
	size_t pos = 0;
	int i;
	size_t size;
	Buffer * buffer;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "%s%s%s", "appinterface_call(", call, ");\n");
#endif
	if((aic = _call_call(appinterface, call)) == NULL)
		return -1;
	if(_send_string(call, buf, buflen, &pos) != 0)
		return -1;
	for(i = 0; i < aic->args_cnt; i++)
	{
		if(aic->args[i].direction == AICD_OUT)
		{
			if(aic->args[i].type != AICT_BUFFER)
				continue;
			buffer = args[i];
			size = buffer_length(buffer);
			aic->args[i].size = size;
			if(_send_buffer((char*)&size, sizeof(uint32_t), buf, 
						buflen, &pos) != 0)
				return -1;
			continue;
		}
		size = aic->args[i].size;
		switch(aic->args[i].type)
		{
			case AICT_VOID:
				continue;
			case AICT_BUFFER:
				buffer = args[i];
				size = buffer_length(buffer);
				aic->args[i].size = size;
				if(_send_buffer((char*)&size, sizeof(uint32_t),
						buf, buflen, &pos) != 0)
					return -1;
				p = buffer_data(buffer);
				break;
			case AICT_STRING:
				_send_string(args[i], buf, buflen, &pos);
				continue;
			default:
				if(sizeof(char*) < size)
					p = args[i];
				else
					p = (char*)&args[i];
				break;
		}
#ifdef DEBUG
		fprintf(stderr, "appinterface_call() sending %d\n", size);
#endif
		if(_send_buffer(p, size, buf, buflen, &pos) != 0)
			return -1;
	}
	return pos;
}

static AppInterfaceCall * _call_call(AppInterface * appinterface, char * call)
{
	int i;

	for(i = 0; i < appinterface->calls_cnt; i++)
		if(string_compare(appinterface->calls[i].name, call) == 0)
			break;
	if(i == appinterface->calls_cnt)
		return NULL;
	return &appinterface->calls[i];
}

static int _send_buffer(char * data, size_t datalen, char * buf, size_t buflen,
		size_t * pos)
{
	if(*pos + datalen > buflen)
		return 1;
	memcpy(&buf[*pos], data, datalen);
	*pos += datalen;
	return 0;
}

static int _send_string(char * string, char buf[], size_t buflen, size_t * pos)
{
	int i = 0;

#ifdef DEBUG
	fprintf(stderr, "%s%s%s", "send_string(\"", string, "\");\n");
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


/* appinterface_call_receive */
int appinterface_call_receive(AppInterface * appinterface, int * ret,
		char * func, void ** args, char buf[], size_t buflen)
	/* FIXME
	 * - this should be in appinterface_call (with an async option)
	 * - we should avoid copying stuff while not enough data is gathered */
{
	AppInterfaceCall * aic;
	int i;
	int pos = 0;
	size_t size;

	for(i = 0; i < appinterface->calls_cnt; i++)
		if(string_compare(appinterface->calls[i].name, func) == 0)
			break;
	if(i == appinterface->calls_cnt)
		return -1;
	aic = &appinterface->calls[i];
	for(i = 0; i < aic->args_cnt; i++)
	{
		if(aic->args[i].direction == AICD_IN)
			continue;
		size = aic->args[i].size;
		switch(aic->args[i].type)
		{
			case AICT_BUFFER:
				if(buflen - sizeof(int) < size)
					return 0;
				memcpy(buffer_data(args[i]), &buf[pos], size);
				pos+=size;
				break;
			default:
#ifdef DEBUG
				fprintf(stderr, "%s", "Not yet implemented\n");
#endif
				/* FIXME */
				break;
		}
	}
	if(pos - buflen < sizeof(int))
		return 0;
	memcpy(ret, &(buf[pos]), sizeof(int));
	return pos + sizeof(int);
}


/* appinterface_receive */
static char * _read_string(char buf[], size_t buflen, size_t * pos);
static int _receive_args(AppInterfaceCall * calls, char buf[], size_t buflen,
		size_t * pos, char bufw[], size_t bufwlen, size_t * bufwpos);
int appinterface_receive(AppInterface * appinterface, char buf[], size_t buflen,
		char bufw[], size_t bufwlen, size_t * bufwpos, int * ret)
{
	size_t pos = 0;
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
	*ret = _receive_args(&appinterface->calls[i], buf, buflen, &pos,
			bufw, bufwlen, bufwpos);
#ifdef DEBUG
	fprintf(stderr, "ret = %d\n", *ret);
#endif
	return pos;
}

static char * _read_string(char buf[], size_t buflen, size_t * pos)
{
	char * str = &buf[*pos];

	for(; *pos < buflen && buf[*pos] != '\0'; (*pos)++);
	if(*pos == buflen)
		return NULL;
	(*pos)++;
#ifdef DEBUG
	fprintf(stderr, "%s%s%s", "_read_string(\"", str, "\")\n");
#endif
	return string_new(str);
}

static int _args_pre_exec(AppInterfaceCall * calls, char buf[], size_t buflen,
		size_t * pos, char ** args);
static int _receive_exec(AppInterfaceCall * calls, char ** args);
static int _args_post_exec(AppInterfaceCall * calls, char buf[], size_t buflen,
		size_t * pos, char ** args, int i);
static int _receive_args(AppInterfaceCall * calls, char buf[], size_t buflen,
		size_t * pos, char bufw[], size_t bufwlen, size_t * bufwpos)
{
	int ret = 0;
	int i;
	char ** args;

	if((args = malloc(sizeof(char*) * calls->args_cnt)) == NULL)
		return 1;
	if((i = _args_pre_exec(calls, buf, buflen, pos, args))
			== calls->args_cnt)
		/* FIXME separate error checking from function actual result */
		ret = _receive_exec(calls, args);
	ret |= _args_post_exec(calls, bufw, bufwlen, bufwpos, args, i);
	free(args);
	return ret;
}

static int _read_buffer(char ** data, size_t datalen, char buf[], size_t buflen,
		size_t * pos);
static int _args_pre_exec(AppInterfaceCall * calls, char buf[], size_t buflen,
		size_t * pos, char ** args)
{
	int i = 0;
	size_t size;

	for(i = 0; i < calls->args_cnt; i++)
	{
#ifdef DEBUG
		fprintf(stderr, "%s%d%s", "_receive_args() reading arg ", i+1,
				"\n");
#endif
		if(calls->args[i].direction == AICD_OUT)
		{
			if(calls->args[i].type != AICT_BUFFER)
				continue;
			_read_buffer((char**)&size, sizeof(int32_t), buf,
					buflen, pos);
			calls->args[i].size = size;
#ifdef DEBUG
			fprintf(stderr, "should send %d\n", size);
#endif
			args[i] = malloc(size); /* FIXME free */
			continue;
		}
		size = calls->args[i].size;
		switch(calls->args[i].type)
		{
			case AICT_VOID:
				continue;
			case AICT_BOOL:
			case AICT_INT8:
			case AICT_UINT8:
			case AICT_INT16:
			case AICT_UINT16:
			case AICT_INT32:
			case AICT_UINT32:
			case AICT_INT64:
			case AICT_UINT64:
				break;
			case AICT_BUFFER:
				_read_buffer((char**)&size, sizeof(int32_t),
						buf, buflen, pos);
				calls->args[i].size = size;
#ifdef DEBUG
				fprintf(stderr, "should send %d\n", size);
#endif
				break;
			case AICT_STRING:
				args[i] = _read_string(buf, buflen, pos);
				continue;
		}
		if(sizeof(char*) < size)
		{
			if((args[i] = malloc(size)) == NULL)
				break;
			if(_read_buffer((char**)args[i], size, buf, buflen, pos)
					!= 0)
				break;
		}
		else if(_read_buffer(&args[i], size, buf, buflen, pos) != 0)
			break;
	}
	return i;
}

static int _read_buffer(char ** data, size_t datalen, char buf[], size_t buflen,
		size_t * pos)
{
	if(datalen > buflen - *pos)
		return 1;
	memcpy(data, &buf[*pos], datalen);
	(*pos)+=datalen;
	return 0;
}

static int _receive_exec(AppInterfaceCall * calls, char ** args)
{
	int (*func0)(void);
	int (*func1)(char *);
	int (*func2)(char *, char *);
	int (*func3)(char *, char *, char *);

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
		case 3:
			func3 = calls->func;
			return func3(args[0], args[1], args[2]);
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

static int _args_post_exec(AppInterfaceCall * calls, char buf[], size_t buflen,
		size_t * pos, char ** args, int i)
{
	int j;
	size_t size;
	char * p;

	/* FIXME free everything allocated */
	for(j = 0; j < i; j++)
	{
#ifdef DEBUG
		fprintf(stderr, "%s%d%s", "_args_post_exec() freeing arg ", j+1,
				"\n");
#endif
		size = calls->args[j].size;
		if(i == calls->args_cnt && (calls->args[j].direction == AICD_OUT
					|| calls->args[j].direction
					== AICD_IN_OUT))
		{
			switch(calls->args[j].type)
			{
				case AICT_VOID:
					p = NULL;
					break;
				case AICT_BUFFER:
					size = calls->args[j].size;
					p = args[j];
					break;
				case AICT_STRING:
					size = string_length(args[j]) + 1;
					p = args[j];
					break;
				default:
					if(sizeof(char*) < size)
						p = args[j];
					else
						p = (char*)&args[j];
					break;
			}
#ifdef DEBUG
			fprintf(stderr, "_args_post_exec() sending %u\n", size);
#endif
			if(_send_buffer(p, size, buf, buflen, pos) != 0)
				break;
		}
		switch(calls->args[j].type)
		{
			case AICT_VOID:
				continue;
			case AICT_BOOL:
			case AICT_INT8:
			case AICT_UINT8:
			case AICT_INT16:
			case AICT_UINT16:
			case AICT_INT32:
			case AICT_UINT32:
			case AICT_INT64:
			case AICT_UINT64:
				break;
			case AICT_BUFFER:
			case AICT_STRING:
				free(args[j]);
				continue;
		}
		if(sizeof(char*) < size)
			free(args[j]);
	}
	return j == i ? 0 : 1;
}
