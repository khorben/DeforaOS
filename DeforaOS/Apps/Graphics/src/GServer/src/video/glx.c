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



#ifdef DEBUG
# include <stdio.h>
#endif
#include <string.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include "video.h"


/* GLX */
/* private */
/* types */
typedef struct _GLXPlugin
{
	Display * display;
	int screen;
	Window window;
	int double_buffered;
	GLXContext context;
	unsigned int width;
	unsigned int height;
} GLXPlugin;


/* prototypes */
static int _glx_init(VideoPlugin * plugin);
static void _glx_destroy(VideoPlugin * plugin);

static void _glx_proto0(VideoPlugin * plugin, VideoProto0 func);
static void _glx_proto1d(VideoPlugin * plugin, VideoProto1d func, double x);
static void _glx_proto1i(VideoPlugin * plugin, VideoProto1i func, int32_t x);
static void _glx_proto3f(VideoPlugin * plugin, VideoProto3f func, float x,
		float y, float z);
static void _glx_proto3i(VideoPlugin * plugin, VideoProto3i func, int32_t x,
		int32_t y, int32_t z);
static void _glx_proto4f(VideoPlugin * plugin, VideoProto4f func, float x,
		float y, float z, float t);

static void _glx_swap_buffers(void);
static int _glx_timeout(void * data);


/* variables */
static void (*_glx_func0[VIDEO_PROTO0_COUNT])(void) =
{
	glEnd,
	glFlush,
	glLoadIdentity,
	_glx_swap_buffers
};

static void (*_glx_func1d[VIDEO_PROTO1d_COUNT])(double) =
{
	glClearDepth
};

static void (*_glx_func1i[VIDEO_PROTO1i_COUNT])(uint32_t) =
{
	glBegin,
	glClear
};

static void (*_glx_func3f[VIDEO_PROTO3f_COUNT])(float, float, float) =
{
	glColor3f,
	glTranslatef,
	glVertex3f
};

static void (*_glx_func3i[VIDEO_PROTO3i_COUNT])(int32_t, int32_t, int32_t) =
{
	glColor3i,
	glVertex3i
};

static void (*_glx_func4f[VIDEO_PROTO4f_COUNT])(float, float, float, float) =
{
	glClearColor,
	glRotatef
};


/* public */
/* variables */
VideoPlugin video_plugin =
{
	NULL,
	"GLX",
	_glx_init,
	_glx_destroy,
	_glx_proto0,
	_glx_proto1d,
	_glx_proto1i,
	_glx_proto3f,
	_glx_proto3i,
	_glx_proto4f,
	NULL
};


/* private */
/* functions */
/* glx_init */
static int _glx_init(VideoPlugin * plugin)
{
	GLXPlugin * glx;
	Event * event;
	Window root;
	int attributes[] = { GLX_RGBA, GLX_RED_SIZE, 4, GLX_GREEN_SIZE, 4,
		GLX_BLUE_SIZE, 4, GLX_DEPTH_SIZE, 16, GLX_DOUBLEBUFFER, None };
	XVisualInfo * vi;
	XSetWindowAttributes attr;
	Atom wdelete;
	int x;
	int y;
	unsigned int depth;
	Window wdummy;
	unsigned int bdummy;
	struct timeval tv;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if((glx = object_new(sizeof(*glx))) == NULL)
		return 1;
	plugin->priv = glx;
	event = plugin->helper->get_event(plugin->helper->gserver);
	glx->display = XOpenDisplay(NULL);
	glx->screen = DefaultScreen(glx->display);
	glx->double_buffered = 1;
	if((vi = glXChooseVisual(glx->display, glx->screen, attributes))
			== NULL)
	{
		glx->double_buffered = 0;
		attributes[(sizeof(attributes) / sizeof(*attributes)) - 2]
			= None;
		vi = glXChooseVisual(glx->display, glx->screen, attributes);
	}
	glx->context = glXCreateContext(glx->display, vi, 0, GL_TRUE);
	glx->width = 640;
	glx->height = 480;
	memset(&attr, 0, sizeof(attr));
	root = RootWindow(glx->display, vi->screen);
	attr.colormap = XCreateColormap(glx->display, root, vi->visual,
			AllocNone);
	attr.border_pixel = 0;
	attr.event_mask = ExposureMask | KeyPressMask | ButtonPressMask
		| StructureNotifyMask;
	glx->window = XCreateWindow(glx->display, root, 0, 0, glx->width,
			glx->height, 0, vi->depth, InputOutput, vi->visual,
			CWBorderPixel | CWColormap | CWEventMask, &attr);
	wdelete = XInternAtom(glx->display, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(glx->display, glx->window, &wdelete, 1);
	XSetStandardProperties(glx->display, glx->window, "GServer GLX",
			"GServer GLX", None, NULL, 0, NULL);
	XMapRaised(glx->display, glx->window);
	glXMakeCurrent(glx->display, glx->window, glx->context);
	XGetGeometry(glx->display, glx->window, &wdummy, &x, &y, &glx->width,
			&glx->height, &bdummy, &depth);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() (%d,%d) (%ux%u@%u)\n", __func__, x, y,
			glx->width, glx->height, depth);
	fprintf(stderr, "DEBUG: Direct rendering: %s\n", glXIsDirect(
				glx->display, glx->context) ? "Yes" : "No");
#endif
	glShadeModel(GL_SMOOTH);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: glClearColor(%.1f, %.1f, %.1f, %.1f)\n", 0.0,
			0.0, 0.0, 0.0);
#endif
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glViewport(0, 0, glx->width, glx->height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if(glx->height == 0)
		glx->height = 1;
	gluPerspective(45.0, (GLfloat)glx->width / (GLfloat)glx->height, 0.1,
			100.0);
	glMatrixMode(GL_MODELVIEW);
	glFlush();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glXSwapBuffers(glx->display, glx->window);
#ifdef DEBUG
	tv.tv_sec = 1;
	tv.tv_usec = 0;
#else
	tv.tv_sec = 0;
	tv.tv_usec = 1000000 / 25;
#endif
	if(_glx_timeout(plugin) == 0)
		event_register_timeout(event, &tv, _glx_timeout, plugin);
	return 0;
}


/* glx_destroy */
static void _glx_destroy(VideoPlugin * plugin)
{
	GLXPlugin * glx = plugin->priv;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(glx->display != NULL)
		XCloseDisplay(glx->display);
	/* FIXME take care of the rest */
	object_delete(glx);
}


/* functions */
/* glx_proto0 */
static void _glx_proto0(VideoPlugin * plugin, VideoProto0 func)
{
	GLXPlugin * glx = plugin->priv;

	if(func == VIDEO_PROTO0_SwapBuffers)
	{
		if(glx->double_buffered != 0)
			glXSwapBuffers(glx->display, glx->window);
		return;
	}
	_glx_func0[func]();
}


/* glx_proto1d */
static void _glx_proto1d(VideoPlugin * plugin, VideoProto1d func, double x)
{
	_glx_func1d[func](x);
}


/* glx_proto1i */
static void _glx_proto1i(VideoPlugin * plugin, VideoProto1i func, int32_t x)
{
	_glx_func1i[func](x);
}


/* glx_proto3f */
static void _glx_proto3f(VideoPlugin * plugin, VideoProto3f func, float x,
		float y, float z)
{
	_glx_func3f[func](x, y, z);
}


/* glx_proto3i */
static void _glx_proto3i(VideoPlugin * plugin, VideoProto3i func, int32_t x,
		int32_t y, int32_t z)
{
	_glx_func3i[func](x, y, z);
}


/* glx_proto4f */
static void _glx_proto4f(VideoPlugin * plugin, VideoProto4f func, float x,
		float y, float z, float t)
{
	_glx_func4f[func](x, y, z, t);
}


/* glx_swap_buffers */
static void _glx_swap_buffers(void)
{
	/* no need to do anything */
}


/* glx_timeout */
static int _glx_timeout(void * data)
{
	VideoPlugin * plugin = data;
	GLXPlugin * glx = plugin->priv;
	XEvent event;
	unsigned int w;
	unsigned int h;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	while(XPending(glx->display) > 0)
	{
		XNextEvent(glx->display, &event);
		switch(event.type)
		{
			case ConfigureNotify:
				if(event.xconfigure.width < 0
						|| event.xconfigure.height < 0)
					break;
				w = event.xconfigure.width;
				h = event.xconfigure.height;
				if(w == glx->width && h == glx->height)
					break;
				glx->width = w;
				glx->height = h;
				glViewport(0, 0, w, h);
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				gluPerspective(45.0, (GLfloat)w / (GLfloat)h,
						0.1, 100.0);
				glMatrixMode(GL_MODELVIEW);
				break;
		}
	}
	plugin->helper->refresh(plugin->helper->gserver);
	return 0;
}
