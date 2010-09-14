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
#include <GL/glut.h>
#include "video.h"


/* GLUT */
/* private */
/* types */
typedef struct _GLUTPlugin
{
	unsigned int width;
	unsigned int height;
} GLUTPlugin;


/* prototypes */
static int _glut_init(VideoPlugin * plugin);
static void _glut_destroy(VideoPlugin * plugin);

static void _glut_display(void);
static void _glut_idle(void);

static void _glut_proto0(VideoPlugin * plugin, VideoProto0 func);
static void _glut_proto1d(VideoPlugin * plugin, VideoProto1d func, double x);
static void _glut_proto1i(VideoPlugin * plugin, VideoProto1i func, int32_t x);
static void _glut_proto3f(VideoPlugin * plugin, VideoProto3f func, float x,
		float y, float z);
static void _glut_proto3i(VideoPlugin * plugin, VideoProto3i func, int32_t x,
		int32_t y, int32_t z);
static void _glut_proto4f(VideoPlugin * plugin, VideoProto4f func, float x,
		float y, float z, float t);

static void _glut_swap_buffers(void);


/* variables */
static void (*_glut_func0[VIDEO_PROTO0_COUNT])(void) =
{
	glEnd,
	glFlush,
	glLoadIdentity,
	_glut_swap_buffers
};

static void (*_glut_func1d[VIDEO_PROTO1d_COUNT])(double) =
{
	glClearDepth
};

static void (*_glut_func1i[VIDEO_PROTO1i_COUNT])(uint32_t) =
{
	glBegin,
	glClear
};

static void (*_glut_func3f[VIDEO_PROTO3f_COUNT])(float, float, float) =
{
	glColor3f,
	glTranslatef,
	glVertex3f
};

static void (*_glut_func3i[VIDEO_PROTO3i_COUNT])(int32_t, int32_t, int32_t) =
{
	glColor3i,
	glVertex3i
};

static void (*_glut_func4f[VIDEO_PROTO4f_COUNT])(float, float, float, float) =
{
	glClearColor,
	glRotatef
};


/* public */
/* variables */
VideoPlugin video_plugin =
{
	NULL,
	"GLUT",
	_glut_init,
	_glut_destroy,
	_glut_proto0,
	_glut_proto1d,
	_glut_proto1i,
	_glut_proto3f,
	_glut_proto3i,
	_glut_proto4f,
	NULL
};


/* private */
/* functions */
/* glut_init */
static int _glut_init(VideoPlugin * plugin)
{
	GLUTPlugin * glut;
	int argc = 1;
	char * argv[] = { "GServer", NULL };

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if((glut = object_new(sizeof(*glut))) == NULL)
		return 1;
	plugin->priv = glut;
	glut->width = 640;
	glut->height = 480;
	glutInit(&argc, argv);
	glutInitWindowSize(glut->width, glut->height);
	glutCreateWindow("GServer GLUT");
	glutDisplayFunc(_glut_display);
	glutIdleFunc(_glut_idle);
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glViewport(0, 0, glut->width, glut->height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if(glut->height == 0)
		glut->height = 1;
	gluPerspective(45.0, (GLfloat)glut->width / (GLfloat)glut->height, 0.1,
			100.0);
	glMatrixMode(GL_MODELVIEW);
	glFlush();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glutSwapBuffers();
	glutMainLoop();
	return 0;
}


/* glut_destroy */
static void _glut_destroy(VideoPlugin * plugin)
{
	GLUTPlugin * glut = plugin->priv;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	/* FIXME take care of the rest */
	object_delete(glut);
}


/* glut_display */
static void _glut_display(void)
{
	video_plugin.helper->refresh(video_plugin.helper->gserver);
}


/* glut_idle */
static int _idle_timeout(void * data);

static void _glut_idle(void)
{
	struct timeval tv;
	Event * event;

	event = video_plugin.helper->get_event(video_plugin.helper->gserver);
#ifdef DEBUG
	tv.tv_sec = 1;
	tv.tv_usec = 0;
#else
	tv.tv_sec = 0;
	tv.tv_usec = 1000000 / 25;
#endif
	event_register_timeout(event, &tv, _idle_timeout, event);
	event_loop(event);
}

static int _idle_timeout(void * data)
{
	Event * event = data;

	event_loop_quit(event);
	return 1;
}


/* glut_proto0 */
static void _glut_proto0(VideoPlugin * plugin, VideoProto0 func)
{
	if(func == VIDEO_PROTO0_SwapBuffers)
	{
		glutSwapBuffers();
		return;
	}
	_glut_func0[func]();
}


/* glut_proto1d */
static void _glut_proto1d(VideoPlugin * plugin, VideoProto1d func, double x)
{
	_glut_func1d[func](x);
}


/* glut_proto1i */
static void _glut_proto1i(VideoPlugin * plugin, VideoProto1i func, int32_t x)
{
	_glut_func1i[func](x);
}


/* glut_proto3f */
static void _glut_proto3f(VideoPlugin * plugin, VideoProto3f func, float x,
		float y, float z)
{
	_glut_func3f[func](x, y, z);
}


/* glut_proto3i */
static void _glut_proto3i(VideoPlugin * plugin, VideoProto3i func, int32_t x,
		int32_t y, int32_t z)
{
	_glut_func3i[func](x, y, z);
}


/* glut_proto4f */
static void _glut_proto4f(VideoPlugin * plugin, VideoProto4f func, float x,
		float y, float z, float t)
{
	_glut_func4f[func](x, y, z, t);
}


/* glut_swap_buffers */
static void _glut_swap_buffers(void)
{
	/* no need to do anything */
}
