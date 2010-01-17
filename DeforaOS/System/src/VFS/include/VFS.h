/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System VFS */
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



#ifndef VFS_VFS_H
# define VFS_VFS_H


/* VFS */
/* constants */
/* lseek */
enum { VFS_SEEK_SET = 0, VFS_SEEK_CUR = 1, VFS_SEEK_END = 2 };


/* open */
enum { VFS_O_RDONLY = 0x0, VFS_O_WRONLY = 0x1, VFS_O_RDWR = 0x2,
	VFS_O_ACCMODE = 0x4 };
enum { VFS_O_CREAT = 0x10, VFS_O_EXCL = 0x20, VFS_O_TRUNC = 0x40,
	VFS_O_NOCTTY = 0x80 };
enum { VFS_O_APPEND = 0x100, VFS_O_DSYNC = 0x200, VFS_O_NONBLOCK = 0x400,
	VFS_O_RSYNC = 0x800, VFS_O_SYNC = 0x1000 };

#endif /* !VFS_VFS_H */
