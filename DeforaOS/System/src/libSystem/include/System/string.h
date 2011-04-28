/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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



#ifndef LIBSYSTEM_STRING_H
# define LIBSYSTEM_STRING_H


/* String */
/* types */
typedef char String;


/* functions */
String * string_new(String const * string);
String * string_new_append(String const * string, ...);
String * string_new_length(String const * string, size_t length);
void string_delete(String * string);

/* accessors */
int string_set(String ** string, String const * string2);

/* returns */
size_t string_length(String const * string);

/* useful */
int string_append(String ** string, String const * append);
void string_cut(String * string, size_t length);

int string_compare(String const * string, String const * string2);
int string_compare_length(String const * string, String const * string2,
		size_t length);

String ** string_explode(String const * string, String const * separator);

String * string_find(String const * string, String const * key);
ssize_t string_index(String const * string, String const * key);

int string_replace(String ** string, String const * what, String const * by);

#endif /* !LIBSYSTEM_STRING_H */
