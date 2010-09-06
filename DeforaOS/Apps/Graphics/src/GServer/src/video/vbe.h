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



#ifndef GSERVER_VIDEO_VBE_H
# define GSERVER_VIDEO_VBE_H

#include <stdint.h>


/* types */
/* FIXME only present useful sections and do the selection in vbe.c */
typedef struct _VbeInfo
{
	char vbesignature[4];
	int16_t vbeversion;
	int32_t * oemstringptr;
	char capabilities[4];
	int32_t videomodeptr;
	int16_t totalmemory;
	/* VBE 2.0+ */
	int16_t oemsoftwarerev;
	int32_t oemvendornameptr;
	int32_t oemproductnameptr;
	int32_t oemproductrevptr;
	char reserved[222];
	char oemdata[256];
} VbeInfo;


/* functions */
int vbe_info(VbeInfo * buf);

#endif /* !GSERVER_VIDEO_VBE_H */
