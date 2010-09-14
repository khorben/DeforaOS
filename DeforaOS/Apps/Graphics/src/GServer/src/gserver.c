/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Graphics GServer */
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



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <GL/gl.h>
#include "video/video.h"
#include "../data/GServer.h"
#include "gserver.h"
#include "../config.h"

#ifdef DEBUG
# define DEBUG_INTERFACE() fprintf(stderr, "DEBUG: %s()\n", __func__)
# define DEBUG_INTERFACE1i(x) fprintf(stderr, "DEBUG: %s(0x%x)\n", __func__, x)
# define DEBUG_INTERFACE3f(x, y, z) fprintf(stderr, \
		"DEBUG: %s(%.1f, %.1f, %.1f)\n", __func__, *x, *y, *z)
# define DEBUG_INTERFACE3i(x, y, z) fprintf(stderr, \
		"DEBUG: %s(%d, %d, %d)\n", __func__, x, y, z)
# define DEBUG_INTERFACE4f(x, y, z, t) fprintf(stderr, \
		"DEBUG: %s(%.1f, %.1f, %.1f, %.1f)\n", __func__, *x, *y, *z, *t)
#else
# define DEBUG_INTERFACE()
# define DEBUG_INTERFACE1i(x)
# define DEBUG_INTERFACE3f(x, y, z)
# define DEBUG_INTERFACE3i(x, y, z)
# define DEBUG_INTERFACE4f(x, y, z, t)
#endif


/* GServer */
/* private */
/* types */
typedef struct _GServerCall GServerCall;
typedef struct _GServerClient GServerClient;

struct _GServer
{
	Event * event;
	int event_own;
	AppServer * appserver;
	int loop;

	/* plugins */
	/* video */
	VideoPluginHelper video_helper;
	void * video_handle;
	VideoPlugin * video_plugin;

	/* clients */
	GServerClient * clients;
	size_t clients_cnt;
};

struct _GServerCall
{
	VideoProto type;
	unsigned int func;
	union
	{
		struct
		{
			double x;
		} _1d;
		struct
		{
			int32_t x;
		} _1i;
		struct
		{
			float x;
			float y;
			float z;
		} _3f;
		struct
		{
			int32_t x;
			int32_t y;
			int32_t z;
		} _3i;
		struct
		{
			float x;
			float y;
			float z;
			float t;
		} _4f;
	} args;
};

struct _GServerClient
{
	void * id;
	GServerCall * calls;
	size_t calls_cnt;
};


/* variables */
static GServer * _gserver = NULL;


/* prototypes */
static void _gserver_client_calls(GServer * gserver, GServerClient * client);
static GServerClient * _gserver_get_client(GServer * gserver, void * id);

/* queue */
static GServerCall * _gserver_queue(GServer * gserver, VideoProto type,
		unsigned int func);
static int _gserver_queue0(GServer * gserver, VideoProto0 func);
static int _gserver_queue1d(GServer * gserver, VideoProto1d func, double x);
static int _gserver_queue1i(GServer * gserver, VideoProto1i func, int32_t x);
static int _gserver_queue3f(GServer * gserver, VideoProto3f func, float x,
		float y, float z);
static int _gserver_queue3i(GServer * gserver, VideoProto3i func, int32_t x,
		int32_t y, int32_t z);
static int _gserver_queue4f(GServer * gserver, VideoProto4f func, float x,
		float y, float z, float t);


/* public */
/* functions */
/* gserver_new */
static int _new_init(AppServerOptions options, GServer * gserver,
		Event * event);
static int _init_video(GServer * gserver);

GServer * gserver_new(AppServerOptions options, Event * event)
{
	GServer * gserver;

	if((gserver = object_new(sizeof(*gserver))) == NULL)
		return NULL;
	_gserver = gserver;
	if(_new_init(options, gserver, event) != 0)
	{
		object_delete(gserver);
		return NULL;
	}
	return gserver;
}

static int _new_init(AppServerOptions options, GServer * gserver, Event * event)
{
	gserver->video_handle = NULL;
	gserver->video_plugin = NULL;
	gserver->clients = NULL;
	gserver->clients_cnt = 0;
	if((gserver->event = event) != NULL)
		gserver->event_own = 0;
	else if((gserver->event = event_new()) == NULL)
		return -1;
	else
		gserver->event_own = 1;
	gserver->video_helper.gserver = gserver;
	gserver->video_helper.get_event = gserver_get_event;
	gserver->video_helper.refresh = gserver_refresh;
	if((gserver->appserver = appserver_new_event("GServer", options,
					gserver->event)) != NULL
			&& _init_video(gserver) == 0)
	{
		gserver->loop = 1;
		return 0;
	}
	if(gserver->appserver != NULL)
		appserver_delete(gserver->appserver);
	if(gserver->event_own != 0)
		event_delete(gserver->event);
	return -1;
}

static int _init_video(GServer * gserver)
	/* FIXME ask Hardware what to load instead of hard-coding glx */
{
	char const filename[] = PREFIX "/lib/" PACKAGE "/video/glx.so";

	if((gserver->video_handle = dlopen(filename, RTLD_LAZY)) == NULL)
		return error_set_code(1, "%s: %s", filename, dlerror());
	if((gserver->video_plugin = dlsym(gserver->video_handle,
					"video_plugin")) == NULL)
	{
		error_set_code(1, "%s: %s", filename, dlerror());
		dlclose(gserver->video_handle);
		return 1;
	}
	gserver->video_plugin->helper = &gserver->video_helper;
	gserver->video_plugin->init(gserver->video_plugin);
	return 0;
}


/* gserver_delete */
static void _destroy_video(GServer * gserver);

void gserver_delete(GServer * gserver)
{
	if(_gserver == gserver)
		_gserver = NULL;
	_destroy_video(gserver);
	if(gserver->appserver != NULL)
		appserver_delete(gserver->appserver);
	if(gserver->event != NULL)
		event_delete(gserver->event);
	object_delete(gserver);
}

static void _destroy_video(GServer * gserver)
{
	if(gserver->video_plugin != NULL)
		gserver->video_plugin->destroy(gserver->video_plugin);
	if(gserver->video_handle != NULL)
		dlclose(gserver->video_handle);
}


/* accessors */
/* gserver_get_event */
Event * gserver_get_event(GServer * gserver)
{
	return gserver->event;
}


/* useful */
/* gserver_loop */
int gserver_loop(GServer * gserver)
{
	int ret = 0;

	while(gserver->loop == 1)
		ret |= event_loop(gserver->event);
	return ret;
}


/* gserver_refresh */
void gserver_refresh(GServer * gserver)
{
	size_t i;

	gserver->video_plugin->proto1i(gserver->video_plugin,
			VIDEO_PROTO1i_glClear, GL_COLOR_BUFFER_BIT
			| GL_DEPTH_BUFFER_BIT);
	gserver->video_plugin->proto0(gserver->video_plugin,
			VIDEO_PROTO0_glLoadIdentity);
	for(i = 0; i < gserver->clients_cnt; i++)
		_gserver_client_calls(gserver, &gserver->clients[i]);
	gserver->video_plugin->proto0(gserver->video_plugin,
			VIDEO_PROTO0_SwapBuffers);
}


/* interface */
#define GSERVER_PROTO0(func) \
	void GServer_ ## func (void) \
{ \
	DEBUG_INTERFACE(); \
	_gserver_queue0(_gserver, VIDEO_PROTO0_ ## func); \
}
#define GSERVER_PROTO1d(func) \
	void GServer_ ## func(double * x) \
{ \
	DEBUG_INTERFACE(); \
	_gserver_queue1d(_gserver, VIDEO_PROTO1d_ ## func, *x); \
}
#define GSERVER_PROTO1i(func, type1) \
	void GServer_ ## func (type1 x) \
{ \
	DEBUG_INTERFACE1i(x); \
	_gserver_queue1i(_gserver, VIDEO_PROTO1i_ ## func, x); \
}
#define GSERVER_PROTO3f(func) \
	void GServer_ ## func (float * x, float * y, float * z) \
{ \
	DEBUG_INTERFACE3f(x, y, z); \
	_gserver_queue3f(_gserver, VIDEO_PROTO3f_ ## func, *x, *y, *z); \
}
#define GSERVER_PROTO3i(func, type1, type2, type3) \
	void GServer_ ## func (int32_t x, int32_t y, int32_t z) \
{ \
	DEBUG_INTERFACE3i(x, y, z); \
	_gserver_queue3i(_gserver, VIDEO_PROTO3i_ ## func, x, y, z); \
}
#define GSERVER_PROTO4f(func) \
	void GServer_ ## func (float * x, float * y, float * z, float * t) \
{ \
	DEBUG_INTERFACE4f(x, y, z, t); \
	_gserver_queue4f(_gserver, VIDEO_PROTO4f_ ## func, *x, *y, *z, *t); \
}

/* proto0 */
GSERVER_PROTO0(glEnd)
GSERVER_PROTO0(glLoadIdentity)
GSERVER_PROTO0(glFlush)
GSERVER_PROTO0(SwapBuffers)

/* proto1d */
GSERVER_PROTO1d(glClearDepth)

/* proto1i */
GSERVER_PROTO1i(glBegin, uint32_t)
GSERVER_PROTO1i(glClear, uint32_t)

/* proto3f */
GSERVER_PROTO3f(glColor3f)
GSERVER_PROTO3f(glTranslatef)
GSERVER_PROTO3f(glVertex3f)

/* proto3i */
GSERVER_PROTO3i(glColor3i, int32_t, int32_t, int32_t)
GSERVER_PROTO3i(glVertex3i, int32_t, int32_t, int32_t)

/* proto4f */
GSERVER_PROTO4f(glClearColor)
GSERVER_PROTO4f(glRotatef)


/* private */
/* functions */
/* gserver_client_calls */
static void _gserver_client_calls(GServer * gserver, GServerClient * client)
{
	size_t i;
	GServerCall * call;
	VideoPlugin * vp = gserver->video_plugin;

	for(i = 0; i < client->calls_cnt; i++)
	{
		call = &client->calls[i];
		switch(call->type)
		{
			case VIDEO_PROTO_0:
				vp->proto0(vp, call->func);
				break;
			case VIDEO_PROTO_1d:
				vp->proto1d(vp, call->func, call->args._1d.x);
				break;
			case VIDEO_PROTO_1i:
				vp->proto1i(vp, call->func, call->args._1i.x);
				break;
			case VIDEO_PROTO_3f:
				vp->proto3f(vp, call->func, call->args._3f.x,
						call->args._3f.y,
						call->args._3f.z);
				break;
			case VIDEO_PROTO_3i:
				vp->proto3i(vp, call->func, call->args._3i.x,
						call->args._3i.y,
						call->args._3i.z);
				break;
			case VIDEO_PROTO_4f:
				vp->proto4f(vp, call->func, call->args._4f.x,
						call->args._4f.y,
						call->args._4f.z,
						call->args._4f.t);
				break;
		}
	}
}


/* gserver_get_client */
static GServerClient * _gserver_get_client(GServer * gserver, void * id)
{
	GServerClient * ret;
	size_t i;

	for(i = 0; i < gserver->clients_cnt; i++)
		if(gserver->clients[i].id == id)
			return &gserver->clients[i];
	if((ret = realloc(gserver->clients, sizeof(*ret) * (gserver->clients_cnt
						+ 1))) == NULL)
		return NULL;
	gserver->clients = ret;
	ret = &gserver->clients[gserver->clients_cnt++];
	ret->id = id;
	ret->calls = NULL;
	ret->calls_cnt = 0;
	return ret;
}


/* gserver_queue */
static GServerCall * _gserver_queue(GServer * gserver, VideoProto type,
		unsigned int func)
{
	GServerCall * ret = NULL;
	void * id;
	GServerClient * gsc;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, %u, %u)\n", __func__, gserver, type,
			func);
#endif
	if((id = appserver_get_client_id(gserver->appserver)) == NULL)
		return NULL;
	if((gsc = _gserver_get_client(gserver, id)) == NULL)
		return NULL;
	if((ret = realloc(gsc->calls, sizeof(*ret) * (gsc->calls_cnt + 1)))
			== NULL)
		return NULL;
	gsc->calls = ret;
	ret = &gsc->calls[gsc->calls_cnt++];
	ret->type = type;
	ret->func = func;
#if 0 /* XXX probably hurts performance without protecting anything */
	memset(&gsc->args, 0, sizeof(gsc->args));
#endif
	return ret;
}


/* gserver_queue0 */
static int _gserver_queue0(GServer * gserver, VideoProto0 func)
{
	GServerCall * gsc;

	if((gsc = _gserver_queue(gserver, VIDEO_PROTO_0, func)) == NULL)
		return -1;
	/* FIXME intercept SwapBuffers() and glClear() */
	return 0;
}


/* gserver_queue1d */
static int _gserver_queue1d(GServer * gserver, VideoProto1d func, double x)
{
	GServerCall * gsc;

	if((gsc = _gserver_queue(gserver, VIDEO_PROTO_1d, func)) == NULL)
		return -1;
	gsc->args._1d.x = x;
	return 0;
}


/* gserver_queue1i */
static int _gserver_queue1i(GServer * gserver, VideoProto1i func, int32_t x)
{
	GServerCall * gsc;

	if((gsc = _gserver_queue(gserver, VIDEO_PROTO_1i, func)) == NULL)
		return -1;
	gsc->args._1i.x = x;
	return 0;
}


/* gserver_queue3f */
static int _gserver_queue3f(GServer * gserver, VideoProto3f func, float x,
		float y, float z)
{
	GServerCall * gsc;

	if((gsc = _gserver_queue(gserver, VIDEO_PROTO_3f, func)) == NULL)
		return -1;
	gsc->args._3f.x = x;
	gsc->args._3f.y = y;
	gsc->args._3f.z = z;
	return 0;
}


/* gserver_queue3i */
static int _gserver_queue3i(GServer * gserver, VideoProto3i func, int32_t x,
		int32_t y, int32_t z)
{
	GServerCall * gsc;

	if((gsc = _gserver_queue(gserver, VIDEO_PROTO_3i, func)) == NULL)
		return -1;
	gsc->args._3i.x = x;
	gsc->args._3i.y = y;
	gsc->args._3i.z = z;
	return 0;
}


/* gserver_queue4f */
static int _gserver_queue4f(GServer * gserver, VideoProto4f func, float x,
		float y, float z, float t)
{
	GServerCall * gsc;

	if((gsc = _gserver_queue(gserver, VIDEO_PROTO_4f, func)) == NULL)
		return -1;
	gsc->args._4f.x = x;
	gsc->args._4f.y = y;
	gsc->args._4f.z = z;
	gsc->args._4f.t = t;
	return 0;
}
