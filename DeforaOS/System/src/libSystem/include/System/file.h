/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System libSystem */
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



#ifndef LIBSYSTEM_FILE_H
# define LIBSYSTEM_FILE_H


/* types */
typedef struct _File File;
typedef int FileMode; /* FIXME actually is an enumerated type */

/* functions */
File * file_new(char const * path, FileMode mode);
void file_delete(File * file);

/* useful */
ssize_t file_read(File * file, void * buf, size_t size, ssize_t count);
ssize_t file_write(File * file, void * buf, size_t size, ssize_t count);

#endif /* !LIBSYSTEM_FILE_H */
