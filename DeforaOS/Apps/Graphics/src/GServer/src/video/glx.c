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
/* prototypes */
static int _glx_init(void);
static void _glx_destroy(void);
static void _glx_proto0(VideoProto0 func);
static void _glx_proto1d(VideoProto1d func, double x);
static void _glx_proto1i(VideoProto1i func, int32_t x);
static void _glx_proto3f(VideoProto3f func, float x, float y, float z);
static void _glx_proto3i(VideoProto3i func, int32_t x, int32_t y, int32_t z);
static void _glx_proto4f(VideoProto4f func, float x, float y, float z, float t);
static void _glx_swap_buffers(void);


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

static void (*_glx_func3i[VIDEO_PROTO3i_COUNT])(int32_t, int32_t, int32_t) =
{
	glColor3i,
	glVertex3i
};

static void (*_glx_func3f[VIDEO_PROTO3f_COUNT])(float, float, float) =
{
	glColor3f,
	glTranslatef,
	glVertex3f
};

static void (*_glx_func4f[VIDEO_PROTO4f_COUNT])(float, float, float, float) =
{
	glClearColor
};

static Display * _display;
static Window _window;


/* public */
/* variables */
VideoPlugin video_plugin =
{
	_glx_init,
	_glx_destroy,
	_glx_proto0,
	_glx_proto1d,
	_glx_proto1i,
	_glx_proto3f,
	_glx_proto3i,
	_glx_proto4f
};


/* private */
/* functions */
/* glx_init */
static int _glx_init(void)
{
	int screen;
	int attributes[] = { GLX_RGBA, GLX_RED_SIZE, 4, GLX_GREEN_SIZE, 4,
		GLX_BLUE_SIZE, 4, GLX_DEPTH_SIZE, 16, GLX_DOUBLEBUFFER, None };
	XVisualInfo * vi;
	GLXContext context;
	XSetWindowAttributes attr;
	Atom wdelete;
	int x;
	int y;
	unsigned int width;
	unsigned int height;
	unsigned int depth;
	Window wdummy;
	unsigned int bdummy;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	_display = XOpenDisplay(0);
	screen = DefaultScreen(_display);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() screen=%d\n", __func__, screen);
#endif
	if((vi = glXChooseVisual(_display, screen, attributes)) == NULL)
	{
		attributes[(sizeof(attributes) / sizeof(*attributes)) - 2]
			= None;
		vi = glXChooseVisual(_display, screen, attributes);
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() got visual\n", __func__);
#endif
	context = glXCreateContext(_display, vi, 0, GL_TRUE);
	memset(&attr, 0, sizeof(attr));
	attr.colormap = XCreateColormap(_display, RootWindow(_display,
				vi->screen), vi->visual, AllocNone);
	attr.border_pixel = 0;
	attr.event_mask = ExposureMask | KeyPressMask | ButtonPressMask
		| StructureNotifyMask;
	_window = XCreateWindow(_display, RootWindow(_display, vi->screen),
			0, 0, 640, 480, 0, vi->depth, InputOutput, vi->visual,
			CWBorderPixel | CWColormap | CWEventMask, &attr);
	wdelete = XInternAtom(_display, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(_display, _window, &wdelete, 1);
	XSetStandardProperties(_display, _window, "GServer GLX", "GServer GLX",
			None, NULL, 0, NULL);
	XMapRaised(_display, _window);
	glXMakeCurrent(_display, _window, context);
	XGetGeometry(_display, _window, &wdummy, &x, &y, &width, &height,
			&bdummy, &depth);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() (%d,%d) (%ux%u@%u)\n", __func__, x, y,
			width, height, depth);
#endif
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if(height == 0)
		height = 1;
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
	glFlush();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glXSwapBuffers(_display, _window);
	return 0;
}


/* glx_destroy */
static void _glx_destroy(void)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
}


/* functions */
/* glx_proto0 */
static void _glx_proto0(VideoProto0 func)
{
	if(func > VIDEO_PROTO0_LAST)
		return;
	_glx_func0[func]();
}


/* glx_proto1d */
static void _glx_proto1d(VideoProto1d func, double x)
{
	if(func > VIDEO_PROTO1d_LAST)
		return;
	_glx_func1d[func](x);
}


/* glx_proto1i */
static void _glx_proto1i(VideoProto1i func, int32_t x)
{
	if(func > VIDEO_PROTO1i_LAST)
		return;
	_glx_func1i[func](x);
}


/* glx_proto3f */
static void _glx_proto3f(VideoProto3f func, float x, float y, float z)
{
	if(func > VIDEO_PROTO3f_LAST)
		return;
	_glx_func3f[func](x, y, z);
}


/* glx_proto3i */
static void _glx_proto3i(VideoProto3i func, int32_t x, int32_t y, int32_t z)
{
	if(func > VIDEO_PROTO3i_LAST)
		return;
	_glx_func3i[func](x, y, z);
}


/* glx_proto4f */
static void _glx_proto4f(VideoProto4f func, float x, float y, float z, float t)
{
	if(func > VIDEO_PROTO4f_LAST)
		return;
	_glx_func4f[func](x, y, z, t);
}


/* glx_swap_buffers */
static void _glx_swap_buffers(void)
{
	glXSwapBuffers(_display, _window);
}
