/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Graphics GServer */



#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include "video/video.h"
#include "gserver.h"


/* GServer */
/* private */
/* types */
struct _GServer
{
	Event * event;
	int event_own;
	AppServer * appserver;
	int loop;
};


/* public */
/* functions */
/* gserver_new */
static int _new_init(GServer * gserver, Event * event);
static int _init_video(GServer * gserver);

GServer * gserver_new(Event * event)
{
	GServer * gserver;

	if((gserver = object_new(sizeof(*gserver))) == NULL)
		return NULL;
	if(_new_init(gserver, event) != 0)
	{
		object_delete(gserver);
		return NULL;
	}
	return gserver;
}

static int _new_init(GServer * gserver, Event * event)
{
	if((gserver->event = event) != NULL)
		gserver->event_own = 0;
	else if((gserver->event = event_new()) == NULL)
		return -1;
	else
		gserver->event_own = 1;
	if((gserver->appserver = appserver_new_event("GServer",
					ASO_LOCAL, gserver->event)) != NULL
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
	/* FIXME ask Hardware what to load instead of hard-coding vesa */
{
	void * handle;
	VideoPlugin * video;

	if((handle = dlopen("video/vesa.so", RTLD_LAZY)) == NULL)
	{
		fprintf(stderr, "%s%s%s", "GServer: vesa: ", dlerror(), "\n");
		return 1;
	}
	if((video = dlsym(handle, "video_plugin")) == NULL)
	{
		fprintf(stderr, "%s%s%s", "GServer: vesa: ", dlerror(), "\n");
		dlclose(handle);
		return 1;
	}
	video->init();
	return 0;
}


/* gserver_delete */
void gserver_delete(GServer * gserver)
{
	if(gserver->appserver != NULL)
		appserver_delete(gserver->appserver);
	if(gserver->event != NULL)
		event_delete(gserver->event);
	object_delete(gserver);
}


/* useful */
int gserver_loop(GServer * gserver)
{
	int ret = 0;

	while(gserver->loop == 1)
		ret |= event_loop(gserver->event);
	return ret;
}
