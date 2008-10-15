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



#ifndef LIBSYSTEM_ERROR_H
# define LIBSYSTEM_ERROR_H


/* functions */
/* accessors */
char const * error_get(void);
char const * error_get_code(int * code);

void error_set(char const * format, ...);
int error_set_code(int code, char const * format, ...);
int error_set_print(char const * program, int code, char const * format, ...);

/* useful */
int error_print(char const * program);

#endif /* !LIBSYSTEM_ERROR_H */
