/* $Id$ */
/* Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System libApp */
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
/* TODO:
 * - implement what's missing */



#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef __WIN32__
# include <Winsock2.h>
#else
# include <arpa/inet.h>
# include <netinet/in.h>
#endif
#ifdef WITH_SSL
# include <openssl/ssl.h>
#endif
#include <errno.h>
#include <System.h>
#include "System/App/appserver.h"
#include "appinterface.h"
#include "../config.h"

#ifndef PREFIX
# define PREFIX	"/usr/local"
#endif
#ifndef ETCDIR
# define ETCDIR	PREFIX "/etc"
#endif


/* AppInterface */
/* private */
/* types */
typedef enum _AppInterfaceCallType
{
	AICT_VOID	= 000,	AICT_BOOL	= 001,
	AICT_INT8	= 002,	AICT_UINT8	= 003,
	AICT_INT16	= 004, 	AICT_UINT16	= 005,
	AICT_INT32	= 006, 	AICT_UINT32	= 007,
	AICT_INT64	= 010, 	AICT_UINT64	= 011,
	AICT_STRING	= 012, 	AICT_BUFFER	= 013,
	AICT_FLOAT	= 014,	AICT_DOUBLE	= 015
} AppInterfaceCallType;
#define AICT_LAST AICT_DOUBLE
#define AICT_COUNT (AICT_LAST + 1)
#define AICT_MASK 077

#ifdef DEBUG
static const String * AICTString[AICT_COUNT] =
{
	"void", "bool", "int8", "uint8", "int16", "uint16", "int32", "uint32",
	"int64", "uint64", "String", "Buffer", "float", "double"
};
#endif

static int _aict_size[AICT_COUNT] =
{
	0,			sizeof(char),
	sizeof(int8_t),		sizeof(uint8_t),
	sizeof(int16_t),	sizeof(uint16_t),
	sizeof(int32_t),	sizeof(uint32_t),
	sizeof(int64_t),	sizeof(uint64_t),
	0,			0,
	sizeof(float),		sizeof(double)
};

typedef enum _AppInterfaceCallDirection
{
	AICD_IN		= 0000,
	AICD_IN_OUT	= 0100,
	AICD_OUT	= 0200
} AppInterfaceCallDirection;
#define AICD_MASK 0700

typedef struct _AppInterfaceCallArg
{
	AppInterfaceCallType type;
	AppInterfaceCallDirection direction;
	size_t size;
} AppInterfaceCallArg;

typedef struct _AppInterfaceCall
{
	char * name;
	AppInterfaceCallArg type;
	AppInterfaceCallArg * args;
	size_t args_cnt;
	void * func;
} AppInterfaceCall;

struct _AppInterface
{
	String * name;
	AppInterfaceCall * calls;
	size_t calls_cnt;
	uint16_t port;
	int error;
};

typedef struct _StringEnum
{
	char const * string;
	int value;
} StringEnum;


/* variables */
StringEnum _string_type[] =
{
	{ "VOID",	AICT_VOID		},
	{ "BOOL",	AICT_BOOL		},
	{ "INT8",	AICT_INT8		},
	{ "UINT8",	AICT_UINT8		},
	{ "INT16",	AICT_INT16		},
	{ "UINT16",	AICT_UINT16		},
	{ "INT32",	AICT_INT32		},
	{ "UINT32",	AICT_UINT32		},
	{ "INT64",	AICT_INT64		},
	{ "UINT64",	AICT_UINT64		},
	{ "STRING",	AICT_STRING		},
	{ "STRING_OUT",	AICT_STRING | AICD_OUT	},
	{ "BUFFER",	AICT_BUFFER		},
	{ "BUFFER_OUT",	AICT_BUFFER | AICD_OUT	},
	{ "FLOAT",	AICT_FLOAT		},
	{ "DOUBLE",	AICT_DOUBLE		},
	{ NULL,		0			}
};


/* prototypes */
static int _string_enum(String const * string, StringEnum * se);
static AppInterfaceCall * _appinterface_get_call(AppInterface * appinterface,
		char const * call);


/* functions */
/* string_enum */
/* FIXME move to string.c */
static int _string_enum(String const * string, StringEnum * se)
{
	size_t i;

	if(string == NULL)
		return -error_set_code(1, "%s", strerror(EINVAL));
	for(i = 0; se[i].string != NULL; i++)
		if(string_compare(string, se[i].string) == 0)
			return se[i].value;
	return -error_set_code(1, "%s\"%s\"", "Unknown enumerated value for ",
			string);
}


/* appinterface_get_call */
static AppInterfaceCall * _appinterface_get_call(AppInterface * appinterface,
		String const * call)
{
	size_t i;

	for(i = 0; i < appinterface->calls_cnt; i++)
		if(string_compare(appinterface->calls[i].name, call) == 0)
			break;
	if(i == appinterface->calls_cnt)
	{
		error_set_code(1, "%s%s%s%s", "Unknown call ", call,
				" for interface ", appinterface->name);
		return NULL;
	}
	return &appinterface->calls[i];
}


/* public */
/* functions */
/* appinterface_new */
static int _new_foreach(char const * key, Hash * value,
		AppInterface * appinterface);
static int _new_append(AppInterface * ai, AppInterfaceCallType type,
		char const * function);
static int _new_append_arg(AppInterface * ai, char const * arg);

AppInterface * appinterface_new(char const * app)
{
#ifdef WITH_SSL
	static int ssl_init = 0;
#endif
	AppInterface * appinterface;
	String * pathname = NULL;
	Config * config = NULL;
	char const * p;

#ifdef WITH_SSL
	if(ssl_init == 0)
	{
		SSL_library_init();
		SSL_load_error_strings();
		ssl_init = 1;
	}
#endif
	if(app == NULL)
		return NULL; /* FIXME report error */
	if((appinterface = object_new(sizeof(*appinterface))) == NULL)
		return NULL;
	appinterface->name = string_new(app);
	appinterface->calls = NULL;
	appinterface->calls_cnt = 0;
	appinterface->port = 0;
	appinterface->error = 0;
	if(appinterface->name == NULL
			|| (pathname = string_new_append(
					ETCDIR "/AppInterface/", app,
					".interface", NULL)) == NULL
			|| (config = config_new()) == NULL
			|| config_load(config, pathname) != 0)
	{
		if(config != NULL)
			config_delete(config);
		string_delete(pathname);
		appinterface_delete(appinterface);
		return NULL;
	}
	if((p = config_get(config, NULL, "port")) != NULL)
		appinterface->port = atoi(p);
	appinterface->error = 0;
	hash_foreach(config, (HashForeach)_new_foreach, appinterface);
	if(appinterface->error != 0)
	{
		appinterface_delete(appinterface);
		return NULL;
	}
	config_delete(config);
	return appinterface;
}

static int _new_foreach(char const * key, Hash * value,
		AppInterface * appinterface)
{
	char const prefix[] = "call::";
	int i;
	char buf[8];
	int type = AICT_VOID;
	char const * p;

	if(key == NULL || strncmp(prefix, key, sizeof(prefix) - 1) != 0)
		return 0;
	key += sizeof(prefix) - 1;
	if((p = hash_get(value, "ret")) != NULL
			&& (type = _string_enum(p, _string_type)) < 0)
	{
		appinterface->error = error_set_code(1, "%s",
				"Invalid return type");
		return -appinterface->error;
	}
	if(_new_append(appinterface, type, key) != 0)
	{
		appinterface->error = 1;
		return -appinterface->error;
	}
	for(i = 0; i < APPSERVER_MAX_ARGUMENTS; i++)
	{
		snprintf(buf, sizeof(buf), "arg%d", i + 1);
		if((p = hash_get(value, buf)) == NULL)
			break;
		if(_new_append_arg(appinterface, p) != 0)
		{
			/* FIXME may crash here? */
			appinterface->error = 1;
			return -1;
		}
	}
	return 0;
}

static int _new_append(AppInterface * ai, AppInterfaceCallType type,
		char const * function)
{
	AppInterfaceCall * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d, \"%s\")\n", __func__, type, function);
#endif
	if((p = realloc(ai->calls, sizeof(*p) * (ai->calls_cnt + 1))) == NULL)
		return -1;
	ai->calls = p;
	p = &ai->calls[ai->calls_cnt];
	if((p->name = string_new(function)) == NULL)
		return -1;
	p->type.type = type & AICT_MASK;
	p->type.direction = type & AICD_MASK;
	p->type.size = _aict_size[p->type.type];
	p->args = NULL;
	p->args_cnt = 0;
	ai->calls_cnt++;
	return 0;
}

static int _new_append_arg(AppInterface * ai, char const * arg)
{
	char buf[16];
	char * p;
	int type;
	AppInterfaceCall * q;
	AppInterfaceCallArg * r;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, arg);
#endif
	snprintf(buf, sizeof(buf), "%s", arg);
	if((p = strchr(buf, ',')) != NULL)
		*p = '\0';
	if((type = _string_enum(buf, _string_type)) < 0)
		return -1;
	q = &ai->calls[ai->calls_cnt - 1];
	if((r = realloc(q->args, sizeof(*r) * (q->args_cnt + 1))) == NULL)
		return -error_set_code(1, "%s", strerror(errno));
	q->args = r;
	r = &q->args[q->args_cnt++];
	r->type = type & AICT_MASK;
	r->direction = type & AICD_MASK;
	r->size = _aict_size[r->type];
#ifdef DEBUG
	fprintf(stderr, "DEBUG: type %s, direction: %d, size: %lu\n",
			AICTString[r->type], r->direction,
			(unsigned long)r->size);
#endif
	return 0;
}


/* appinterface_new_server */
AppInterface * appinterface_new_server(char const * app)
{
	AppInterface * ai;
	Plugin * handle;
	size_t i;
	String * name;

	if((handle = plugin_new_self()) == NULL)
		return NULL;
	if((ai = appinterface_new(app)) == NULL)
		return NULL;
	for(i = 0; i < ai->calls_cnt; i++)
	{
		name = string_new_append(ai->name, "_", ai->calls[i].name,
				NULL);
		ai->calls[i].func = plugin_lookup(handle, name);
		string_delete(name);
		if(ai->calls[i].func == NULL)
		{
			appinterface_delete(ai);
			plugin_delete(handle);
			return NULL;
		}
	}
	plugin_delete(handle);
	return ai;
}


/* appinterface_delete */
void appinterface_delete(AppInterface * appinterface)
{
	size_t i;

	for(i = 0; i < appinterface->calls_cnt; i++)
	{
		free(appinterface->calls[i].name);
		free(appinterface->calls[i].args);
	}
	free(appinterface->calls);
	string_delete(appinterface->name);
	object_delete(appinterface);
}


/* accessors */
/* appinterface_get_port */
int appinterface_get_port(AppInterface * appinterface)
{
	return appinterface->port;
}


/* appinterface_get_args_count */
int appinterface_get_args_count(AppInterface * appinterface, size_t * count,
		char const * function)
{
	AppInterfaceCall * aic;

	if((aic = _appinterface_get_call(appinterface, function)) == NULL)
		return -1;
	*count = aic->args_cnt;
	return 0;
}


/* useful */
/* appinterface_call
 * PRE
 * POST
 * 	<= 0	an error occured
 * 	else	the number of bytes added to the buffer */
static int _send_bytes(char const * data, size_t datalen, char * buf,
		size_t buflen, size_t * pos);
static int _send_string(char const * string, char * buf, size_t buflen,
		size_t * pos);

int appinterface_call(AppInterface * appinterface, char buf[], size_t buflen,
		char const * function, void ** args, va_list arg)
{
	AppInterfaceCall * aic;
	size_t pos = 0;
	size_t i;
	void * p = NULL;
	size_t size;
	double d;
	float f;
	int8_t i8;
	int16_t i16;
	int32_t i32;
	int64_t i64;
	Buffer * b = NULL;

#ifdef DEBUG
	fprintf(stderr, "%s%s%s", "DEBUG: call \"", function, "\"\n");
#endif
	if((aic = _appinterface_get_call(appinterface, function)) == NULL
			|| _send_string(function, buf, buflen, &pos) != 0)
		return -1;
	for(i = 0; i < aic->args_cnt; i++)
	{
#ifdef DEBUG
		fprintf(stderr, "%s%lu%s", "DEBUG: argument ",
				(unsigned long)i, "\n");
#endif
		size = 0;
		if(aic->args[i].direction == AICD_IN)
		{
			size = aic->args[i].size;
			switch(aic->args[i].type)
			{
				case AICT_VOID:
					break;
				case AICT_BOOL:
				case AICT_INT8:
				case AICT_UINT8:
					i8 = va_arg(arg, int);
					p = &i8;
					break;
				case AICT_INT16:
				case AICT_UINT16:
					i16 = htons(va_arg(arg, int));
					p = &i16;
					break;
				case AICT_INT32:
				case AICT_UINT32:
					i32 = htonl(va_arg(arg, int32_t));
					p = &i32;
					break;
				case AICT_DOUBLE:
					d = va_arg(arg, double);
					p = &d;
					break;
				case AICT_FLOAT:
					f = va_arg(arg, double);
					p = &f;
					break;
				case AICT_INT64: /* FIXME wrong endian */
				case AICT_UINT64:
					i64 = va_arg(arg, int64_t);
					p = &i64;
					break;
				case AICT_STRING: /* FIXME handle NULL? */
					p = va_arg(arg, String *);
					size = strlen(p) + 1;
					break;
				case AICT_BUFFER: /* FIXME handle NULL? */
					b = va_arg(arg, Buffer *);
					i32 = htonl(buffer_get_size(b));
					p = &i32;
					if(_send_bytes(p, sizeof(i32), buf,
								buflen, &pos)
							!= 0)
						return -1;
					size = buffer_get_size(b);
					p = buffer_get_data(b);
					break;
			}
		}
		else if(aic->args[i].direction == AICD_IN_OUT)
		{
			size = aic->args[i].size;
			switch(aic->args[i].type)
			{
				case AICT_VOID:
					break;
				case AICT_BOOL:
				case AICT_INT8:
				case AICT_UINT8:
					args[i] = va_arg(arg, int8_t *);
					i8 = *(int8_t *)args[i];
					p = &i8;
					break;
				case AICT_INT16:
				case AICT_UINT16:
					args[i] = va_arg(arg, int16_t *);
					i16 = htons(*(int16_t *)args[i]);
					p = &i16;
					break;
				case AICT_INT32:
				case AICT_UINT32:
				case AICT_FLOAT:
					args[i] = va_arg(arg, int32_t *);
					i32 = htonl(*(int32_t *)args[i]);
					p = &i32;
					break;
				case AICT_INT64: /* FIXME wrong endian */
				case AICT_UINT64:
				case AICT_DOUBLE:
					args[i] = va_arg(arg, int64_t *);
					i64 = *(int64_t *)args[i];
					p = &i64;
					break;
				case AICT_STRING: /* FIXME handle NULL? */
					args[i] = va_arg(arg, String **);
					p = *(char **)args[i];
					size = strlen(p) + 1;
					break;
				case AICT_BUFFER:
					b = va_arg(arg, Buffer *);
					args[i] = b;
					i32 = htonl(buffer_get_size(b));
					p = &i32;
					if(_send_bytes(p, sizeof(i32), buf,
								buflen, &pos)
							!= 0)
						return -1;
					size = buffer_get_size(b);
					p = buffer_get_data(b);
					break;
			}
		}
		else if(aic->args[i].direction == AICD_OUT)
			switch(aic->args[i].type)
			{
				case AICT_VOID:
					break;
				case AICT_BOOL:
				case AICT_INT8:
				case AICT_UINT8:
					p = va_arg(arg, int8_t *);
					args[i] = p;
					break;
				case AICT_INT16:
				case AICT_UINT16:
					p = va_arg(arg, int16_t *);
					args[i] = p;
					break;
				case AICT_INT32:
				case AICT_UINT32:
				case AICT_FLOAT:
					p = va_arg(arg, int32_t *);
					args[i] = p;
					break;
				case AICT_INT64:
				case AICT_UINT64:
				case AICT_DOUBLE:
					p = va_arg(arg, int64_t *);
					args[i] = p;
					break;
				case AICT_STRING:
					p = va_arg(arg, String **);
					args[i] = p;
					break;
				case AICT_BUFFER:
					b = va_arg(arg, Buffer *);
					args[i] = b;
					break;
			}
		if(size == 0)
			continue;
#ifdef DEBUG
		fprintf(stderr, "DEBUG: => size %lu\n", (unsigned long)size);
#endif
		if(_send_bytes(p, size, buf, buflen, &pos) != 0)
			return -1;
	}
	return pos;
}

static int _send_bytes(char const * data, size_t datalen, char * buf,
		size_t buflen, size_t * pos)
	/* FIXME the buffer is sometimes too short */
{
	if(*pos + datalen > buflen)
	{
		errno = ENOBUFS;
		return error_set_code(1, "%s", strerror(ENOBUFS));
	}
	memcpy(&buf[*pos], data, datalen);
	*pos += datalen;
	return 0;
}

static int _send_string(char const * string, char buf[], size_t buflen,
		size_t * pos)
{
	size_t i;

	if(string == NULL)
		/* XXX always send something until the protocol allows NULL */
		string = "";
	for(i = 0; *pos < buflen; i++)
	{
		buf[*pos] = string[i];
		(*pos)++;
		if(string[i] == '\0')
			return 0;
	}
	errno = ENOBUFS;
	return error_set_code(1, "%s", strerror(ENOBUFS));
}


/* appinterface_call_receive
 * PRE
 * POST
 * 	< 0	an error occured
 * 	0	not enough data ready
 * 	> 0	the amount of data read */
static String * _read_string(char buf[], size_t buflen, size_t * pos);

int appinterface_call_receive(AppInterface * appinterface, int32_t * ret,
		char buf[], size_t buflen, char const * function, void ** args)
{
	AppInterfaceCall * aic;
	size_t i;
	size_t size;
	void * v;
	char ** p;
	Buffer * b = NULL;
	uint32_t bsize;
	size_t pos = 0;
	float * f;
	int16_t * i16;
	int32_t * i32;

	if((aic = _appinterface_get_call(appinterface, function)) == NULL)
		return -1;
#ifdef DEBUG
	fprintf(stderr, "%s%s%s", "DEBUG: call \"", function, "\" receive\n");
#endif
	for(i = 0; i < aic->args_cnt; i++)
	{
#ifdef DEBUG
		fprintf(stderr, "%s%lu%s", "DEBUG: argument ",
				(unsigned long)i + 1, "\n");
#endif
		if(aic->args[i].direction == AICD_IN)
			continue;
		v = args[i];
		size = aic->args[i].size;
		switch(aic->args[i].type)
		{
			case AICT_VOID:
			case AICT_BOOL:
			case AICT_INT8:
			case AICT_UINT8:
			case AICT_INT16:
			case AICT_UINT16:
			case AICT_INT32:
			case AICT_UINT32:
			case AICT_FLOAT:
			case AICT_INT64:
			case AICT_UINT64:
			case AICT_DOUBLE:
				break; /* nothing more to do */
			case AICT_STRING:
				if((v = _read_string(buf, buflen, &pos))
						== NULL)
					return -1;
				if((p = args[i]) != NULL)
					*p = v;
				break;
			case AICT_BUFFER: /* read the size */
				b = args[i];
				v = &bsize;
				size = sizeof(bsize);
				break;
		}
		if(size == 0)
			continue;
		if(pos + size > buflen)
			return 0;
#ifdef DEBUG
		fprintf(stderr, "%s%lu%s", "DEBUG: <= size ",
				(unsigned long)size, "\n");
#endif
		memcpy(v, &buf[pos], size);
		pos += size;
		size = 0;
		switch(aic->args[i].type)
		{
			case AICT_VOID:
			case AICT_BOOL:
			case AICT_INT8:
			case AICT_UINT8:
#ifdef DEBUG
				fprintf(stderr, "%s", "DEBUG: <= int8\n");
#endif
				break; /* nothing more to do */
			case AICT_INT16:
			case AICT_UINT16:
				i16 = v;
				*i16 = ntohs(*i16);
#ifdef DEBUG
				fprintf(stderr, "%s%d%s", "DEBUG: <= int16",
						*i16, "\n");
#endif
				break;
			case AICT_INT32:
			case AICT_UINT32:
				i32 = v;
				*i32 = ntohl(*i32);
#ifdef DEBUG
				fprintf(stderr, "%s%d%s", "DEBUG: <= int32",
						*i32, "\n");
#endif
				break;
			case AICT_FLOAT:
				f = v;
#ifdef DEBUG
				fprintf(stderr, "%s%.1f%s", "DEBUG: <= float",
						*f, "\n");
#endif
				break;
			case AICT_INT64:
			case AICT_UINT64:
#ifdef DEBUG
				fprintf(stderr, "%s", "DEBUG: <= int64\n");
#endif
				break; /* FIXME wrong endian */
			case AICT_DOUBLE:
#ifdef DEBUG
				fprintf(stderr, "%s", "DEBUG: <= double\n");
#endif
				break; /* FIXME wrong endian */
			case AICT_STRING: /* already done and never reached */
				break;
			case AICT_BUFFER:
				bsize = ntohl(bsize);
				if(buffer_set_size(b, bsize) != 0)
					return -1; /* not enough space in b */
				size = bsize;
				v = buffer_get_data(b);
#ifdef DEBUG
				fprintf(stderr, "%s%lu%s", "DEBUG: <= Buffer"
						" size ", (unsigned long)size,
						"\n");
#endif
				break;
		}
		if(size == 0)
			continue;
		if(pos + size > buflen)
			return 0;
		memcpy(v, &buf[pos], size);
		pos += size;
	}
	if(pos + sizeof(*ret) > buflen) /* only the return value is left */
		return 0;
	if(ret != NULL) /* read the return value */
	{
		memcpy(ret, &buf[pos], sizeof(*ret));
		*ret = ntohl(*ret);
	}
	return pos + sizeof(*ret);
}


/* appinterface_receive */
static int _receive_args(AppInterfaceCall * call, int * ret, char buf[],
		size_t buflen, size_t * pos, char bufw[], size_t bufwlen,
		size_t * bufwpos);

int appinterface_receive(AppInterface * appinterface, int * ret, char buf[],
		size_t buflen, char bufw[], size_t bufwlen, size_t * bufwpos)
	/* FIXME should work like appinterface_call_receive */
{
	size_t pos = 0;
	String * func;
	AppInterfaceCall * aic;

	if((func = _read_string(buf, buflen, &pos)) == NULL)
		return -error_set_code(1, "%s", "Could not read the name of the"
				" call");
	aic = _appinterface_get_call(appinterface, func);
	string_delete(func);
	if(aic == NULL)
		return -1;
	if(_receive_args(aic, ret, buf, buflen, &pos, bufw, bufwlen, bufwpos)
			!= 0)
		return -1;
	return pos;
}

static String * _read_string(char buf[], size_t buflen, size_t * pos)
{
	char * str = &buf[*pos];

	for(; *pos < buflen && buf[*pos] != '\0'; (*pos)++);
	if(*pos == buflen)
		return NULL;
	(*pos)++;
	return string_new(str);
}

/* _receive_args */
static size_t _args_pre_exec(AppInterfaceCall * call, char buf[], size_t buflen,
		size_t * pos, void ** args);
static int _args_exec(AppInterfaceCall * call, int * ret, void ** args);
static size_t _args_post_exec(AppInterfaceCall * call, char buf[],
		size_t buflen, size_t * pos, void ** args, size_t i);

static int _receive_args(AppInterfaceCall * call, int * ret, char buf[],
		size_t buflen, size_t * pos, char bufw[], size_t bufwlen,
		size_t * bufwpos)
	/* FIXME _args_post_exec() sends data even when _args_exec() fails */
{
	void ** args;
	size_t i;

	if((args = malloc(sizeof(*args) * call->args_cnt)) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	if((i = _args_pre_exec(call, buf, buflen, pos, args)) != call->args_cnt)
	{
		_args_post_exec(call, bufw, bufwlen, bufwpos, args, i);
		free(args);
#ifdef DEBUG
		error_print(call->name);
#endif
		return 1;
	}
	_args_exec(call, ret, args);
	if(_args_post_exec(call, bufw, bufwlen, bufwpos, args, i) != i)
	{
		free(args);
#ifdef DEBUG
		error_print(call->name);
#endif
		return 1;
	}
	free(args);
#ifdef DEBUG
	fprintf(stderr, "%s%d%s", "DEBUG: => ", *ret, "\n");
#endif
	return 0;
}

/* _args_pre_exec
 * Prepares the arguments to execute the desired function */
static int _pre_exec_in(AppInterfaceCallArg * aica, char buf[], size_t buflen,
		size_t * pos, void * arg);
#warning IMPLEMENT THIS
/* static int _pre_exec_in_out(AppInterfaceCallArg * aica, char buf[],
		size_t buflen, size_t * pos, void * arg); */
static int _pre_exec_out(AppInterfaceCallArg * aica, void * arg);
static int _read_bytes(void * data, size_t datalen, char buf[], size_t buflen,
		size_t * pos);

static size_t _args_pre_exec(AppInterfaceCall * call, char buf[], size_t buflen,
		size_t * pos, void ** args)
	/* FIXME check calls to _read_bytes and _read_string */
{
	size_t i;
	AppInterfaceCallArg * aica;

#ifdef DEBUG
	fprintf(stderr, "%s%s(", "DEBUG: ", call->name);
#endif
	for(i = 0; i < call->args_cnt; i++)
	{
#ifdef DEBUG
		fprintf(stderr, "%s", i > 0 ? ", " : "");
#endif
		aica = &call->args[i];
		switch(aica->direction)
		{
			case AICD_IN:
#ifdef DEBUG
				fputs("in ", stderr);
#endif
				if(_pre_exec_in(aica, buf, buflen, pos,
							&args[i]) != 0)
					return i;
				break;
			case AICD_IN_OUT:
#warning IMPLEMENT THIS
#ifdef DEBUG
				fputs("in out ", stderr);
#endif
/*				if(_pre_exec_in_out(aica, buf, buflen, pos,
							&args[i]) != 0) */
					return i;
				break;
			case AICD_OUT:
#ifdef DEBUG
				fputs("out ", stderr);
#endif
				if(_pre_exec_out(aica, &args[i]) != 0)
					return i;
				break;
		}
	}
#ifdef DEBUG
	fputs(")\n", stderr);
#endif
	return i;
}

static int _pre_exec_in(AppInterfaceCallArg * aica, char buf[], size_t buflen,
		size_t * pos, void * arg)
{
	char ** p = arg;
	float ** f = arg;
	int8_t i8;
	int16_t i16;
	int32_t i32;
	long * l = arg;
	uint32_t size;
	Buffer ** b;

	switch(aica->type)
	{
		case AICT_VOID:
			break;
		case AICT_BOOL:
		case AICT_INT8:
		case AICT_UINT8:
			if(_read_bytes(&i8, sizeof(i8), buf, buflen, pos) != 0)
				return -1;
			*l = i8;
#ifdef DEBUG
			fprintf(stderr, "%ld", *l);
#endif
			break;
		case AICT_INT16:
		case AICT_UINT16:
			if(_read_bytes(&i16, sizeof(i16), buf, buflen, pos)
					!= 0)
				return -1;
			*l = ntohs(i16);
#ifdef DEBUG
			fprintf(stderr, "%ld", *l);
#endif
			break;
		case AICT_INT32:
		case AICT_UINT32:
			if(_read_bytes(&i32, sizeof(i32), buf, buflen, pos)
					!= 0)
				return -1;
			*l = ntohl(i32);
#ifdef DEBUG
			fprintf(stderr, "%ld", *l);
#endif
			break;
		case AICT_FLOAT:
			if((*f = malloc(sizeof(**f))) == NULL)
				return -1;
			if(_read_bytes(*f, sizeof(**f), buf, buflen, pos) != 0)
				return -1;
#ifdef DEBUG
			fprintf(stderr, "%.1f", **f);
#endif
			break;
		case AICT_INT64: /* FIXME not supported */
		case AICT_UINT64:
		case AICT_DOUBLE:
			errno = ENOSYS;
			return -error_set_code(1, "%s", strerror(ENOSYS));
		case AICT_BUFFER:
			if(_read_bytes(&size, sizeof(size), buf, buflen, pos)
					!= 0)
				return -1;
			size = ntohl(size);
			b = arg;
			if((*b = buffer_new(size, NULL)) == NULL)
				return -1;
			if(_read_bytes(buffer_get_data(*b), size, buf, buflen,
						pos) != 0)
			{
				buffer_delete(*b);
				return -1;
			}
#ifdef DEBUG
			fprintf(stderr, "%s", "Buffer");
#endif
			break;
		case AICT_STRING:
			if((*p = _read_string(buf, buflen, pos)) == NULL)
				return -1;
#ifdef DEBUG
			fprintf(stderr, "\"%s\"", *p);
#endif
			break;
	}
	return 0;
}

static int _pre_exec_out(AppInterfaceCallArg * aica, void * arg)
{
	char *** p;
	Buffer ** b;

	switch(aica->type)
	{
		case AICT_VOID:
			break;
		case AICT_BOOL:
		case AICT_INT8:		case AICT_UINT8:
		case AICT_INT16:	case AICT_UINT16:
		case AICT_INT32:	case AICT_UINT32:
		case AICT_INT64:	case AICT_UINT64:
			p = arg;
			if((*p = malloc(aica->size)) == NULL)
				return -1;
#ifdef DEBUG
			fputs(" integer", stderr);
#endif
			break;
		case AICT_FLOAT:	case AICT_DOUBLE:
			p = arg;
			if((*p = malloc(aica->size)) == NULL)
				return -1;
#ifdef DEBUG
			fputs(" float", stderr);
#endif
			break;
		case AICT_BUFFER:
			b = arg;
			if((*b = buffer_new(0, NULL)) == NULL)
				return -1;
#ifdef DEBUG
			fputs("Buffer", stderr);
#endif
			break;
		case AICT_STRING:
			p = arg;
			if((*p = malloc(sizeof(**p))) == NULL)
				return -1;
			**p = NULL;
#ifdef DEBUG
			fputs("String", stderr);
#endif
			break;
	}
	return 0;
}

static int _read_bytes(void * data, size_t datalen, char buf[], size_t buflen,
		size_t * pos)
{
	if(datalen > buflen - *pos)
	{
		errno = EAGAIN;
		return -error_set_code(1, "%s", strerror(EAGAIN));
	}
	memcpy(data, &buf[*pos], datalen);
	(*pos) += datalen;
	return 0;
}

static int _args_exec(AppInterfaceCall * call, int * ret, void ** args)
{
	int (*func0)(void);
	int (*func1)(void *);
	int (*func2)(void *, void *);
	int (*func3)(void *, void *, void *);
	int (*func4)(void *, void *, void *, void *);

	switch(call->args_cnt) /* FIXME not flexible */
	{
		case 0:
			func0 = call->func;
			*ret = func0();
			break;
		case 1:
			func1 = call->func;
			*ret = func1(args[0]);
			break;
		case 2:
			func2 = call->func;
			*ret = func2(args[0], args[1]);
			break;
		case 3:
			func3 = call->func;
			*ret = func3(args[0], args[1], args[2]);
			break;
		case 4:
			func4 = call->func;
			*ret = func4(args[0], args[1], args[2], args[3]);
			break;
		default:
			return error_set_code(1, "%s%lu%s", "AppInterface: "
					"functions with ",
					(unsigned long)call->args_cnt,
					"arguments are not supported");
	}
	if(call->type.type == AICT_VOID) /* avoid information leak */
		*ret = 0;
	return 0;
}

/* args_post_exec
 * Sends back data as necessary and frees previously allocated memory */
static int _post_exec_out(AppInterfaceCallArg * aica, char buf[], size_t buflen,
		size_t * pos, void * arg);
static int _post_exec_free_in(AppInterfaceCallArg * aica, void * arg);
static int _post_exec_free_out(AppInterfaceCallArg * aica, void * arg);

static size_t _args_post_exec(AppInterfaceCall * call, char buf[],
		size_t buflen, size_t * pos, void ** args, size_t i)
{
	size_t ret = i;
	size_t j;
	AppInterfaceCallArg * aica;

	if(i == call->args_cnt) /* send results */
	{
		for(j = 0; j < i; j++)
		{
			aica = &call->args[j];
			switch(aica->direction)
			{
				case AICD_IN: /* nothing to do here */
					break;
				case AICD_IN_OUT:
#warning IMPLEMENT THIS
					/* if(_post_exec_in_out(aica, args[j])
								!= 0)
						ret = j; */
					break;
				case AICD_OUT:
					if(_post_exec_out(aica, buf, buflen,
								pos, args[j])
							!= 0)
						ret = j;
					break;
			}
		}
	}
	for(j = 0; j < i; j++) /* free arguments */
	{
		aica = &call->args[j];
		switch(aica->direction)
		{
			case AICD_IN:
				_post_exec_free_in(aica, args[j]);
				break;
			case AICD_IN_OUT:
#warning IMPLEMENT THIS
				break;
			case AICD_OUT:
				_post_exec_free_out(aica, args[j]);
				break;
		}
	}
	return ret;
}

static int _post_exec_out(AppInterfaceCallArg * aica, char buf[], size_t buflen,
		size_t * pos, void * arg)
{
	int16_t * i16;
	int32_t * i32;
	Buffer * b;
	char ** p;
	uint32_t size;

	if(aica->size > buflen)
	{
		errno = ENOBUFS;
		return -error_set_code(1, "%s", strerror(ENOBUFS));
	}
	switch(aica->type)
	{
		case AICT_VOID:
			break;
		case AICT_BOOL:
		case AICT_INT8:
		case AICT_UINT8:
			if(_send_bytes(arg, aica->size, buf, buflen, pos) != 0)
				return -1;
			break;
		case AICT_INT16:
		case AICT_UINT16:
			i16 = arg;
			*i16 = htons(*i16);
			if(_send_bytes(arg, aica->size, buf, buflen, pos) != 0)
				return -1;
			break;
		case AICT_INT32:
		case AICT_UINT32:
		case AICT_FLOAT:
			i32 = arg;
			*i32 = htonl(*i32);
			if(_send_bytes(arg, aica->size, buf, buflen, pos) != 0)
				return -1;
			break;
		case AICT_INT64: /* FIXME not supported */
		case AICT_UINT64:
		case AICT_DOUBLE:
			errno = ENOSYS;
			return -error_set_code(1, "%s", strerror(ENOSYS));
		case AICT_BUFFER:
			b = arg;
			size = htonl(buffer_get_size(b)); /* size of buffer */
			if(_send_bytes((char*)&size, sizeof(size), buf, buflen,
						pos) != 0)
				return -1;
			if(_send_bytes(buffer_get_data(b), buffer_get_size(b),
					buf, buflen, pos) != 0)
				return -1;
			break;
		case AICT_STRING:
			p = arg;
			if(_send_string(*p, buf, buflen, pos) != 0)
				return -1;
			break;
	}
	return 0;
}

static int _post_exec_free_in(AppInterfaceCallArg * aica, void * arg)
{
	Buffer * b = arg;

	switch(aica->type)
	{
		case AICT_VOID:
		case AICT_BOOL:
		case AICT_INT8:		case AICT_UINT8:
		case AICT_INT16:	case AICT_UINT16:
		case AICT_INT32:	case AICT_UINT32:
			break;
		case AICT_INT64:	case AICT_UINT64:
		case AICT_DOUBLE:
			/* FIXME not supported */
			errno = ENOSYS;
			return -error_set_code(1, "%s", strerror(ENOSYS));
		case AICT_BUFFER:
			buffer_delete(b);
			break;
		case AICT_FLOAT:
		case AICT_STRING:
			free(arg);
			break;
	}
	return 0;
}

static int _post_exec_free_out(AppInterfaceCallArg * aica, void * arg)
{
	Buffer * b;

	switch(aica->type)
	{
		case AICT_VOID:
			break;
		case AICT_BOOL:
		case AICT_INT8:		case AICT_UINT8:
		case AICT_INT16:	case AICT_UINT16:
		case AICT_INT32:	case AICT_UINT32:
		case AICT_INT64:	case AICT_UINT64:
		case AICT_FLOAT:	case AICT_DOUBLE:
			free(arg);
			break;
		case AICT_BUFFER:
			b = arg;
			buffer_delete(b);
			break;
		case AICT_STRING: /* FIXME not supported */
			errno = ENOSYS;
			return -error_set_code(1, "%s", strerror(ENOSYS));
	}
	return 0;
}
