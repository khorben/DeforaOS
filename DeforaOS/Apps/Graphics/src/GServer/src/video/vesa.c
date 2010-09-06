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



#include <stddef.h>
#include "video.h"
#include "vbe.h"


/* private */
/* functions */
/* vesa_init */
static int _vesa_init(void)
{
	return 0;
}


/* vesa_destroy */
static void _vesa_destroy(void)
{
}


/* public */
/* variables */
VideoPlugin video_plugin =
{
	_vesa_init,
	_vesa_destroy,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};
