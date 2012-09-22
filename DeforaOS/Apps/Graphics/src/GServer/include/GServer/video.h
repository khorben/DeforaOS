/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
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



#ifndef GRAPHICS_GSERVER_VIDEO_H
# define GRAPHICS_GSERVER_VIDEO_H

# include <stdint.h>
# include <System.h>
# include "gserver.h"


/* Video */
/* types */
typedef enum _GServerVideoProto
{
	GSERVER_VIDEO_PROTO_0 = 0,
	GSERVER_VIDEO_PROTO_1d,
	GSERVER_VIDEO_PROTO_1i,
	GSERVER_VIDEO_PROTO_3f,
	GSERVER_VIDEO_PROTO_3i,
	GSERVER_VIDEO_PROTO_4f
} GServerVideoProto;

typedef enum _GServerVideoProto0
{
	GSERVER_VIDEO_PROTO0_glEnd = 0,
	GSERVER_VIDEO_PROTO0_glFlush,
	GSERVER_VIDEO_PROTO0_glLoadIdentity,
	GSERVER_VIDEO_PROTO0_SwapBuffers
} GServerVideoProto0;
# define GSERVER_VIDEO_PROTO0_LAST GSERVER_VIDEO_PROTO0_SwapBuffers
# define GSERVER_VIDEO_PROTO0_COUNT (GSERVER_VIDEO_PROTO0_LAST + 1)

typedef enum _GServerVideoProto1d
{
	GSERVER_VIDEO_PROTO1d_glClearDepth = 0
} GServerVideoProto1d;
# define GSERVER_VIDEO_PROTO1d_LAST GSERVER_VIDEO_PROTO1d_glClearDepth
# define GSERVER_VIDEO_PROTO1d_COUNT (GSERVER_VIDEO_PROTO1d_LAST + 1)

typedef enum _GServerVideoProto1i
{
	GSERVER_VIDEO_PROTO1i_glBegin = 0,
	GSERVER_VIDEO_PROTO1i_glClear
} GServerVideoProto1i;
# define GSERVER_VIDEO_PROTO1i_LAST GSERVER_VIDEO_PROTO1i_glClear
# define GSERVER_VIDEO_PROTO1i_COUNT (GSERVER_VIDEO_PROTO1i_LAST + 1)

typedef enum _GServerVideoProto3f
{
	GSERVER_VIDEO_PROTO3f_glColor3f = 0,
	GSERVER_VIDEO_PROTO3f_glTranslatef,
	GSERVER_VIDEO_PROTO3f_glVertex3f
} GServerVideoProto3f;
# define GSERVER_VIDEO_PROTO3f_LAST GSERVER_VIDEO_PROTO3f_glVertex3f
# define GSERVER_VIDEO_PROTO3f_COUNT (GSERVER_VIDEO_PROTO3f_LAST + 1)

typedef enum _GServerVideoProto3i
{
	GSERVER_VIDEO_PROTO3i_glColor3i = 0,
	GSERVER_VIDEO_PROTO3i_glVertex3i
} GServerVideoProto3i;
# define GSERVER_VIDEO_PROTO3i_LAST GSERVER_VIDEO_PROTO3i_glVertex3i
# define GSERVER_VIDEO_PROTO3i_COUNT (GSERVER_VIDEO_PROTO3i_LAST + 1)

typedef enum _GServerVideoProto4f
{
	GSERVER_VIDEO_PROTO4f_glClearColor = 0,
	GSERVER_VIDEO_PROTO4f_glRotatef
} GServerVideoProto4f;
# define GSERVER_VIDEO_PROTO4f_LAST GSERVER_VIDEO_PROTO4f_glRotatef
# define GSERVER_VIDEO_PROTO4f_COUNT (GSERVER_VIDEO_PROTO4f_LAST + 1)

/* GServerVideoPlugin */
typedef struct _GServerVideoPlugin GServerVideoPlugin;

typedef struct _GServerVideoPluginHelper
{
	GServer * gserver;
	char const * (*config_get)(GServer * gserver, char const * section,
			char const * variable);
	Event * (*get_event)(GServer * gserver);
	void (*refresh)(GServer * gserver);
} GServerVideoPluginHelper;

struct _GServerVideoPlugin
{
	GServerVideoPluginHelper * helper;
	char const * name;
	int (*init)(GServerVideoPlugin * plugin);
	void (*destroy)(GServerVideoPlugin * plugin);
	void (*proto0)(GServerVideoPlugin * plugin, GServerVideoProto0 func);
	void (*proto1d)(GServerVideoPlugin * plugin, GServerVideoProto1d func,
			double x);
	void (*proto1i)(GServerVideoPlugin * plugin, GServerVideoProto1i func,
			int32_t x);
	void (*proto3f)(GServerVideoPlugin * plugin, GServerVideoProto3f func,
			float x, float y, float z);
	void (*proto3i)(GServerVideoPlugin * plugin, GServerVideoProto3i func,
			int32_t x, int32_t y, int32_t z);
	void (*proto4f)(GServerVideoPlugin * plugin, GServerVideoProto4f func,
			float x, float y, float z, float t);
	void * priv;
};

#endif /* !GRAPHICS_GSERVER_VIDEO_H */
