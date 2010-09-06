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



#ifndef GSERVER_VIDEO_VIDEO_H
# define GSERVER_VIDEO_VIDEO_H

# include <stdint.h>


/* Video */
/* types */
typedef enum _VideoProto0
{
	VIDEO_PROTO0_glEnd = 0,
	VIDEO_PROTO0_glFlush,
	VIDEO_PROTO0_glLoadIdentity,
	VIDEO_PROTO0_SwapBuffers
} VideoProto0;
# define VIDEO_PROTO0_LAST VIDEO_PROTO0_SwapBuffers
# define VIDEO_PROTO0_COUNT (VIDEO_PROTO0_LAST + 1)

typedef enum _VideoProto1d
{
	VIDEO_PROTO1d_glClearDepth = 0
} VideoProto1d;
# define VIDEO_PROTO1d_LAST VIDEO_PROTO1d_glClearDepth
# define VIDEO_PROTO1d_COUNT (VIDEO_PROTO1d_LAST + 1)

typedef enum _VideoProto1i
{
	VIDEO_PROTO1i_glBegin = 0,
	VIDEO_PROTO1i_glClear
} VideoProto1i;
# define VIDEO_PROTO1i_LAST VIDEO_PROTO1i_glClear
# define VIDEO_PROTO1i_COUNT (VIDEO_PROTO1i_LAST + 1)

typedef enum _VideoProto3f
{
	VIDEO_PROTO3f_glColor3f = 0,
	VIDEO_PROTO3f_glTranslatef,
	VIDEO_PROTO3f_glVertex3f
} VideoProto3f;
# define VIDEO_PROTO3f_LAST VIDEO_PROTO3f_glVertex3f
# define VIDEO_PROTO3f_COUNT (VIDEO_PROTO3f_LAST + 1)

typedef enum _VideoProto3i
{
	VIDEO_PROTO3i_glColor3i = 0,
	VIDEO_PROTO3i_glVertex3i
} VideoProto3i;
# define VIDEO_PROTO3i_LAST VIDEO_PROTO3i_glVertex3i
# define VIDEO_PROTO3i_COUNT (VIDEO_PROTO3i_LAST + 1)

typedef enum _VideoProto4f
{
	VIDEO_PROTO4f_glClearColor = 0
} VideoProto4f;
# define VIDEO_PROTO4f_LAST VIDEO_PROTO4f_glClearColor
# define VIDEO_PROTO4f_COUNT (VIDEO_PROTO4f_LAST + 1)

typedef struct _VideoPlugin
{
	int (*init)(void);
	void (*destroy)(void);
	void (*proto0)(VideoProto0 func);
	void (*proto1d)(VideoProto1d func, double x);
	void (*proto1i)(VideoProto1i func, int32_t x);
	void (*proto3f)(VideoProto3f func, float x, float y, float z);
	void (*proto3i)(VideoProto3i func, int32_t x, int32_t y, int32_t z);
	void (*proto4f)(VideoProto4f func, float x, float y, float z, float t);
} VideoPlugin;

#endif /* !GSERVER_VIDEO_VIDEO_H */
