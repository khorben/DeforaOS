/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix others */
/* others is not free software; you can redistribute it and/or modify it under
 * the terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * others is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with others; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#ifndef OTHERS_TAR_H
# define OTHERS_TAR_H

# include <sys/types.h>
# include <stdint.h>


/* types */
typedef enum {
	FT_NORMAL = 0,
	FT_HARDLINK = 1,
	FT_SYMLINK = 2,
	FT_CHAR = 3,
	FT_BLOCK = 4,
	FT_DIRECTORY = 5,
	FT_FIFO = 6,
	FT_CONTIGUOUS = 7
} FileType;

#pragma pack(1)
typedef struct _TarFileHeaderBuffer
{
	char filename[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char checksum[8];
	uint8_t type;
	char link[100];
} TarFileHeaderBuffer;
#pragma pack()

typedef struct _TarFileHeader
{
	char filename[101];
	mode_t mode;
	uid_t uid;
	gid_t gid;
	size_t size;
	time_t mtime;
	FileType type;
	char link[101];
} TarFileHeader;

#endif /* !OTHERS_TAR_H */
