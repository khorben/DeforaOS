/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS GServer */



#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <System.h>
#include "video/video.h"


/* GServer */
/* types */
typedef struct _GServer
{
	Event * event;
	AppServer * appserver;
	int loop;
} GServer;


/* functions */
static int _gserver_init(GServer * gserver);
static void _gserver_destroy(GServer * gserver);

static int _gserver(void)
{
	GServer gserver;

	if(_gserver_init(&gserver) != 0)
		return 2;
	while(gserver.loop)
	{
		/* FIXME */
		gserver.loop = 0;
	}
	_gserver_destroy(&gserver);
	return 2;
}

static int _init_video(GServer * gs);
static int _gserver_init(GServer * gs)
{
	memset(gs, 0, sizeof(GServer));
	if((gs->event = event_new()) != NULL
			&& (gs->appserver = appserver_new_event("GServer",
					ASO_LOCAL, gs->event)) != NULL
			&& _init_video(gs) == 0)
	{
		gs->loop = 1;
		return 0;
	}
	event_delete(gs->event);
	if(gs->appserver)
		appserver_delete(gs->appserver);
	return 1;
}

static void _gserver_destroy(GServer * gs)
{
	if(gs->appserver)
		appserver_delete(gs->appserver);
	if(gs->event)
		event_delete(gs->event);
}


/* video */
static int _init_video(GServer * gs)
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


/* main */
int main(int argc, char * argv[])
{
	return _gserver();
}
